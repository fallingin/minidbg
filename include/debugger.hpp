#ifndef MINIDBG_DEBUGGER_HPP
#define MINIDBG_DEBUGGER_HPP

#include <linux/types.h>

#include <string>
#include <unordered_map>
#include <utility>

#include "breakpoint.hpp"

class debugger {
 public:
  debugger(std::string prog_name, pid_t pid)
      : m_prog_name{std::move(prog_name)}, m_pid{pid} {}

  void run();

 private:
  void set_breakpoint_at_address(std::intptr_t addr);
  void handle_command(const std::string& line);
  void continue_execution();
  void wait_for_signal();
  void dump_registers();
  uint64_t read_memory(uint64_t address);
  void write_memory(uint64_t address, uint64_t value);
  void step_over_breakpoint();
  uint64_t get_pc();
  void set_pc(uint64_t pc);

  std::unordered_map<std::intptr_t, breakpoint> m_breakpoints;
  // 要调试的程序名
  std::string m_prog_name;
  // 进程号
  pid_t m_pid;
};

#endif
