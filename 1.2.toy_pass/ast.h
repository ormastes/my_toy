#pragma once

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//
class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual llvm::Value *Codegen() = 0;
};

class  VariableExprAST : public ExprAST {
    std::string Name;
public:
    VariableExprAST(const std::string &name) : Name(name) {}
    virtual llvm::Value *Codegen() override;
};

class NumericExprAST : public ExprAST {
    int Val;
public:
    NumericExprAST(int val) : Val(val) {}
    virtual llvm::Value *Codegen() override;
};

class BinaryExprAST : public ExprAST {
    std::string Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinaryExprAST(const std::string &op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST>  rhs) :
        Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}
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
    FunctionPrototypeAST(const std::string &name, const std::vector<std::string> &args) :
        Func_name(name), Arg_names(args) {}
    virtual llvm::Function *Codegen() ;
};

class FunctionImplAST : public FunctionAST {
    std::unique_ptr<FunctionPrototypeAST> Func_Decl;
    std::unique_ptr<ExprAST> Func_Body;
public:
    FunctionImplAST(std::unique_ptr<FunctionPrototypeAST> proto,  std::unique_ptr<ExprAST> body) :
        Func_Decl(std::move(proto)), Func_Body(std::move(body)) {}
    virtual llvm::Function *Codegen() ; 
};

class CallExprAST : public ExprAST {
    std::string Func_Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) :
        Func_Callee(callee), Args(std::move(args)) {}
    virtual llvm::Value *Codegen() override;
};
