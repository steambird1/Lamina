#pragma once
#include <string>
#include <vector>

enum class TokenType {
    Print,
    Var,
    Identifier,
    Assign,
    Number,
    LParen,
    RParen,
    String,
    Semicolon,
    EndOfFile,
    Unknown
};

struct Token {
    TokenType type;
    std::string text;
};

class Lexer {
public:
    static std::vector<Token> tokenize(const std::string& src);
};
