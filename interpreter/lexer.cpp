#include "lexer.hpp"
#include <cctype>

std::vector<Token> Lexer::tokenize(const std::string& src) {
    std::vector<Token> tokens;
    size_t i = 0;
    int line = 1, col = 1;
    while (i < src.size()) {
        if (src[i] == '\n') { ++line; col = 1; ++i; continue; }
        if (isspace(src[i])) { ++col; ++i; continue; }
        size_t start_col = col;
        if (src.compare(i, 5, "print") == 0) {
            tokens.push_back(Token(TokenType::Print, "print", line, start_col));
            i += 5; col += 5;
        } else if (src.compare(i, 3, "var") == 0 && !isalnum(src[i+3])) {
            tokens.push_back(Token(TokenType::Var, "var", line, start_col));
            i += 3; col += 3;
        } else if (src.compare(i, 4, "func") == 0 && !isalnum(src[i+4])) {
            tokens.push_back(Token(TokenType::Func, "func", line, start_col));
            i += 4; col += 4;
        } else if (src.compare(i, 2, "if") == 0 && !isalnum(src[i+2])) {
            tokens.push_back(Token(TokenType::If, "if", line, start_col));
            i += 2; col += 2;
        } else if (src.compare(i, 4, "else") == 0 && !isalnum(src[i+4])) {
            tokens.push_back(Token(TokenType::Else, "else", line, start_col));
            i += 4; col += 4;
        } else if (src.compare(i, 5, "while") == 0 && !isalnum(src[i+5])) {
            tokens.push_back(Token(TokenType::While, "while", line, start_col));
            i += 5; col += 5;
        } else if (src.compare(i, 3, "for") == 0 && !isalnum(src[i+3])) {
            tokens.push_back(Token(TokenType::For, "for", line, start_col));
            i += 3; col += 3;
        } else if (src.compare(i, 6, "return") == 0 && !isalnum(src[i+6])) {
            tokens.push_back(Token(TokenType::Return, "return", line, start_col));
            i += 6; col += 6;
        } else if (src.compare(i, 7, "include") == 0 && !isalnum(src[i+7])) {
            tokens.push_back(Token(TokenType::Include, "include", line, start_col));
            i += 7; col += 7;
        } else if (isalpha(src[i]) || src[i] == '_') {
            size_t j = i+1;
            while (j < src.size() && (isalnum(src[j]) || src[j] == '_')) ++j;
            tokens.push_back(Token(TokenType::Identifier, src.substr(i, j-i), line, start_col));
            col += (j-i); i = j;
        } else if (src[i] == '=') {
            tokens.push_back(Token(TokenType::Assign, "=", line, start_col));
            ++i; ++col;
        } else if (isdigit(src[i])) {
            size_t j = i;
            while (j < src.size() && isdigit(src[j])) ++j;
            tokens.push_back(Token(TokenType::Number, src.substr(i, j-i), line, start_col));
            col += (j-i); i = j;
        } else if (src[i] == '(') {
            tokens.push_back(Token(TokenType::LParen, "(", line, start_col));
            ++i; ++col;
        } else if (src[i] == ')') {
            tokens.push_back(Token(TokenType::RParen, ")", line, start_col));
            ++i; ++col;
        } else if (src[i] == ';') {
            tokens.push_back(Token(TokenType::Semicolon, ";", line, start_col));
            ++i; ++col;
        } else if (src[i] == '"') {
            size_t j = i + 1;
            while (j < src.size() && src[j] != '"') ++j;
            tokens.push_back(Token(TokenType::String, src.substr(i + 1, j - i - 1), line, start_col));
            col += (j-i+1); i = j + 1;
        } else if (src[i] == '+') {
            tokens.push_back(Token(TokenType::Plus, "+", line, start_col));
            ++i; ++col;
        } else if (src[i] == '-') {
            tokens.push_back(Token(TokenType::Minus, "-", line, start_col));
            ++i; ++col;
        } else if (src[i] == '*') {
            tokens.push_back(Token(TokenType::Star, "*", line, start_col));
            ++i; ++col;
        } else if (src[i] == '/') {
            tokens.push_back(Token(TokenType::Slash, "/", line, start_col));
            ++i; ++col;
        } else if (src[i] == '/' && src[i+1] == '/') {
            tokens.push_back(Token(TokenType::DoubleSlash, "//", line, start_col));
            i += 2; col += 2;
        } else if (src[i] == '%') {
            tokens.push_back(Token(TokenType::Percent, "%", line, start_col));
            ++i; ++col;
        } else if (src[i] == '^') {
            tokens.push_back(Token(TokenType::Caret, "^", line, start_col));
            ++i; ++col;
        } else if (src[i] == '!') {
            tokens.push_back(Token(TokenType::Bang, "!", line, start_col));
            ++i; ++col;
        } else if (src[i] == '{') {
            tokens.push_back(Token(TokenType::LBrace, "{", line, start_col));
            ++i; ++col;
        } else if (src[i] == '}') {
            tokens.push_back(Token(TokenType::RBrace, "}", line, start_col));
            ++i; ++col;
        } else if (src[i] == ',') {
            tokens.push_back(Token(TokenType::Comma, ",", line, start_col));
            ++i; ++col;
        } else if (src[i] == '.') {
            tokens.push_back(Token(TokenType::Dot, ".", line, start_col));
            ++i; ++col;
        } else {
            tokens.push_back(Token(TokenType::Unknown, std::string(1, src[i]), line, start_col));
            ++i; ++col;
        }
    }
    tokens.push_back(Token(TokenType::EndOfFile, "", line, col));
    return tokens;
}
