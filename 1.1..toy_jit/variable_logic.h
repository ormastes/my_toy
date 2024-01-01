#pragma once

// for jit support
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

#include "Interpreter.h"

//using namespace llvm;

extern std::unique_ptr<Interpreter> TheJIT;

void variable_bootup_init();

void variable_InitializeModule();
void variable_functionast_codegen();
void variable_post_main();

void variable_Handle_Top_Level_Expression();
void    variable_Interpreter_init();
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level();
raw_ostream & variable_ExprAST_dump(ExprAST& self, raw_ostream &out, int ind) ;


raw_ostream & variable_VariableExprAST_dump(VariableExprAST& self, raw_ostream &out, int ind) ;
raw_ostream & variable_NumericExprAST_dump(NumericExprAST& self, raw_ostream &out, int ind;
raw_ostream & variable_BinaryExprAST_dump(BinaryExprAST& self, raw_ostream &out, int ind) ;
raw_ostream & variable_FunctionImplAST_dump(FunctionImplAST& self, raw_ostream &out, int ind) ;
raw_ostream & variable_CallExprAST_dump(CallExprAST& self, raw_ostream &out, int ind);


//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a int and returns 0.
extern "C" DLLEXPORT int putchard(int X);
/// printd - printf that takes a int prints it as "%d\n", returning 0.
extern "C" DLLEXPORT int printd(int X);
