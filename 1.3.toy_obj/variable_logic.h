#pragma once
#include "common.h"

#include "llvm/IR/LegacyPassManager.h"

#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Support/CodeGen.h"





//using namespace llvm;

class FunctionPrototypeAST;
class ExprAST;
class VariableExprAST;
class NumericExprAST;
class BinaryExprAST;
class FunctionImplAST;
class CallExprAST;

void variable_bootup_init();

void variable_InitializeModule();
void variable_functionast_codegen(llvm::Function *TheFunction);
void variable_post_main();

void variable_Handle_Top_Level_Expression();
void variable_Handle_Function_Definition(llvm::Function *function);
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc);


llvm::raw_ostream & variable_ExprAST_dump(ExprAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_VariableExprAST_dump(VariableExprAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_NumericExprAST_dump(NumericExprAST& self, llvm::raw_ostream &out, int ind);
llvm::raw_ostream & variable_BinaryExprAST_dump(BinaryExprAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_FunctionImplAST_dump(FunctionImplAST& self, llvm::raw_ostream &out, int ind) ;
llvm::raw_ostream & variable_CallExprAST_dump(CallExprAST& self, llvm::raw_ostream &out, int ind);
