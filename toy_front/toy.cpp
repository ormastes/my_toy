#include <string>
#include <stdio.h>
#include <vector>
#include <map>

// for static single assignment
#include "llvm/IR/Value.h"
// IrBuilder
#include "llvm/IR/IRBuilder.h"
// LogError
//#include "llvm/Support/Debug.h"
// verifyFunction
#include "llvm/IR/Verifier.h"

//llvm #include "llvm/IR/LLVMContext.h"

using namespace llvm;
// clang++-16 -I ~/dev/0.my/play_llvm_x86_linux_install/include toy.cpp -o toy

enum Token_Type {
    EOF_TOKEN = 0,
    NUMERIC_TOKEN,
    IDENTIFIER_TOKEN,
    PAREN_TOKEN,
    DEF_TOKEN
};
static int Numeric_Val;
static std::string Identifier_string;
FILE* file;

static int gettok() {
    static int LastChar = ' ';
    while(isspace(LastChar)) LastChar = fgetc(file);

    if (isalpha(LastChar)) {
        Identifier_string = LastChar;
        while(isalnum((LastChar = fgetc(file)))) Identifier_string += LastChar;

        if (Identifier_string == "def") return DEF_TOKEN;
        return IDENTIFIER_TOKEN;
    }
    if (isdigit(LastChar)) {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = fgetc(file);
        } while(isdigit(LastChar));
        Numeric_Val = strtod(NumStr.c_str(), 0);
        return NUMERIC_TOKEN;
    }
    if (LastChar == '#') {
        do LastChar = fgetc(file);
        while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF) return gettok();
    }
    if (LastChar == EOF) return EOF_TOKEN;
    int ThisChar = LastChar;
    LastChar = fgetc(file);
    return ThisChar;

}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//
class BaseAST {
public:
    virtual ~BaseAST() {}
    virtual Value *Codegen() = 0;
};

class  VariableAST : public BaseAST {
    std::string Var_Name;
public:
    VariableAST(const std::string &name) : Var_Name(name) {}
    virtual Value *Codegen() override;
};

class NumericAST : public BaseAST {
    int Numeric_val;
public:
    NumericAST(int val) : Numeric_val(val) {}
    virtual Value *Codegen() override;
};

class BinaryAST : public BaseAST {
    std::string Bin_operator;
    BaseAST *LHS, *RHS;
public:
    BinaryAST(const std::string &op, BaseAST *lhs, BaseAST *rhs) :
        Bin_operator(op), LHS(lhs), RHS(rhs) {}
    virtual Value *Codegen() override;
};

class FunctionDeclAST : public BaseAST {
public:
    std::string Func_name;
    std::vector<std::string> Arg_names;
    FunctionDeclAST(const std::string &name, const std::vector<std::string> &args) :
        Func_name(name), Arg_names(args) {}
    virtual Value *Codegen() override;
};

class FunctionDefAST : public BaseAST {
    FunctionDeclAST *Func_Decl;
    BaseAST *Func_Body;
public:
    FunctionDefAST(FunctionDeclAST *proto, BaseAST *body) :
        Func_Decl(proto), Func_Body(body) {}
    virtual Value *Codegen() override; 
};

class FunctionCallAST : public BaseAST {
    std::string Func_Callee;
    std::vector<BaseAST*> Func_Args;
public:
    FunctionCallAST(const std::string &callee, std::vector<BaseAST*> &args) :
        Func_Callee(callee), Func_Args(args) {}
    virtual Value *Codegen() override;
};


static std::unique_ptr<LLVMContext> TheContext; // contains all the states global to the compiler
static std::unique_ptr<Module> TheModule; // contains functions and global variables
static std::unique_ptr<IRBuilder<>> Builder; // ir build keeps track of the current location for inserting new instructions
static std::map<std::string, Value*> Named_Values;
 // keeps track of which values are defined in the current scope
Value * LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
Value * LogErrorV(const char *text) {
    LogError(text);
    return 0;
}

