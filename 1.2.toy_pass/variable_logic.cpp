#include "variable_logic.h"
static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;


void variable_bootup_init() {}

void variable_InitializeModule() {
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
void variable_functionast_codegen() {
 // Run the optimizer on the function.
        TheFPM->run(*TheFunction);

}
