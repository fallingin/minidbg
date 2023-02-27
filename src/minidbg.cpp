#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "debugger.hpp"

/*
enum __ptrace_request
{
        PTRACE_TRACEME = 0,		//被调试进程调用
        PTRACE_PEEKDATA = 2,	//查看内存
        PTRACE_PEEKUSER = 3,	//查看struct user 结构体的值
        PTRACE_POKEDATA = 5,	//修改内存
        PTRACE_POKEUSER = 6,	//修改struct user 结构体的值
        PTRACE_CONT = 7,		//让被调试进程继续
        PTRACE_SINGLESTEP = 9,	//让被调试进程执行一条汇编指令
        PTRACE_GETREGS = 12,	//获取一组寄存器(struct user_regs_struct)
        PTRACE_SETREGS = 13,	//修改一组寄存器(struct user_regs_struct)
        PTRACE_ATTACH = 16,		//附加到一个进程
        PTRACE_DETACH = 17,		//解除附加的进程
        PTRACE_SYSCALL = 24,	//让被调试进程在系统调用前和系统调用后暂停
};
 */

void execute_debugee(const std::string& prog_name) {
  // PTRACE_TRACEME通知主进程，本进程可供追踪
  if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
    std::cerr << "Error in ptrace\n";
    return;
  }
  // 用指定程序替换正在运行的子进程
  execl(prog_name.c_str(), prog_name.c_str(), nullptr);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Program name not specified";
    return -1;
  }

  auto prog = argv[1];

  auto pid = fork();
  if (pid == 0) {
    // child
    // execute debugee
    // 在子进程，用需要被调试的程序替换正在执行的程序
    execute_debugee(prog);

  } else if (pid >= 1) {
    // parent
    // execute debugger
    // 在父进程，执行调试循环(处理输入，并根据输入执行相应动作)
    std::cout << "Started debugging process " << pid << '\n';
    debugger dbg{prog, pid};
    dbg.run();
  }
}
