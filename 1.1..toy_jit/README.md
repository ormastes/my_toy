# Toy Jit
Generate Native code.
Base on Toy Front

## Entrance 
toy.cpp has main() entrance.


## Init
COFF need different init.

```cpp
void variable_Interpreter_init(llvm::orc::JITTargetMachineBuilder JTMB, llvm::orc::RTDyldObjectLinkingLayer &ObjectLayer) {
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}
```


## Interpreter
orc base interpreter which generate native code.

```cpp
  std::unique_ptr<llvm::orc::ExecutionSession> ES;

  llvm::DataLayout DL;
  llvm::orc:: MangleAndInterner Mangle;

  llvm::orc::RTDyldObjectLinkingLayer ObjectLayer;
  llvm::orc::IRCompileLayer CompileLayer;

  llvm::orc::JITDylib &MainJD;
```

## 
ResourceTracker is added with Module(and Context) to JIT.
And lookup function(__anon_expr) and run it
and remove the function.

```cpp
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
```

## Top level Anonymous Expression
when parse anonymous expression, name it to "__anon_expr"

```cpp
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc) {
	return std::make_unique<FunctionPrototypeAST>(CurLoc, "__anon_expr", std::vector<std::string>()) ;
}
```
