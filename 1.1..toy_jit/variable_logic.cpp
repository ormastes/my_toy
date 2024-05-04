#include "variable_logic.h"
#include "Interpreter.h"
#include "llvm/Support/Error.h"
#include "common.h"
#include <string>
#include "ast.h"

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
