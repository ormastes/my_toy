#pragma once


class ExprAST;
class FunctionAST;


extern llvm::ExitOnError ExitOnErr;
extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule; 

inline llvm::Value * LogError(const char *text) {
    fprintf(stderr, "Error: %s\n", text);
    return nullptr;
}
inline llvm::Function * LogErrorF(const char *text) {
    LogError(text);
    return nullptr;
}
inline std::unique_ptr<ExprAST> LogErrorEA(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}
inline std::unique_ptr<FunctionAST>  LogErrorFA(const char *text) {
    LogError(text);
    return nullptr;
}
inline void  PrintModule() {
    // Print out all of the generated code.
    TheModule->print(llvm::errs(), nullptr);
}
