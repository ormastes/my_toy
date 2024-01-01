#pragma once


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
            variable_Handle_Top_Level_Expression();
        }
    } else {
        getNextToken();
    }
}
