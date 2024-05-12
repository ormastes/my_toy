#pragma once
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DIBuilder.h"
#include "variable_logic.h"
extern std::unique_ptr<llvm::DIBuilder> DBuilder;
extern std::map<std::string, llvm::Value*> Named_Values;

llvm::Value *NumericExprAST::Codegen() {
    KSDbgInfo.emitLocation(this);
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(32, Val));
}
llvm::Value *VariableExprAST::Codegen() {
    llvm::Value *V = Named_Values[Name];
    if (!V) return LogError("Unknown variable name");
    KSDbgInfo.emitLocation(this);
    return  Builder->CreateLoad(llvm::Type::getDoubleTy(*TheContext), V, Name.c_str());
}
llvm::Value *BinaryExprAST::Codegen() {
    KSDbgInfo.emitLocation(this);
    llvm::Value *L = LHS->Codegen();
    llvm::Value *R = RHS->Codegen();
    if (!L || !R) return 0;

    switch(Op[0]) {
        case '+': return Builder->CreateAdd(L, R, "addtmp");
        case '-': return Builder->CreateSub(L, R, "subtmp");
        case '*': return Builder->CreateMul(L, R, "multmp");
        case '<':
            L = Builder->CreateICmpULT(L, R, "cmptmp");
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "booltmp");
        default: return LogError("invalid binary operator");
    }
}
llvm::Function *FunctionPrototypeAST::Codegen() {
    std::vector<llvm::Type*> Integers(Arg_names.size(),  llvm::Type::getInt32Ty(*TheContext));
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), Integers, false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Func_name, TheModule.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(Arg_names[Idx++]);

    return F;
}

llvm::Function *FunctionImplAST::Codegen() {
    Named_Values.clear();
    auto &P = *Func_Decl;
    FunctionProtos[Func_Decl->Func_name] = std::move(Func_Decl);
    llvm::Function *TheFunction = getFunction(P.Func_name);
    if (TheFunction == 0) return 0;

    Named_Values.clear();
    for (auto &Arg : TheFunction->args())
        Named_Values[std::string(Arg.getName())] = &Arg;
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);
    
  // Create a subprogram DIE for this function.
  llvm::DIFile *Unit = DBuilder->createFile(KSDbgInfo.TheCU->getFilename(),
                                      KSDbgInfo.TheCU->getDirectory());
  llvm::DIScope *FContext = Unit;
  unsigned LineNo = P.getLine();
  unsigned ScopeLine = LineNo;
  llvm::DISubprogram *SP = DBuilder->createFunction(
      FContext, P.getName(), llvm::StringRef(), Unit, LineNo,
      CreateFunctionType(TheFunction->arg_size()), ScopeLine,
      llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition);
  TheFunction->setSubprogram(SP);

  // Push the current scope.
  KSDbgInfo.LexicalBlocks.push_back(SP);

  // Unset the location for the prologue emission (leading instructions with no
  // location in a function are considered part of the prologue and the debugger
  // will run past them when breaking on a function)
  KSDbgInfo.emitLocation(nullptr);

// Record the function arguments in the NamedValues map.
  Named_Values.clear();
  unsigned ArgIdx = 0;
  for (auto &Arg : TheFunction->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

    // Create a debug descriptor for the variable.
    llvm::DILocalVariable *D = DBuilder->createParameterVariable(
        SP, Arg.getName(), ++ArgIdx, Unit, LineNo, KSDbgInfo.getDoubleTy(),
        true);

    DBuilder->insertDeclare(Alloca, D, DBuilder->createExpression(),
                            llvm::DILocation::get(SP->getContext(), LineNo, 0, SP),
                            Builder->GetInsertBlock());

    // Store the initial value into the alloca.
    Builder->CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    Named_Values[std::string(Arg.getName())] = Alloca;
  }

  KSDbgInfo.emitLocation(Func_Body.get());

  if (llvm::Value *RetVal = Func_Body->Codegen()) {
    // Finish off the function.
    Builder->CreateRet(RetVal);

    // Pop off the lexical block for the function.
    KSDbgInfo.LexicalBlocks.pop_back();

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    return TheFunction;
  }
    TheFunction->eraseFromParent();

      // Pop off the lexical block for the function since we added it
  // unconditionally.
  KSDbgInfo.LexicalBlocks.pop_back();

    return 0;
}

llvm::Value *CallExprAST::Codegen() {
    KSDbgInfo.emitLocation(this);
    llvm::Function *CalleeF = getFunction(Func_Callee);
    if (CalleeF == 0) return LogError("Unknown function referenced");

    if (CalleeF->arg_size() != Args.size()) return LogError("Incorrect # arguments passed");

    std::vector<llvm::Value*> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->Codegen());
        if (ArgsV.back() == 0) return 0;
    }

    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}
