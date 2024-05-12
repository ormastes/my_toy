#include "variable_logic.h"
#include "llvm/Support/Error.h"
#include "common.h"
#include <string>
#include "ast.h"
#include "Interpreter.h"
// IrBuilder
#include "llvm/IR/IRBuilder.h"

std::unique_ptr<Interpreter> TheJIT; // for jit support
extern std::unique_ptr<llvm::IRBuilder<>> Builder;

void variable_bootup_init() {
    // select the target // jit need target selection
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    TheJIT = ExitOnErr(Interpreter::Create());;
}

void variable_InitializeModule() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>(__PROJECT_NAME__, *TheContext);
    TheModule->setDataLayout(TheJIT->getDataLayout());// getTargetMachine().createDataLayout());

    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

void variable_Handle_Top_Level_Expression() {}
void variable_post_main() {}

void variable_Interpreter_init(llvm::orc::JITTargetMachineBuilder JTMB, llvm::orc::RTDyldObjectLinkingLayer &ObjectLayer) {
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc) {
	return std::make_unique<FunctionPrototypeAST>(CurLoc, "__anon_expr", std::vector<std::string>()) ;
}


//===----------------------------------------------------------------------===//
// dump
//===----------------------------------------------------------------------===//


llvm::raw_ostream & variable_ExprAST_dump(ExprAST& self, llvm::raw_ostream &out, int ind) {
    return out;
}
llvm::raw_ostream & variable_VariableExprAST_dump(VariableExprAST& self, llvm::raw_ostream &out, int ind)  {
    return out;
}
llvm::raw_ostream & variable_NumericExprAST_dump(NumericExprAST& self, llvm::raw_ostream &out, int ind)  {
    return out;
}
llvm::raw_ostream & variable_BinaryExprAST_dump(BinaryExprAST& self, llvm::raw_ostream &out, int ind)  {
    return out;
}
llvm::raw_ostream & variable_FunctionImplAST_dump(FunctionImplAST& self, llvm::raw_ostream &out, int ind) {
    return out;
}
llvm::raw_ostream & variable_CallExprAST_dump(CallExprAST& self, llvm::raw_ostream &out, int ind)  {
    return out;
}

