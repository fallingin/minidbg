#include "debugger.hpp"

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "breakpoint.hpp"
#include "linenoise.h"
#include "regs.hpp"

// 将字符串 s 用分隔符 delimiter 分割
std::vector<std::string> split(const std::string& s, char delimiter) {
  std::vector<std::string> out{};
  std::stringstream ss{s};
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    out.push_back(item);
  }

  return out;
}

// 检验字符串 s 是否为字符串 of 的前缀
bool is_prefix(const std::string& s, const std::string& of) {
  if (s.size() > of.size()) return false;
  return std::equal(s.begin(), s.end(), of.begin());
}

void debugger::run() {
  // 设置 linenoise 的历史追溯命令条数
  linenoiseHistorySetMaxLen(20);

  int wait_status;
  auto options = 0;
  //等待直到子进程完成启动
  waitpid(m_pid, &wait_status, options);
  // 然后一直从 linenoise 获取输入直到收到 EOF（CTRL+D）
  char* line = nullptr;
  while ((line = linenoise("minidbg> ")) != nullptr) {
    handle_command(line);
    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }
}

void debugger::handle_command(const std::string& line) {
  auto args = split(line, ' ');
  auto command = args[0];
  // 如果输入是 continue 的前缀，使子进程继续运行直到下一个断点
  if (is_prefix(command, "continue")) {
    continue_execution();
  } else if (is_prefix(command, "break")) {  // 设置新断点
    std::string addr{args[1],
                     2};  // naively assume that the user has written 0xADDRESS
    set_breakpoint_at_address(std::stol(addr, 0, 16));
  } else if (is_prefix(command, "register")) { // 寄存器相关
    if (is_prefix(args[1], "dump")) { // 显示所有寄存器的值
      dump_registers();
    } else if (is_prefix(args[1], "read")) { //根据名字读取指定寄存器的值
      std::cout << get_register_value(m_pid, get_register_from_name(args[2]))
                << std::endl;
    } else if (is_prefix(args[1], "write")) { //根据名字写入指定寄存器的值
      std::string val{args[3], 2};  // assume 0xVAL
      set_register_value(m_pid, get_register_from_name(args[2]),
                         std::stol(val, 0, 16));
    }
  } else if (is_prefix(command, "memory")) { // 内存相关 
    std::string addr{args[2], 2};  // assume 0xADDRESS
    if (is_prefix(args[1], "read")) {
      std::cout << std::hex << read_memory(std::stol(addr, 0, 16)) << std::endl;
    }
    if (is_prefix(args[1], "write")) {
      std::string val{args[3], 2};  // assume 0xVAL
      write_memory(std::stol(addr, 0, 16), std::stol(val, 0, 16));
    }
  } else {
    std::cerr << "Unknown command\n";
  }
}

void debugger::continue_execution() {
  // 跳过当前断点
  step_over_breakpoint(); 
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

  wait_for_signal();
}

void debugger::set_breakpoint_at_address(std::intptr_t addr) {
  std::cout << "Set breakpoint at address 0x" << std::hex << addr << std::endl;
  breakpoint bp{m_pid, addr};
  bp.enable();
  m_breakpoints[addr] = bp;
}
// 导出所有寄存器的值
void debugger::dump_registers() {
  for (const auto& rd : g_register_descriptors) {
    std::cout << rd.name << " : 0x" << std::setfill('0') << std::setw(16)
              << std::hex << get_register_value(m_pid, rd.r) << std::endl;
  }
}

uint64_t debugger::read_memory(uint64_t address) {
  return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr);
}

void debugger::write_memory(uint64_t address, uint64_t value) {
  ptrace(PTRACE_POKEDATA, m_pid, address, value);
}
// 封装常用的 waitpid 模式
void debugger::wait_for_signal() {
  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
}
// 封装程序计数器寄存器的读取与写入
uint64_t debugger::get_pc() { 
    return get_register_value(m_pid, reg::rip); 
} 
void debugger::set_pc(uint64_t pc) { 
    set_register_value(m_pid, reg::rip, pc); 
} 
// 检查当前程序计算器的值是否设置了一个断点。
// 如果有，首先把执行返回到断点之前，停用它，执行原来的单条指令，再重新启用断点。
void debugger::step_over_breakpoint() { 
    // - 1 because execution will go past the breakpoint 
    auto possible_breakpoint_location = get_pc() - 1; 
    if (m_breakpoints.count(possible_breakpoint_location)) { 
        auto& bp = m_breakpoints[possible_breakpoint_location]; 
        if (bp.is_enabled()) { 
            auto previous_instruction_address = possible_breakpoint_location; 
            set_pc(previous_instruction_address); 
            bp.disable(); 
            ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr); 
            wait_for_signal(); 
            bp.enable(); 
        } 
    } 
} 