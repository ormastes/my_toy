#pragma once

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