Value *NumericAST::Codegen() {
    return ConstantInt::get(*TheContext, APInt(32, Numeric_val));
}
Value *VariableAST::Codegen() {
    Value *V = Named_Values[Var_Name];
    if (!V) return LogErrorV("Unknown variable name");
    return V;
}
Value *BinaryAST::Codegen() {
    Value *L = LHS->Codegen();
    Value *R = RHS->Codegen();
    if (!L || !R) return 0;

    switch(Bin_operator[0]) {
        case '+': return Builder->CreateAdd(L, R, "addtmp");
        case '-': return Builder->CreateSub(L, R, "subtmp");
        case '*': return Builder->CreateMul(L, R, "multmp");
        case '<':
            L = Builder->CreateICmpULT(L, R, "cmptmp");
            return Builder->CreateZExt(L, Type::getInt32Ty(*TheContext), "booltmp");
        default: return LogErrorV("invalid binary operator");
    }
}
Value *FunctionDeclAST::Codegen() {
    auto p1 = Arg_names.size();
    auto p2 = Type::getInt32Ty(*TheContext);
    std::vector<Type*> Integers(p1, p2);
    FunctionType *FT = FunctionType::get(Type::getInt32Ty(*TheContext), Integers, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, Func_name, TheModule.get());

    if (F->getName() != Func_name) {
        F->eraseFromParent();
        F = TheModule->getFunction(Func_name);

        if (!F->empty()) return LogErrorV("Function cannot be redefined");
        if (F->arg_size() != Arg_names.size()) return LogErrorV("Function argument size mismatch");
    }
    unsigned Idx = 0;
    for (Function::arg_iterator Arg_It = F->arg_begin(); Idx != Arg_names.size(); ++Arg_It, ++Idx) {
        Arg_It->setName(Arg_names[Idx]);
        Named_Values[Arg_names[Idx]] = Arg_It;
    }

    return F;
}

Value *FunctionDefAST::Codegen() {
    Named_Values.clear();

    Function *TheFunction = (Function*)Func_Decl->Codegen();
    if (TheFunction == 0) return 0;
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);
    if (Value *RetVal = Func_Body->Codegen()) {
        Builder->CreateRet(RetVal);
        verifyFunction(*TheFunction);
        return TheFunction;
    }
    TheFunction->eraseFromParent();

    return 0;
}

Value *FunctionCallAST::Codegen() {
    Function *CalleeF = TheModule->getFunction(Func_Callee);
    if (CalleeF == 0) return LogErrorV("Unknown function referenced");

    if (CalleeF->arg_size() != Func_Args.size()) return LogErrorV("Incorrect # arguments passed");

    std::vector<Value*> ArgsV;
    for (unsigned i = 0, e = Func_Args.size(); i != e; ++i) {
        ArgsV.push_back(Func_Args[i]->Codegen());
        if (ArgsV.back() == 0) return 0;
    }

    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}
//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

static std::map<char, int> Binop_Precedence;

static BaseAST* Base_Parser();
static BaseAST* Parse_Expression();

static void init_precedence() {
    Binop_Precedence['<'] = 10;
    Binop_Precedence['+'] = 20;
    Binop_Precedence['-'] = 20;
    Binop_Precedence['*'] = 40;
}

