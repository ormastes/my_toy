#pragma once

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//




class ExprAST {
  SourceLocation Loc;

public:
    ExprAST(SourceLocation Loc = CurLoc) : Loc(Loc) {}

    int getLine() const { return Loc.Line; }
    int getCol() const { return Loc.Col; }
    virtual raw_ostream &dump(raw_ostream &out, int ind) {
        return out << ':' << getLine() << ':' << getCol() << '\n';
    }
    virtual ~ExprAST() = default;
    virtual llvm::Value *Codegen() = 0;
};

class  VariableExprAST : public ExprAST {
    std::string Name;
public:
      VariableExprAST(SourceLocation Loc, const std::string &Name)
      : ExprAST(Loc), Name(Name) {}
    const std::string &getName() const { return Name; }

    raw_ostream &dump(raw_ostream &out, int ind) override {
        return ExprAST::dump(out << Name, ind);
    }
    virtual llvm::Value *Codegen() override;
};

class NumericExprAST : public ExprAST {
    int Val;
public:
    NumericExprAST(int val) : Val(val) {}
    raw_ostream &dump(raw_ostream &out, int ind) override {
        return ExprAST::dump(out << Val, ind);
    }
    virtual llvm::Value *Codegen() override;
};

class BinaryExprAST : public ExprAST {
    std::string Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinaryExprAST(SourceLocation Loc, char Op, std::unique_ptr<ExprAST> LHS,
                    std::unique_ptr<ExprAST> RHS)
        : ExprAST(Loc), Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    raw_ostream &dump(raw_ostream &out, int ind) override {
        ExprAST::dump(out << "binary" << Op, ind);
        LHS->dump(indent(out, ind) << "LHS:", ind + 1);
        RHS->dump(indent(out, ind) << "RHS:", ind + 1);
        return out;
    }
    virtual llvm::Value *Codegen() override;
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
    raw_ostream &dump(raw_ostream &out, int ind) {
        indent(out, ind) << "FunctionAST\n";
        ++ind;
        indent(out, ind) << "Body:";
        return Body ? Body->dump(out, ind) : out << "null\n";
    }
};

class CallExprAST : public ExprAST {
    std::string Func_Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(SourceLocation Loc, const std::string &Callee,
                std::vector<std::unique_ptr<ExprAST>> Args)
        : ExprAST(Loc), Callee(Callee), Args(std::move(Args)) {}

    raw_ostream &dump(raw_ostream &out, int ind) override {
        ExprAST::dump(out << "call " << Callee, ind);
        for (const auto &Arg : Args)
        Arg->dump(indent(out, ind + 1), ind + 1);
        return out;
    }
    virtual llvm::Value *Codegen() override;
};
