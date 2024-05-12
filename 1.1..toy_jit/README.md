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

### variable_InitializeModule
DataLayout added

```cpp
TheModule->setDataLayout(TheJIT->getDataLayout());
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

## Top level Anonymous Expression
when parse anonymous expression, name it to "__anon_expr"

```cpp
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc) {
	return std::make_unique<FunctionPrototypeAST>(CurLoc, "__anon_expr", std::vector<std::string>()) ;
}
```
