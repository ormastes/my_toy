#include "variable_logic.h"
#include "Interpreter.h"
#include "llvm/Support/Error.h"
#include "common.h"

std::unique_ptr<Interpreter> TheJIT; // for jit support

void variable_bootup_init() {
    // select the target // jit need target selection
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    TheJIT = ExitOnErr(Interpreter::Create());;
}

void variable_InitializeModule() {
    TheModule->setDataLayout(TheJIT->getDataLayout());// getTargetMachine().createDataLayout());
}

void variable_Handle_Top_Level_Expression() {
    PrintModule();
    auto RT = TheJIT->getMainJITDylib().createResourceTracker();
    auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule), std::move(TheContext));
    ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
    variable_InitializeModule();
    
    // Search the JIT for the __anon_expr symbol. which is root
    auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anon_expr"));
    //assert(ExprSymbol && "Function not found");

    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a double) so we can call it as a native function.
    int (*FP)() = ExprSymbol.getAddress().toPtr<int (*)()>();
    fprintf(stderr, "Evaluated to %d\n", FP());

    // Delete the anonymous expression module from the JIT.
    ExitOnErr(RT->remove());
}
void variable_Interpreter_init(){}

//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a int and returns 0.
extern "C" DLLEXPORT int putchard(int X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a int prints it as "%d\n", returning 0.
extern "C" DLLEXPORT int printd(int X) {
  fprintf(stderr, "%d\n", X);
  return 0;
}
