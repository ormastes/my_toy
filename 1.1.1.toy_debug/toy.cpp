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

std::string getTokName(int Tok) {
  switch (Tok) {
  case tok_eof:
    return "eof";
  case tok_def:
    return "def";
  case tok_extern:
    return "extern";
  case tok_identifier:
    return "identifier";
  case tok_number:
    return "number";
  case tok_if:
    return "if";
  case tok_then:
    return "then";
  case tok_else:
    return "else";
  case tok_for:
    return "for";
  case tok_in:
    return "in";
  case tok_binary:
    return "binary";
  case tok_unary:
    return "unary";
  case tok_var:
    return "var";
  }
  return std::string(1, (char)Tok);
}

namespace {
class PrototypeAST;
class ExprAST;
}

struct DebugInfo {
  DICompileUnit *TheCU;
  DIType *DblTy;
  std::vector<DIScope *> LexicalBlocks;

  void emitLocation(ExprAST *AST);
  DIType *getDoubleTy();
} KSDbgInfo;

struct SourceLocation {
  int Line;
  int Col;
};
static SourceLocation CurLoc;
static SourceLocation LexLoc = {1, 0};

static int advance() {
  int LastChar =  fgetc(file);

  if (LastChar == '\n' || LastChar == '\r') {
    LexLoc.Line++;
    LexLoc.Col = 0;
  } else
    LexLoc.Col++;
  return LastChar;
}


static int gettok() {
    static int LastChar = ' ';
    // skip any whitespace
    while(isspace(LastChar)) LastChar = advance;
    // identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LastChar)) {
        IdentifierStr = LastChar;
        while(isalnum((LastChar = advance))) IdentifierStr += LastChar;

        if (IdentifierStr == "def") return tok_def;
        if (IdentifierStr == "extern") return tok_extern;
        return tok_identifier;
    }
    // number: [0-9.]+ 
    if (isdigit(LastChar)) {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = advance;
        } while(isdigit(LastChar));
        NumVal = strtod(NumStr.c_str(), 0);
        return tok_number;
    }
    // comment until end of line
    if (LastChar == '#') {
        do LastChar = advance;
        while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF) return gettok();
    }
    // end of file
    if (LastChar == EOF) return tok_eof;
    // otherwise just return the character as its ascii value
    int ThisChar = LastChar;
    // update LastChar
    LastChar = advance;
    return ThisChar;

}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//


raw_ostream &indent(raw_ostream &O, int size) {
  return O << std::string(size, ' ');
}

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
    TheModule = std::make_unique<llvm::Module>("my toy debug", *TheContext);

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




