cmake_minimum_required(VERSION 3.22)

project(toy  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(toy_front)
add_subdirectory(toy_jit)