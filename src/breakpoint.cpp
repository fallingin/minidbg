#include "breakpoint.hpp"

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

// 用 int 3 指令 - 编码为 0xcc - 替换当前指定地址的指令。保存该地址之前的值，以便后面恢复该代码
void breakpoint::enable() {
    m_saved_data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
    uint64_t int3 = 0xcc;
    uint64_t data_with_int3 = ((m_saved_data & ~0xff) | int3); //set bottom byte to 0xcc
    ptrace(PTRACE_POKEDATA, m_pid, m_addr, data_with_int3);
    m_enabled = true;
}
// 恢复用 0xcc 所覆盖的原始数据
void breakpoint::disable() {
    ptrace(PTRACE_POKEDATA, m_pid, m_addr, m_saved_data);
    m_enabled = false;
}