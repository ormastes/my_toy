#pragma once

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

extern SourceLocation CurLoc;

class ExprAST {
  SourceLocation Loc;

public:
    ExprAST(SourceLocation Loc = CurLoc) : Loc(Loc) {}
    int getLine() const { return Loc.Line; }
    int getCol() const { return Loc.Col; }

    virtual llvm::raw_ostream &dump(llvm::raw_ostream &out, int ind) {return variable_ExprAST_dump(*this, out,ind);}
    virtual ~ExprAST() = default;
    virtual llvm::Value *Codegen() = 0;
};

class  VariableExprAST : public ExprAST {
    std::string Name;
public:
      VariableExprAST(SourceLocation Loc, const std::string &Name)
      : ExprAST(Loc), Name(Name) {}
    const std::string &getName() const { return Name; }
    virtual llvm::Value *Codegen() override;
    virtual llvm::raw_ostream &dump(llvm::raw_ostream &out, int ind) override {return variable_VariableExprAST_dump(*this, out,ind);}
};

class NumericExprAST : public ExprAST {
    int Val;
public:
    NumericExprAST(int val) : Val(val) {}
    virtual llvm::Value *Codegen() override;
    virtual llvm::raw_ostream &dump(llvm::raw_ostream &out, int ind) override {return variable_NumericExprAST_dump(*this, out,ind);};
};

class BinaryExprAST : public ExprAST {
public:
    std::string Op;
    std::unique_ptr<ExprAST> LHS, RHS;

    BinaryExprAST(SourceLocation Loc, std::string Op, std::unique_ptr<ExprAST> LHS,
                    std::unique_ptr<ExprAST> RHS)
        : ExprAST(Loc), Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    virtual llvm::Value *Codegen() override;
    virtual llvm::raw_ostream &dump(llvm::raw_ostream &out, int ind) override {return variable_BinaryExprAST_dump(*this, out,ind);}
};

class FunctionAST {
public:
    virtual ~FunctionAST() = default;
    virtual llvm::Function *Codegen() = 0;
};

class FunctionPrototypeAST  : public FunctionAST{
public:
    std::string Func_name;
    std::vector<std::string> Arg_names;
    int Line;

    FunctionPrototypeAST(SourceLocation Loc, const std::string &name, const std::vector<std::string> &args) :
        Func_name(name), Arg_names(args), Line(Loc.Line) {}
    int getLine() const { return Line; }
    virtual llvm::Function *Codegen() ;
};

class FunctionImplAST : public FunctionAST {
    std::unique_ptr<FunctionPrototypeAST> Func_Decl;
    std::unique_ptr<ExprAST> Func_Body;
public:
    FunctionImplAST(std::unique_ptr<FunctionPrototypeAST> proto,  std::unique_ptr<ExprAST> body) :
        Func_Decl(std::move(proto)), Func_Body(std::move(body)) {}
    virtual llvm::Function *Codegen() ; 
    virtual llvm::raw_ostream &dump(llvm::raw_ostream &out, int ind) {while(1);}//return variable_FunctionImplAST_dump(*this, out,ind);}
};

class CallExprAST : public ExprAST {
    std::string Func_Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(SourceLocation Loc, const std::string &Callee,
                std::vector<std::unique_ptr<ExprAST>> Args)
        : ExprAST(Loc), Func_Callee(Callee), Args(std::move(Args)) {}
    virtual llvm::Value *Codegen() override;
    virtual llvm::raw_ostream &dump(llvm::raw_ostream &out, int ind) override {return variable_CallExprAST_dump(*this, out,ind);}
};
