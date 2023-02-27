#include "regs.hpp"

#include <algorithm>

/* 
struct user_regs_struct
{
  __extension__ unsigned long long int r15;
  __extension__ unsigned long long int r14;
  __extension__ unsigned long long int r13;
  __extension__ unsigned long long int r12;
  __extension__ unsigned long long int rbp;
  __extension__ unsigned long long int rbx;
  __extension__ unsigned long long int r11;
  __extension__ unsigned long long int r10;
  __extension__ unsigned long long int r9;
  __extension__ unsigned long long int r8;
  __extension__ unsigned long long int rax;
  __extension__ unsigned long long int rcx;
  __extension__ unsigned long long int rdx;
  __extension__ unsigned long long int rsi;
  __extension__ unsigned long long int rdi;
  __extension__ unsigned long long int orig_rax;
  __extension__ unsigned long long int rip;
  __extension__ unsigned long long int cs;
  __extension__ unsigned long long int eflags;
  __extension__ unsigned long long int rsp;
  __extension__ unsigned long long int ss;
  __extension__ unsigned long long int fs_base;
  __extension__ unsigned long long int gs_base;
  __extension__ unsigned long long int ds;
  __extension__ unsigned long long int es;
  __extension__ unsigned long long int fs;
  __extension__ unsigned long long int gs;
};
 */

// 获取指定寄存器的值
uint64_t get_register_value(pid_t pid, reg r) { 
    user_regs_struct regs; 
    // 读出的是所有寄存器的值
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs); 
    // 根据自定义数组下标来读取指定寄存器的值
    auto it = std::find_if(begin(g_register_descriptors), end(g_register_descriptors), 
                       [r](auto&& rd) { return rd.r == r; });
    return *(reinterpret_cast<uint64_t*>(&regs) +
             (it - begin(g_register_descriptors)));
} 
// 设置指定寄存器的值
void set_register_value(pid_t pid, reg r, uint64_t value) { 
    user_regs_struct regs; 
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs); 
    auto it = std::find_if(begin(g_register_descriptors), end(g_register_descriptors), 
                           [r](auto&& rd) { return rd.r == r; }); 
    *(reinterpret_cast<uint64_t*>(&regs) + (it - begin(g_register_descriptors))) = value; 
    ptrace(PTRACE_SETREGS, pid, nullptr, &regs); 
} 
// 通过dwarf编号获取指定寄存器的值
uint64_t get_register_value_from_dwarf_register (pid_t pid, unsigned int regnum) { 
    auto it = std::find_if(begin(g_register_descriptors), end(g_register_descriptors), 
                           [regnum](auto&& rd) { return rd.dwarf_r == regnum; }); 
    if (it == end(g_register_descriptors)) { 
        throw std::out_of_range{"Unknown dwarf register"}; 
    } 
    return get_register_value(pid, it->r); 
} 
// 获取寄存器名字
std::string get_register_name(reg r) { 
    auto it = std::find_if(begin(g_register_descriptors), end(g_register_descriptors), 
                           [r](auto&& rd) { return rd.r == r; }); 
    return it->name; 
} 
// 通过名字获取寄存器枚举
reg get_register_from_name(const std::string& name) { 
    auto it = std::find_if(begin(g_register_descriptors), end(g_register_descriptors), 
                           [name](auto&& rd) { return rd.name == name; }); 
    return it->r; 
}
