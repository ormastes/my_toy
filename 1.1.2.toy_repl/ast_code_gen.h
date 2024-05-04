#pragma once


llvm::Value *NumericExprAST::Codegen() {
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(32, Val));
}
llvm::Value *VariableExprAST::Codegen() {
    llvm::Value *V = Named_Values[Name];
    if (!V) return LogError("Unknown variable name");
    return V;
}
llvm::Value *BinaryExprAST::Codegen() {
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
    if (llvm::Value *RetVal = Func_Body->Codegen()) {
        Builder->CreateRet(RetVal);
        verifyFunction(*TheFunction);

        return TheFunction;
    }
    TheFunction->eraseFromParent();

    return 0;
}

llvm::Value *CallExprAST::Codegen() {
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
