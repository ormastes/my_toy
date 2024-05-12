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

#include "common.h"

//using namespace llvm;
//using namespace llvm::orc;

class FunctionPrototypeAST;
class ExprAST;
class VariableExprAST;
class NumericExprAST;
class BinaryExprAST;
class FunctionImplAST;
class CallExprAST;

extern std::unique_ptr<Interpreter> TheJIT;

void variable_bootup_init();

void variable_InitializeModule();
void variable_functionast_codegen(llvm::Function *TheFunction);
void variable_post_main();

void variable_Handle_Top_Level_Expression();
void variable_Handle_Function_Definition(llvm::Function *function);
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc);
void variable_Interpreter_init(llvm::orc::JITTargetMachineBuilder JTMB, llvm::orc::RTDyldObjectLinkingLayer &ObjectLayer);

llvm::raw_ostream & variable_ExprAST_dump(ExprAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_VariableExprAST_dump(VariableExprAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_NumericExprAST_dump(NumericExprAST& self, llvm::raw_ostream &out, int ind);
llvm::raw_ostream & variable_BinaryExprAST_dump(BinaryExprAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_FunctionImplAST_dump(FunctionImplAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_CallExprAST_dump(CallExprAST& self, llvm::raw_ostream &out, int ind);


//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#elif defined(__linux__)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

/// putchard - putchar that takes a int and returns 0.
extern "C" EXPORT int putchard(int X);
/// printd - printf that takes a int prints it as "%d\n", returning 0.
extern "C" EXPORT int printd(int X);
