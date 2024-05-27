# Toy Front
How parse and generate IR for toy language.
Reference: https://llvm.org/docs/WritingAnLLVMBackend.html

## Entrance 
toy.cpp has main() entrance.

## Tokenize and Parse
toy.cpp calls Driver()
Driver() calls getNextToken() to get token.
Driver() calls Handle_XX() in parser.h
Handle_XX() calls parse_XX() in parser.h
Handle_XX() can calls Codegen() in ast_code_gen.h
Codegen() can put parsed one in FunctionProtos[]
Codegen() call Builder in toy.cpp to generate IR
Last,
PrintModule() print IR by TheModule->print(errs(), nullptr) in common.h

## Object Reference
TheModule -> TheContext
Builder -> TheContext
```cpp
static void InitializeModule() {
    // Open a new context and module.
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>(__PROJECT_NAME__, *TheContext);
    variable_InitializeModule();
    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}
```