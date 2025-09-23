#include "lexer.hpp"
#include "new_parser.hpp"
std::unique_ptr<Expression> NewParser::parse_a_token() {
    const auto tok = skip_token();
    if (tok.type == TokenType::Number) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Int);
    }
    if (tok.type == TokenType::String) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::String);
    }
    if (tok.type == TokenType::Null) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Null);
    }
    if (tok.type == TokenType::True) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Bool);
    }
    if (tok.type == TokenType::False) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Int);
    }
    if (tok.type == TokenType::Identifier) {
        return std::make_unique<IdentifierExpr>(tok.text);
    }
    if (tok.type == TokenType::Lambda) {
        skip_token();
        auto param = parse_params(TokenType::RParen);
        skip_token();
        auto stmt = parse_block(true);
        return std::make_unique<LambdaDeclStmt>(param,stmt);
    }
    if (tok.type == TokenType::LBrace) {
        skip_token();
        skip_token();
    }
    return nullptr;
};

std::unique_ptr<Expression> NewParser::parse_func_call(std::unique_ptr<GetMemberStmt> node) {
    skip_token();
    auto param = parse_params(TokenType::RParen);
    skip_token();
    return std::make_unique<CallExpr>(std::move(node),std::move(param));
};

std::unique_ptr<GetMemberStmt> NewParser::parse_get_member(std::unique_ptr<GetMemberStmt> node) {
    skip_token();
    auto child = std::make_unique<IdentifierExpr>(skip_token().text);
    return std::make_unique<GetMemberStmt>(std::move(node),std::move(child));
};

std::unique_ptr<GetItemStmt> NewParser::parse_get_item(std::unique_ptr<GetMemberStmt> node) {
    skip_token();
    auto param = parse_params(TokenType::RBracket);
    skip_token();
    return std::make_unique<GetItemStmt>(std::move(node),std::move(param));
};

std::unique_ptr<Expression> NewParser::parse_func_call(std::unique_ptr<IdentifierExpr> node) {
    skip_token();
    auto param = parse_params(TokenType::RParen);
    skip_token();
    return std::make_unique<CallExpr>(std::move(node),std::move(param));
};

std::unique_ptr<GetMemberStmt> NewParser::parse_get_member(std::unique_ptr<IdentifierExpr> node) {
    skip_token();
    auto child = std::make_unique<IdentifierExpr>(skip_token().text);
    return std::make_unique<GetMemberStmt>(std::move(node),std::move(child));
};

std::unique_ptr<GetItemStmt> NewParser::parse_get_item(std::unique_ptr<IdentifierExpr> node) {
    skip_token();
    auto param = parse_params(TokenType::RBracket);
    skip_token();
    return std::make_unique<GetItemStmt>(std::move(node),std::move(param));
};

std::vector<std::unique_ptr<Expression>> NewParser::parse_params(const TokenType endswith){
    std::vector<std::unique_ptr<Expression>> params;
    while (curr_token().type != endswith) {
        params.emplace_back(parse_expression());
        if (curr_token().type == TokenType::Comma) skip_token();
    }
    return params;
};