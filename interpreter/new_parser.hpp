#pragma once
#include "ast.hpp"


enum class TokenType;
struct Token;

class LAMINA_API NewParser {
private:
    const std::vector<Token> tokens_;
    int curr_tok_idx_ = 0;

public:
    explicit NewParser(const std::vector<Token>& tokens);
    ~NewParser() = default;
    Token skip_token();
    [[nodiscard]] Token curr_token() const;
    void must_token(const std::string& text, const std::string& waring) const;
    std::vector<std::unique_ptr<ASTNode>> parse_program();
    std::unique_ptr<Statement> parse_stmt();

    std::unique_ptr<Expression> parse_expression();
    // parse expr
    std::unique_ptr<Expression> parse_comparison();
    std::unique_ptr<Expression> parse_add_sub();
    std::unique_ptr<Expression> parse_mul_div_mod();
    std::unique_ptr<Expression> parse_power();
    std::unique_ptr<Expression> parse_unary();
    std::unique_ptr<Expression> parse_factor();

    // parse factor
    std::unique_ptr<Expression> parse_a_token();
    std::unique_ptr<Expression> parse_func_call(std::unique_ptr<IdentifierExpr> node);
    std::unique_ptr<GetMemberStmt> parse_get_member(std::unique_ptr<IdentifierExpr> node);
    std::unique_ptr<GetItemStmt> parse_get_item(std::unique_ptr<IdentifierExpr> node);
    // 重载版本
    std::unique_ptr<Expression> parse_func_call(std::unique_ptr<GetMemberStmt> node);
    std::unique_ptr<GetMemberStmt> parse_get_member(std::unique_ptr<GetMemberStmt> node);
    std::unique_ptr<GetItemStmt> parse_get_item(std::unique_ptr<GetMemberStmt> node);
    std::vector<std::unique_ptr<Expression>> NewParser::parse_params(TokenType endswith);

    std::unique_ptr<BlockStmt> parse_block(bool is_global);

    // parse stmt
    std::unique_ptr<Statement> parse_if();
    std::unique_ptr<Statement> parse_func();
    std::unique_ptr<Statement> parse_var();
    std::unique_ptr<Statement> parse_ass();
    std::unique_ptr<Statement> parse_struct();
    std::unique_ptr<Statement> parse_while();
};