static int GetTokPrecedence() {
    if (!isascii(CurTok)) return -1;
    int TokPrec = Binop_Precedence[CurTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

static BaseAST* Parse_Numeric() {
    BaseAST *Result = new NumericAST(Numeric_Val);
    getNextToken();
    return Result;
}

static BaseAST* Parse_Bin_op(int ExprPrec, BaseAST *LHS) {
    while(1) {
        int TokPrec = GetTokPrecedence();
        if (TokPrec < ExprPrec) return LHS;

        int BinOp = CurTok;
        getNextToken();

        BaseAST *RHS = Parse_Expression();
        if (!RHS) return 0;

        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = Parse_Bin_op(TokPrec+1, RHS);
            if (RHS == 0) return 0;
        }
        LHS = new BinaryAST(std::string(1, BinOp), LHS, RHS);
    }
}


static BaseAST* Parse_Paren_Expression() {
    getNextToken();
    BaseAST *V = Parse_Expression();
    if (!V) return 0;

    if (CurTok != ')') return 0;
    getNextToken();
    return V;
}

static BaseAST* Parse_Identifier() {
    std::string IdName = Identifier_string;
    getNextToken();
    if (CurTok != '(') return new VariableAST(IdName);

    getNextToken();
    std::vector<BaseAST*> Args;
    if (CurTok != ')') {
        while(1) {
            BaseAST *Arg = Base_Parser();
            if (!Arg) return 0;
            Args.push_back(Arg);
            if (CurTok == ')') break;
            if (CurTok != ',') return 0;
            getNextToken();
        }
    }
    getNextToken();
    return new FunctionCallAST(IdName, Args);
}

static BaseAST* Base_Parser() {
    switch(CurTok) {
        case IDENTIFIER_TOKEN:
            return Parse_Identifier();
        case NUMERIC_TOKEN:
            return Parse_Numeric();
        default:
            return 0;
    }
}
static BaseAST* Parse_Expression() {
    BaseAST *LHS = Base_Parser();
    if (!LHS) return 0;

    return LHS;
}

static FunctionDeclAST* Parse_Function_Declaration() {
    getNextToken();
    if (CurTok != IDENTIFIER_TOKEN) return 0;

    std::string Fn_Name = Identifier_string;
    getNextToken();
    if (CurTok != '(') return 0;

    std::vector<std::string> Arg_Names;
    while(getNextToken() == IDENTIFIER_TOKEN) Arg_Names.push_back(Identifier_string);
    if (CurTok != ')') return 0;

    getNextToken();
    return new FunctionDeclAST(Fn_Name, Arg_Names);
}

static FunctionDefAST* Parse_Function_Definition() {
    getNextToken();
    FunctionDeclAST *Decl = Parse_Function_Declaration();
    if (Decl == 0) return 0;

    if (BaseAST *Body = Parse_Expression()) return new FunctionDefAST(Decl, Body);
    return 0;
}
static FunctionDefAST* Parse_Top_Level() {
    if (BaseAST *E = Parse_Expression()) {
        FunctionDeclAST *Func_Decl = new FunctionDeclAST("", std::vector<std::string>());
        return new FunctionDefAST(Func_Decl, E);
    }
    return 0;
}
static void Handle_Function_Definition() {
    if (FunctionDefAST *F = Parse_Function_Definition()) {
       if (Function* LF = (Function*)F->Codegen()) {}
    } else {
        getNextToken();
    }
}

static void Handle_Top_Level_Expression() {
    if (FunctionDefAST *F = Parse_Top_Level()) {
        if (Function* LF = (Function*)F->Codegen()) {}
    } else {
        getNextToken();
    }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
static void Driver() {
    init_precedence();

    while(1) {
        switch(CurTok) {
            case EOF_TOKEN:
                return;
            case ';':
                getNextToken();
                break;
            case DEF_TOKEN:
                Handle_Function_Definition();
                break;
            default:
                Handle_Top_Level_Expression();
                break;
        }
    }
}



// Avoid including "llvm-c/Core.h" for compile time, fwd-declare this instead.
//llvm  extern "C" LLVMContextRef LLVMGetGlobalContext(void);

//llvm  Module *Module_Ob;

int main(int argc, char** argv) {
    //llvm  LLVMContextRef Context = LLVMGetGlobalContext();
    file = fopen("/home/yoon/dev/0.my/my_toy/toy_front/example", "r"); //fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Can't open file\n");
        return 1;
    }

    
    getNextToken();

    //Module_Ob = new Module("my cool jit", Context);
    Driver();
    //Module_Ob->dump();

    return 0;
}
