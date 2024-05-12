# Toy Jit
Generate Native code.
Base on Toy Front

## Entrance 
toy.cpp has main() entrance.


## Resource Tracker and run top level expression.
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


