cmake_minimum_required(VERSION 3.22)

project(toy  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:/dev/tool/msys2/mingw64/lib/cmake/zstd")
#find_package(zstd REQUIRED)
#set(CMAKE_PREFIX_PATH ~/dev/install/llvm-project-work_linux_x64_debug/lib/cmake/llvm)

#SET(CMAKE_C_COMPILER /home/ormastes/dev/install/clang_17_X86_reldeb//bin/clang)
#SET(CMAKE_CXX_COMPILER /home/ormastes/dev/install/clang_17_X86_reldeb//bin/clang++)
#SET(CMAKE_ASM_COMPILER /home/ormastes/dev/install/clang_17_X86_reldeb//bin/clang)
set(CMAKE_PREFIX_PATH /home/ormastes/dev/install/clang_17_X86_reldeb/lib/cmake/llvm)



find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# use ld liner
add_link_options(-fuse-ld=lld)
#add_compile_options(-v)
#add_link_options(-v)

add_subdirectory(1..toy_front)
add_subdirectory(1.1..toy_jit)
#add_subdirectory(1.1.1.toy_debug)
add_subdirectory(1.1.2.toy_repl)
#add_subdirectory(1.2.toy_pass)
# not working LLVMInitializeX86AsmPrinter is not exist...
# full build llvm-project or static llink may fix??
# add_subdirectory(1.3toy_obj)