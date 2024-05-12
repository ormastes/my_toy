#include "variable_logic.h"
#include "llvm/Support/Error.h"
#include "common.h"
#include <string>
#include "ast.h"
class FunctionPrototypeAST;
// IrBuilder
#include "llvm/IR/IRBuilder.h"
static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;

void variable_bootup_init() {}

void variable_InitializeModule() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>(__PROJECT_NAME__, *TheContext);
	
    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
	
    // Create a new pass manager attached to it.
    TheFPM = std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(llvm::createCFGSimplificationPass());

    TheFPM->doInitialization();
}

void variable_Handle_Top_Level_Expression() {}
void variable_Handle_Function_Definition(llvm::Function *function) {}
void variable_functionast_codegen(llvm::Function *TheFunction) {
 // Run the optimizer on the function.
        TheFPM->run(*TheFunction);

}
void variable_post_main() {}
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc) {
	return std::make_unique<FunctionPrototypeAST>(CurLoc, "main", std::vector<std::string>()) ;
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


