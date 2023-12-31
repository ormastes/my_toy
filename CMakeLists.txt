cmake_minimum_required(VERSION 3.22)

project(toy  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_PREFIX_PATH ~/dev/install/llvm-project-work_linux_x64_debug/lib/cmake/llvm)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_subdirectory(1..toy_front)
add_subdirectory(1.1..toy_jit)
add_subdirectory(1.1.1.toy_debug)
add_subdirectory(1.1.2.toy_repl)
add_subdirectory(1.2.toy_pass)
# not working LLVMInitializeX86AsmPrinter is not exist...
# full build llvm-project or static llink may fix??
# add_subdirectory(1.3toy_obj)