llvm::Value *NumericExprAST::Codegen() {
      KSDbgInfo.emitLocation(this);
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(32, Val));
}
llvm::Value *VariableExprAST::Codegen() {
    llvm::Value *V = Named_Values[Name];
    if (!V) return LogError("Unknown variable name");
    KSDbgInfo.emitLocation(this);
    return  Builder->CreateLoad(Type::getDoubleTy(*TheContext), V, Name.c_str());
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
  DIFile *Unit = DBuilder->createFile(KSDbgInfo.TheCU->getFilename(),
                                      KSDbgInfo.TheCU->getDirectory());
  DIScope *FContext = Unit;
  unsigned LineNo = P.getLine();
  unsigned ScopeLine = LineNo;
  DISubprogram *SP = DBuilder->createFunction(
      FContext, P.getName(), StringRef(), Unit, LineNo,
      CreateFunctionType(TheFunction->arg_size()), ScopeLine,
      DINode::FlagPrototyped, DISubprogram::SPFlagDefinition);
  TheFunction->setSubprogram(SP);

  // Push the current scope.
  KSDbgInfo.LexicalBlocks.push_back(SP);

  // Unset the location for the prologue emission (leading instructions with no
  // location in a function are considered part of the prologue and the debugger
  // will run past them when breaking on a function)
  KSDbgInfo.emitLocation(nullptr);

// Record the function arguments in the NamedValues map.
  NamedValues.clear();
  unsigned ArgIdx = 0;
  for (auto &Arg : TheFunction->args()) {
    // Create an alloca for this variable.
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

    // Create a debug descriptor for the variable.
    DILocalVariable *D = DBuilder->createParameterVariable(
        SP, Arg.getName(), ++ArgIdx, Unit, LineNo, KSDbgInfo.getDoubleTy(),
        true);

    DBuilder->insertDeclare(Alloca, D, DBuilder->createExpression(),
                            DILocation::get(SP->getContext(), LineNo, 0, SP),
                            Builder->GetInsertBlock());

    // Store the initial value into the alloca.
    Builder->CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    NamedValues[std::string(Arg.getName())] = Alloca;
  }

  KSDbgInfo.emitLocation(Body.get());

  if (Value *RetVal = Body->codegen()) {
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
        SourceLocation BinLoc = CurLoc;
        getNextToken();

        auto RHS = Parse_Primary();
        if (!RHS) return 0;

        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = Parse_Bin_op(TokPrec+1, std::move(RHS));
            if (RHS == 0) return 0;
        }
        LHS = std::make_unique<BinaryExprAST>(BinLoc, std::string(1, BinOp),
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
    
    SourceLocation LitLoc = CurLoc;
    getNextToken();
    if (CurTok != '(') return std::make_unique<VariableExprAST>(LitLoc, IdName);

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
    return std::make_unique<CallExprAST>(LitLoc, IdName, std::move(Args));
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
static std::unique_ptr<FunctionAST> Parse_Extern() {
    getNextToken();
    return Parse_Function_Prototype();
}

//prototype ::= id '(' id* ')'
static  std::unique_ptr<FunctionPrototypeAST>  Parse_Function_Prototype() {
    if (CurTok != tok_identifier) return 0;

    std::string Fn_Name = IdentifierStr;
    SourceLocation FnLoc = CurLoc;
    getNextToken();
    if (CurTok != '(') return 0;

    std::vector<std::string> Arg_Names;
    while(getNextToken() == tok_identifier) Arg_Names.push_back(IdentifierStr);
    if (CurTok != ')') return 0;

    getNextToken();
    return std::make_unique<FunctionPrototypeAST>(FnLoc, Fn_Name, Arg_Names);
}

static std::unique_ptr<FunctionAST> Parse_Function_Definition() {
    getNextToken();
    auto Decl = Parse_Function_Prototype();
    if (Decl == 0) return 0;

    if (auto Body = Parse_Expression()) return std::make_unique<FunctionImplAST>(std::move(Decl), std::move(Body));
    return 0;
}
static std::unique_ptr<FunctionImplAST> Parse_Top_Level() {
    SourceLocation FnLoc = CurLoc;
    if (auto E = Parse_Expression()) {// Make an anonymous proto.
        auto Proto = std::make_unique<PrototypeAST>(FnLoc, "main", std::vector<std::string>());
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



// Avoid including "llvm-c/Core.h" for compile time, fwd-declare this instead.
//llvm  extern "C" LLVMContextRef LLVMGetGlobalContext(void);

//llvm  Module *Module_Ob;

int main(int argc, char** argv) {
    InitializeModule();
    //llvm  LLVMContextRef Context = LLVMGetGlobalContext();
    file = fopen("/home/yoon/dev/0.my/my_toy/toy_debug/example", "r"); //fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Can't open file\n");
        return 1;
    }

    
  // Add the current debug info version into the module.
  TheModule->addModuleFlag(Module::Warning, "Debug Info Version",
                           DEBUG_METADATA_VERSION);

  // Darwin only supports dwarf2.
  if (Triple(sys::getProcessTriple()).isOSDarwin())
    TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

  // Construct the DIBuilder, we do this here because we need the module.
  DBuilder = std::make_unique<DIBuilder>(*TheModule);

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  KSDbgInfo.TheCU = DBuilder->createCompileUnit(
      dwarf::DW_LANG_C, DBuilder->createFile("fib.ks", "."),
      "Kaleidoscope Compiler", false, "", 0);

    //Module_Ob = new Module("my cool jit", Context);
    Driver();
    
    PrintModule();

    return 0;
}
