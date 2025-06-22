#pragma once
#include "lexer.hpp"
#include "ast.hpp"
#include <vector>

class Parser {
public:
    static std::unique_ptr<ASTNode> parse(const std::vector<Token>& tokens);
    static std::unique_ptr<ASTNode> parse_expression(const std::vector<Token>& tokens, size_t& i);
};
