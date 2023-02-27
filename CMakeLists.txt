# 最低cmake版本，不满足报warning
cmake_minimum_required (VERSION 3.0)
# 定义工程名称
project (MiniDbg)

# 添加编译选项，使用c++14标准编译
add_compile_options(-std=c++14)

# 向工程添加多个特定的头文件搜索路径，相当于指定g++编译器的-I参数
include_directories(ext/libelfin ext/linenoise include)
# 编译所有指定的源文件，生成可执行文件minidbg
add_executable(minidbg src/minidbg.cpp src/debugger.cpp src/breakpoint.cpp src/regs.cpp ext/linenoise/linenoise.c)

add_executable(hello examples/hello.cpp)
# 改变目标可执行文件的属性，添加编译选项-g(ps：产生gdb(GNU) 的调试信息)，-O0(ps：不执行编译优化)
set_target_properties(hello
                      PROPERTIES COMPILE_FLAGS "-g -O0")

add_executable(variable examples/variable.cpp)
# -gdwarf-2(ps：产生DWARF version2 的格式的调试信息)
set_target_properties(variable
                      PROPERTIES COMPILE_FLAGS "-gdwarf-2 -O0")

add_executable(unwinding examples/stack_unwinding.cpp)
set_target_properties(unwinding
                      PROPERTIES COMPILE_FLAGS "-g -O0")

# 用于执行某些指令，此处在指定目录中执行make，生成libelfin
add_custom_target(
   libelfin
   COMMAND make
   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/ext/libelfin
)
# 为可执行文件minidbg添加需要链接的共享库，相当于指定g++编译器-l参数
target_link_libraries(minidbg
                      ${PROJECT_SOURCE_DIR}/ext/libelfin/dwarf/libdwarf++.so
                      ${PROJECT_SOURCE_DIR}/ext/libelfin/elf/libelf++.so)
# 提醒编译器需要先生成minidbg的依赖(libelfin),然后再去生成minidbg，
# 否则libelfin的生成会后于主可执行文件minidbg，导致minidbg找不到上条命令中的动态链接库
add_dependencies(minidbg libelfin)
