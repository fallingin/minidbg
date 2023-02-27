#ifndef MINIDBG_BREAKPOINT_HPP
#define MINIDBG_BREAKPOINT_HPP

#include <linux/types.h>

#include <string>
#include <utility>
class breakpoint {
public:
    breakpoint(pid_t pid, std::intptr_t addr)
        : m_pid{pid}, m_addr{addr}, m_enabled{false}, m_saved_data{}
    {}
    breakpoint(){}
    // 启用断点
    void enable();
    // 弃用断点
    void disable();
    auto is_enabled() const -> bool { return m_enabled; }
    auto get_address() const -> std::intptr_t { return m_addr; }
private:
    pid_t m_pid;// 进程号
    std::intptr_t m_addr;// 断点地址
    bool m_enabled;
    uint64_t m_saved_data; // 原始数据
};

#endif