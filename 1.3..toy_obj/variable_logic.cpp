#include "variable_logic.h"
#include "llvm/Support/Error.h"
#include "common.h"
#include <string>
#include "ast.h"
class FunctionPrototypeAST;
// IrBuilder
#include "llvm/IR/IRBuilder.h"

extern std::unique_ptr<llvm::IRBuilder<>> Builder;

void variable_bootup_init() {}

void variable_InitializeModule() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>(__PROJECT_NAME__, *TheContext);
	
    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

void variable_Handle_Top_Level_Expression() {}
void variable_Handle_Function_Definition(llvm::Function *function) {}
void variable_functionast_codegen(llvm::Function *TheFunction) {}
void variable_post_main() {
	
    // Initialize the target registry etc.
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        llvm::errs() << Error;
        return;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>();
    auto TheTargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "output.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CodeGenFileType::CGFT_ObjectFile; // windows it was? ObjectFile??

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*TheModule);
    dest.flush();

    llvm::outs() << "Wrote " << Filename << "\n";
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

