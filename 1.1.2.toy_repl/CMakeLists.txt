cmake_minimum_required(VERSION 3.22)

project(toy_repl  LANGUAGES C CXX)


add_executable(${PROJECT_NAME} toy.cpp variable_logic.cpp)


# ${LLVM_INSTALL_PREFIX}/lib/cmake/llvm/include
include_directories(${LLVM_INCLUDE_DIRS})

# ./llvm-config --cxxflags
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# next only  include core lib
# ./llvm-config --ldflags --libs --system-libs --libs support core irreader
llvm_map_components_to_libnames(llvm_libs support core irreader native
    ExecutionEngine
    OrcJIT)
message(STATUS "llvm_libs: ${llvm_libs}")
target_link_libraries(${PROJECT_NAME} ${llvm_libs})

target_compile_definitions(${PROJECT_NAME} PRIVATE __PROJECT_NAME__="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE __EXAMPLE_FILE__="${CMAKE_CURRENT_SOURCE_DIR}/example")




