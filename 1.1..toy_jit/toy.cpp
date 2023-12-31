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

// for jit support
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

#include "Interpreter.h"

//llvm #include "llvm/IR/LLVMContext.h"

//using namespace llvm;
//using namespace llvm::orc;
// clang++-16 -I ~/dev/0.my/play_llvm_x86_linux_install/include toy.cpp -o toy
static llvm::ExitOnError ExitOnErr;
enum Token_Type {
    tok_eof = -1,
    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
    
    tok_paren,
    
};
static int NumVal;
static std::string IdentifierStr;
FILE* file;

static int gettok() {
    static int LastChar = ' ';
    // skip any whitespace
    while(isspace(LastChar)) LastChar = fgetc(file);
    // identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LastChar)) {
        IdentifierStr = LastChar;
        while(isalnum((LastChar = fgetc(file)))) IdentifierStr += LastChar;

        if (IdentifierStr == "def") return tok_def;
        if (IdentifierStr == "extern") return tok_extern;
        return tok_identifier;
    }
    // number: [0-9.]+ 
    if (isdigit(LastChar)) {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = fgetc(file);
        } while(isdigit(LastChar));
        NumVal = strtod(NumStr.c_str(), 0);
        return tok_number;
    }
    // comment until end of line
    if (LastChar == '#') {
        do LastChar = fgetc(file);
        while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF) return gettok();
    }
    // end of file
    if (LastChar == EOF) return tok_eof;
    // otherwise just return the character as its ascii value
    int ThisChar = LastChar;
    // update LastChar
    LastChar = fgetc(file);
    return ThisChar;

}

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

static std::map<std::string, std::unique_ptr<FunctionPrototypeAST>> FunctionProtos;

static std::unique_ptr<llvm::LLVMContext> TheContext; // contains all the states global to the compiler
static std::unique_ptr<llvm::Module> TheModule{}; // contains functions and global variables //{} to ensure nullpt init
static std::unique_ptr<llvm::IRBuilder<>> Builder; // ir build keeps track of the current location for inserting new instructions
static std::map<std::string, llvm::Value*> Named_Values;

static std::unique_ptr<Interpreter> TheJIT; // for jit support

static void  PrintModule() {
    // Print out all of the generated code.
    TheModule->print(llvm::errs(), nullptr);
}
llvm::Function *getFunction(std::string Name) {
  // First, see if the function has already been added to the current module.
  if (auto *F = TheModule->getFunction(Name))
    return F;

  // If not, check whether we can codegen the declaration from some existing
  // prototype.
  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end())
    return FI->second->Codegen();

  // If no existing prototype exists, return null.
  return nullptr;
}
 // keeps track of which values are defined in the current scope
static void InitializeModule() {
    // Open a new context and module.
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("my jit toy", *TheContext);
    TheModule->setDataLayout(TheJIT->getDataLayout());// getTargetMachine().createDataLayout());
    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

llvm::Value * LogError(const char *text) {
    fprintf(stderr, "Error: %s\n", text);
    return nullptr;
}
llvm::Function * LogErrorF(const char *text) {
    LogError(text);
    return nullptr;
}
std::unique_ptr<ExprAST> LogErrorEA(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<FunctionAST>  LogErrorFA(const char *text) {
    LogError(text);
    return nullptr;
}

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
//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

static std::map<char, int> Binop_Precedence;

static std::unique_ptr<ExprAST> Parse_Primary();
static std::unique_ptr<ExprAST> Parse_Expression();
static std::unique_ptr<FunctionPrototypeAST> Parse_Function_Prototype();

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

// numberexpr ::= number
static std::unique_ptr<ExprAST> Parse_Numeric() {
    auto Result =  std::make_unique<NumericExprAST>(NumVal);
    getNextToken();
    return Result;
}

// binoprhs ::= ('+' primary)*
static std::unique_ptr<ExprAST> Parse_Bin_op(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    while(1) {
        int TokPrec = GetTokPrecedence();
        if (TokPrec < ExprPrec) return LHS;

        int BinOp = CurTok;
        getNextToken();

        auto RHS = Parse_Primary();
        if (!RHS) return 0;

        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = Parse_Bin_op(TokPrec+1, std::move(RHS));
            if (RHS == 0) return 0;
        }
        LHS = std::make_unique<BinaryExprAST>(std::string(1, BinOp),
             std::move(LHS), std::move(RHS));
    }
}

// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> Parse_Paren_Expression() {
    getNextToken();
    auto V = Parse_Expression();
    if (!V) return 0;

    if (CurTok != ')') return LogErrorEA("Expected ')'");
    getNextToken();
    return V;
}

