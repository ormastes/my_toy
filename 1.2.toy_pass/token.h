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
