#include "lexer.hpp"
#include <cctype>

std::vector<Token> Lexer::tokenize(const std::string& src) {
    std::vector<Token> tokens;
    size_t i = 0;
    while (i < src.size()) {
        if (isspace(src[i])) { ++i; continue; }
        if (src.compare(i, 5, "print") == 0) {
            tokens.push_back({TokenType::Print, "print"});
            i += 5;
        } else if (src.compare(i, 3, "var") == 0 && !isalnum(src[i+3])) {
            tokens.push_back({TokenType::Var, "var"});
            i += 3;
        } else if (isalpha(src[i]) || src[i] == '_') {
            size_t j = i+1;
            while (j < src.size() && (isalnum(src[j]) || src[j] == '_')) ++j;
            tokens.push_back({TokenType::Identifier, src.substr(i, j-i)});
            i = j;
        } else if (src[i] == '=') {
            tokens.push_back({TokenType::Assign, "="});
            ++i;
        } else if (isdigit(src[i])) {
            size_t j = i;
            while (j < src.size() && isdigit(src[j])) ++j;
            tokens.push_back({TokenType::Number, src.substr(i, j-i)});
            i = j;
        } else if (src[i] == '(') {
            tokens.push_back({TokenType::LParen, "("});
            ++i;
        } else if (src[i] == ')') {
            tokens.push_back({TokenType::RParen, ")"});
            ++i;
        } else if (src[i] == ';') {
            tokens.push_back({TokenType::Semicolon, ";"});
            ++i;
        } else if (src[i] == '"') {
            size_t j = i + 1;
            while (j < src.size() && src[j] != '"') ++j;
            tokens.push_back({TokenType::String, src.substr(i + 1, j - i - 1)});
            i = j + 1;
        } else {
            tokens.push_back({TokenType::Unknown, std::string(1, src[i])});
            ++i;
        }
    }
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}