// identifierexpr ::= identifier | identifier '(' expression* ')'
static std::unique_ptr<ExprAST> Parse_Identifier() {
    std::string IdName = IdentifierStr;
    getNextToken();
    if (CurTok != '(') return std::make_unique<VariableExprAST>(IdName);

    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while(1) {
            std::unique_ptr<ExprAST> Arg = Parse_Primary();
            if (!Arg) return LogErrorEA("Expected expression after open parenthesis ')'");
            Args.push_back(std::move(Arg));
            if (CurTok == ')') break;
            if (CurTok != ',') return LogErrorEA("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }
    getNextToken();
    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

// primary ::= identifierexpr|numberexpr|parenexpr
static std::unique_ptr<ExprAST> Parse_Primary() {
    switch(CurTok) {
        case tok_identifier:
            return Parse_Identifier();
        case tok_number:
            return Parse_Numeric();
        case '(':
            return Parse_Paren_Expression();
        default:
            return 0;
    }
}

// expression ::= primary binoprhs
static std::unique_ptr<ExprAST> Parse_Expression() {
    std::unique_ptr<ExprAST> LHS = Parse_Primary();
    if (!LHS) return 0;

    return Parse_Bin_op(0, std::move(LHS));
}

//external ::= 'extern' prototype
static std::unique_ptr<FunctionPrototypeAST> Parse_Extern() {
    getNextToken();
    return Parse_Function_Prototype();
}

//prototype ::= id '(' id* ')'
static  std::unique_ptr<FunctionPrototypeAST>  Parse_Function_Prototype() {
    if (CurTok != tok_identifier) return 0;

    std::string Fn_Name = IdentifierStr;
    getNextToken();
    if (CurTok != '(') return 0;

    std::vector<std::string> Arg_Names;
    while(getNextToken() == tok_identifier) Arg_Names.push_back(IdentifierStr);
    if (CurTok != ')') return 0;

    getNextToken();
    return std::make_unique<FunctionPrototypeAST>(Fn_Name, Arg_Names);
}

static std::unique_ptr<FunctionAST> Parse_Function_Definition() {
    getNextToken();
    auto Decl = Parse_Function_Prototype();
    if (Decl == 0) return 0;

    if (auto Body = Parse_Expression()) return std::make_unique<FunctionImplAST>(std::move(Decl), std::move(Body));
    return 0;
}
static std::unique_ptr<FunctionImplAST> Parse_Top_Level() {
    if (auto E = Parse_Expression()) {// Make an anonymous proto.
        auto Proto = std::make_unique<FunctionPrototypeAST>("__anon_expr", std::vector<std::string>());
        return std::make_unique< FunctionImplAST>(std::move(Proto), std::move(E));
    }
    return 0;
}
static void Handle_Function_Definition() {
    if (auto F = Parse_Function_Definition()) {
       if (auto LF = F->Codegen()) {}
    } else {
        getNextToken();
    }
}

static void Handle_Extern() {
    if (auto F = Parse_Extern()) {
        if (auto LF = F->Codegen()) {
            FunctionProtos[F->Func_name] = std::move(F);
        }
    } else {
        getNextToken();
    }
}

static void Handle_Top_Level_Expression() {
    if (auto F = Parse_Top_Level()) {
        if (auto LF = F->Codegen()) {
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
    } else {
        getNextToken();
    }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
static void Driver() {
    init_precedence();

    getNextToken();

    while(1) {
        switch(CurTok) {
            case tok_eof:
                return;
            case ';':
                getNextToken();
                break;
            case tok_def:
                Handle_Function_Definition();
                break;
            case tok_extern:
                Handle_Extern();
                break;
            default:
                Handle_Top_Level_Expression();
                break;
        }
    }
}


//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a int and returns 0.
extern "C" DLLEXPORT int putchard(int X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a int prints it as "%d\n", returning 0.
extern "C" DLLEXPORT int printd(int X) {
  fprintf(stderr, "%d\n", X);
  return 0;
}
// Avoid including "llvm-c/Core.h" for compile time, fwd-declare this instead.
//llvm  extern "C" LLVMContextRef LLVMGetGlobalContext(void);

//llvm  Module *Module_Ob;

int main(int argc, char** argv) {
    // select the target // jit need target selection
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    TheJIT = ExitOnErr(Interpreter::Create());;
    InitializeModule();
    //llvm  LLVMContextRef Context = LLVMGetGlobalContext();
    file = fopen("/home/yoon/dev/0.my/my_toy/toy_jit/example", "r"); //fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Can't open file\n");
        return 1;
    }

    //Module_Ob = new Module("my cool jit", Context);
    Driver();
    
    PrintModule();

    return 0;
}
