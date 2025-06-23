#pragma once
#include <string>
#include <vector>

enum class TokenType {
    Print,
    Var,
    Func,      
    If,        
    Else,      
    While,     
    For,       
    Return,    
    Include,   // 新增
    Break,     // 新增
    Continue,  // 新增
    Identifier,
    Assign,
    Number,
    LParen,
    RParen,
    LBrace,    
    RBrace,    
    Comma,     
    Dot,       // 新增
    String,
    Semicolon,
    Plus,    
    Minus,   
    Star,    
    Slash,   
    DoubleSlash, 
    Percent,     
    Caret,       
    Bang,        
    EndOfFile,
    Unknown
};

struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;
    Token(TokenType t, const std::string& txt, int l, int c)
        : type(t), text(txt), line(l), column(c) {}
};

class Lexer {
public:
    static std::vector<Token> tokenize(const std::string& src);
};
