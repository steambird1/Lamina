#pragma once
#include "ast.hpp"
#include "lexer.hpp"


enum class TokenType;
struct Token;

class LAMINA_API Parser {
    const std::vector<Token> tokens_;
    long long unsigned int curr_tok_idx_ = 0;

public:
    explicit Parser(const std::vector<Token>& tokens);
    ~Parser() = default;
    Token skip_token(const std::string& want_skip = "");
    void skip_end_of_ln();
    [[nodiscard]] Token curr_token() const;
    void must_token(const std::string& text, const std::string& waring) const;
    std::vector<std::unique_ptr<Statement>> parse_program();
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
    std::unique_ptr<Expression> parse_func_call(std::unique_ptr<Expression> node);
    std::unique_ptr<NameSpaceGetMemberExpr> parse_namespace_get_member(std::unique_ptr<Expression> node);
    std::unique_ptr<GetMemberExpr> parse_get_member(std::unique_ptr<Expression> node);
    std::unique_ptr<GetItemExpr> parse_get_item(std::unique_ptr<Expression> node);
    std::vector<std::unique_ptr<Expression>> parse_params(TokenType endswith);

    std::unique_ptr<BlockStmt> parse_block(bool is_global);

    // parse stmt
    std::unique_ptr<Statement> parse_if();
    std::unique_ptr<Statement> parse_func();
    std::unique_ptr<Statement> parse_var();
    std::unique_ptr<Statement> parse_struct();
    std::unique_ptr<Statement> parse_while();
};
