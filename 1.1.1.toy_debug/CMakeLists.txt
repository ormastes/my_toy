cmake_minimum_required(VERSION 3.22)

project(toy_debug  LANGUAGES C CXX)


find_package(LLVM REQUIRED CONFIG)

add_executable(${PROJECT_NAME} toy.cpp variable_logic.cpp token.cpp)


# ${LLVM_INSTALL_PREFIX}/lib/cmake/llvm/include
include_directories(${LLVM_INCLUDE_DIRS})

# ./llvm-config --cxxflags
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# next only  include core lib
# ./llvm-config --ldflags --libs --system-libs --libs support core irreader
llvm_map_components_to_libnames(llvm_libs support core irreader 
    native
#    Analysis
    ExecutionEngine
#    InstCombine
    Object
#    RuntimeDyld
#    ScalarOpts
    OrcJIT
    TargetParser
    all
##    ASMParser MCA MCTargetDesc
##    LLVMIRPrinter LLVMAsmPrinter CodeGen LLVMX86CodeGen LLVMCParser LLVMIRParser
##    LLVMTarget LLVMTargetParser
##    LLVMX86AsmParser LLVMX86TargetMCA LLVMX86Desc LLVMX86AsmPrinter LLVMX86Utils LLVMX86Info LLVMX86Disassembler
)
message(STATUS "llvm_libs: ${llvm_libs}")
target_link_libraries(${PROJECT_NAME} ${llvm_libs})

target_compile_definitions(${PROJECT_NAME} PRIVATE __PROJECT_NAME__="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE __EXAMPLE_FILE__="${CMAKE_CURRENT_SOURCE_DIR}/example")

add_custom_target(run_${PROJECT_NAME}
    COMMAND ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/example
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CAMKE_CURRENT_BUILD_DIR}
    COMMENT "Running ${PROJECT_NAME}")



