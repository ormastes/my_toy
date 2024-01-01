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

void variable_Interpreter_init() {
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc) {
	return std::make_unique<FunctionPrototypeAST>(CurLoc, "main", std::vector<std::string>()); ;
}
//===----------------------------------------------------------------------===//
// dump
//===----------------------------------------------------------------------===//
raw_ostream &indent(raw_ostream &O, int size) {
  return O << std::string(size, ' ');
}

raw_ostream & variable_ExprAST_dump(ExprAST& self, raw_ostream &out, int ind) {
        return out << ':' << self.getLine() << ':' << self.getCol() << '\n';
 }


raw_ostream & variable_VariableExprAST_dump(VariableExprAST& self, raw_ostream &out, int ind) override {
        return ExprAST::dump(out << self.Name, ind);
    }
raw_ostream & variable_NumericExprAST_dump(NumericExprAST& self, raw_ostream &out, int ind) override {
        return ExprAST::dump(out << self.Val, ind);
    }
raw_ostream & variable_BinaryExprAST_dump(BinaryExprAST& self, raw_ostream &out, int ind) override {
        self.ExprAST::dump(out << "binary" << self.Op, ind);
        self.LHS->dump(indent(out, ind) << "LHS:", ind + 1);
        self.RHS->dump(indent(out, ind) << "RHS:", ind + 1);
        return out;
    }
raw_ostream & variable_FunctionImplAST_dump(FunctionImplAST& self, raw_ostream &out, int ind) {
        indent(out, ind) << "FunctionAST\n";
        ++ind;
        indent(out, ind) << "Body:";
        return self.Body ? self.Body->dump(out, ind) : out << "null\n";
    }
raw_ostream & variable_CallExprAST_dump(CallExprAST& self, raw_ostream &out, int ind) override {
        ExprAST::dump(out << "call " << self.Callee, ind);
        for (const auto &Arg : self.Args)
        	self.Arg->dump(indent(out, ind + 1), ind + 1);
        return out;
    }



//===----------------------------------------------------------------------===//
// Debug Info Support
//===----------------------------------------------------------------------===//

static std::unique_ptr<DIBuilder> DBuilder;

DIType *DebugInfo::getDoubleTy() {
  if (DblTy)
    return DblTy;

  DblTy = DBuilder->createBasicType("double", 64, dwarf::DW_ATE_float);
  return DblTy;
}

void DebugInfo::emitLocation(ExprAST *AST) {
  if (!AST)
    return Builder->SetCurrentDebugLocation(DebugLoc());
  DIScope *Scope;
  if (LexicalBlocks.empty())
    Scope = TheCU;
  else
    Scope = LexicalBlocks.back();
  Builder->SetCurrentDebugLocation(DILocation::get(
      Scope->getContext(), AST->getLine(), AST->getCol(), Scope));
}

static DISubroutineType *CreateFunctionType(unsigned NumArgs) {
  SmallVector<Metadata *, 8> EltTys;
  DIType *DblTy = KSDbgInfo.getDoubleTy();

  // Add the result type.
  EltTys.push_back(DblTy);

  for (unsigned i = 0, e = NumArgs; i != e; ++i)
    EltTys.push_back(DblTy);

  return DBuilder->createSubroutineType(DBuilder->getOrCreateTypeArray(EltTys));
}


void variable_Handle_Top_Level_Expression() {
  // for jit support
  PrintModule();
  auto RT = TheJIT->getMainJITDylib().createResourceTracker();
  auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule), std::move(TheContext));
  ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
  InitializeModule();
  
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
