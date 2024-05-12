#pragma once
#include <string>
#include <vector>
#include "llvm/IR/DIBuilder.h"
#include "common.h"

enum Token_Type {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  // control
  tok_if = -6,
  tok_then = -7,
  tok_else = -8,
  tok_for = -9,
  tok_in = -10,

  // operators
  tok_binary = -11,
  tok_unary = -12,

  // var definition
  tok_var = -13
};

extern int NumVal;
extern std::string IdentifierStr;
extern FILE* file;

std::string getTokName(int Tok) ;


class PrototypeAST;
class ExprAST;

struct DebugInfo {
  llvm::DICompileUnit *TheCU;
  llvm::DIType *DblTy;
  std::vector<llvm::DIScope *> LexicalBlocks;

  void emitLocation(ExprAST *AST);
  llvm::DIType *getDoubleTy();
};

extern DebugInfo KSDbgInfo;


extern SourceLocation CurLoc;
extern SourceLocation LexLoc;

int advance();


int gettok();