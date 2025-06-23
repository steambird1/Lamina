#pragma once
#include "lexer.hpp"
#include "ast.hpp"
#include <vector>

class Parser {
public:
    static std::unique_ptr<ASTNode> parse(const std::vector<Token>& tokens);
    static std::unique_ptr<Expression> parse_expression(const std::vector<Token>& tokens, size_t& i);
    // 解析不同层级的表达式，处理正确的运算符优先级
    static std::unique_ptr<Expression> parse_comparison(const std::vector<Token>& tokens, size_t& i);
    static std::unique_ptr<Expression> parse_addition(const std::vector<Token>& tokens, size_t& i);
    static std::unique_ptr<Expression> parse_term(const std::vector<Token>& tokens, size_t& i);
    static std::unique_ptr<Expression> parse_power(const std::vector<Token>& tokens, size_t& i);
    static std::unique_ptr<Expression> parse_unary(const std::vector<Token>& tokens, size_t& i);
    static std::unique_ptr<Expression> parse_primary(const std::vector<Token>& tokens, size_t& i);
    // 解析一组语句，返回 BlockStmt
    static std::unique_ptr<BlockStmt> parse_block(const std::vector<Token>& tokens, size_t& i);
    // 解析单条语句
    static std::unique_ptr<Statement> parse_statement(const std::vector<Token>& tokens, size_t& i);
};
