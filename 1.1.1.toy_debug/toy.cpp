#include <string>
#include <stdio.h>
#include <vector>
#include <map>

// for static single assignment
#include "llvm/IR/Value.h"
// IrBuilder
#include "llvm/IR/IRBuilder.h"
// LogError
//#include "llvm/Support/Debug.h"
// verifyFunction
#include "llvm/IR/Verifier.h"

#include "llvm/IR/DIBuilder.h"

#include "variable_logic.h"

llvm::Function *getFunction(std::string Name);
class FunctionPrototypeAST;
llvm::ExitOnError ExitOnErr;
static std::map<std::string, std::unique_ptr<FunctionPrototypeAST>> FunctionProtos;
std::unique_ptr<llvm::LLVMContext> TheContext; // contains all the states global to the compiler
std::unique_ptr<llvm::Module> TheModule{}; // contains functions and global variables //{} to ensure nullpt init
std::unique_ptr<llvm::IRBuilder<>> Builder; // ir build keeps track of the current location for inserting new instructions
std::map<std::string, llvm::Value*> Named_Values;



#include "common.h"
#include "token.h"
#include "ast.h"
#include "ast_code_gen.h"
#include "parser.h"


// clang++-16 -I ~/dev/0.my/play_llvm_x86_linux_install/include toy.cpp -o toy
llvm::Function *getFunction(std::string Name) {
  // First, see if the function has already been added to the current module.
  if (auto *F = TheModule->getFunction(Name))
    return F;

  // If not, check whether we can codegen the declaration from some existing
  // prototype.
  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end())
    return FI->second->Codegen();

  // If no existing prototype exists, return null.
  return nullptr;
}

static void bootup_init() {
    variable_bootup_init();
}
 // keeps track of which values are defined in the current scope
static void InitializeModule() {
    // Open a new context and module.
    variable_InitializeModule();
}



//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
static void Driver() {
    init_precedence();

    getNextToken();

    while(1) {
        switch(CurTok) {
            case tok_eof:
                return;
            case ';':
                getNextToken();
                break;
            case tok_def:
                Handle_Function_Definition();
                break;
            case tok_extern:
                Handle_Extern();
                break;
            default:
                Handle_Top_Level_Expression();
                break;
        }
    }
}

// Avoid including "llvm-c/Core.h" for compile time, fwd-declare this instead.
//llvm  extern "C" LLVMContextRef LLVMGetGlobalContext(void);

//llvm  Module *Module_Ob;

int main(int argc, char** argv) {
    bootup_init();
    InitializeModule();
    //llvm  LLVMContextRef Context = LLVMGetGlobalContext();
    file = fopen(__EXAMPLE_FILE__, "r"); //fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Can't open file\n");
        return 1;
    }

    
  // Add the current debug info version into the module.
  TheModule->addModuleFlag(llvm::Module::Warning, "Debug Info Version",
                           llvm::DEBUG_METADATA_VERSION);

  // Darwin only supports dwarf2.
  if (llvm::Triple(TheJIT->getTargetTriple()).isOSDarwin())
    TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

  // Construct the DIBuilder, we do this here because we need the module.
  DBuilder = std::make_unique<llvm::DIBuilder>(*TheModule);

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  KSDbgInfo.TheCU = DBuilder->createCompileUnit(
      llvm::dwarf::DW_LANG_C, DBuilder->createFile("fib.ks", "."),
      "Kaleidoscope Compiler", false, "", 0);

    //Module_Ob = new Module("my cool jit", Context);
    Driver();
    
    PrintModule();
    variable_post_main();

    return 0;
}
