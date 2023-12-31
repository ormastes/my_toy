cmake_minimum_required(VERSION 3.22)

project(toy_obj  LANGUAGES C CXX)


find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(${PROJECT_NAME} toy.cpp)


# ${LLVM_INSTALL_PREFIX}/lib/cmake/llvm/include
include_directories(${LLVM_INCLUDE_DIRS})

# ./llvm-config --cxxflags
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# next only  include core lib
# ./llvm-config --ldflags --libs --system-libs --libs support core irreader
llvm_map_components_to_libnames(llvm_libs support core irreader all
MC
TargetParser
)

message(STATUS "llvm_libs: ${llvm_libs}")
target_link_libraries(${PROJECT_NAME} ${llvm_libs})





