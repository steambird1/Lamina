#include "lexer.hpp"
#include "parser.hpp"
std::unique_ptr<Expression> Parser::parse_a_token() {
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
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Bool);
    }
    if (tok.type == TokenType::Identifier) {
        return std::make_unique<IdentifierExpr>(tok.text);
    }
    if (tok.type == TokenType::Lambda) {
        skip_token("(");
        std::vector<std::string> params;
        while (curr_token().type != TokenType::RParen) {
            params.emplace_back(skip_token().text);
            if (curr_token().type == TokenType::Comma) skip_token(",");
        }
        skip_token(")");

        skip_token("{");
        auto stmt = parse_block(true);
        skip_token("}");
        return std::make_unique<LambdaDeclExpr>(std::move(params),std::move(stmt));
    }
    if (tok.type == TokenType::LBrace) {
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> init_vec{};
        skip_token("{");
        while (curr_token().type != TokenType::RBrace) {
            auto key = skip_token().text;
            skip_token("=");
            auto val = parse_expression();
            init_vec.emplace_back(std::move(key), std::move(val));
        }
        skip_token("}");
        return std::make_unique<LambdaStructDeclExpr>(std::move(init_vec));
    }
    if (tok.type == TokenType::LBracket) {
        skip_token("[");
        auto param = parse_params(TokenType::RBracket);
        skip_token("]");
        return std::make_unique<ArrayExpr>(std::move(param));
    }
    if (tok.type == TokenType::LParen) {
        skip_token("(");
        auto expr = parse_expression();
        skip_token(")");
        return expr;
    }
    return nullptr;
}

std::unique_ptr<Expression> Parser::parse_func_call(std::unique_ptr<Expression> node) {
    skip_token("(");
    auto param = parse_params(TokenType::RParen);
    skip_token(")");
    return std::make_unique<CallExpr>(std::move(node),std::move(param));
}

std::unique_ptr<GetMemberExpr> Parser::parse_get_member(std::unique_ptr<Expression> node) {
    skip_token(".");
    auto child = std::make_unique<IdentifierExpr>(skip_token().text);
    return std::make_unique<GetMemberExpr>(std::move(node),std::move(child));
}

std::unique_ptr<NameSpaceGetMemberExpr> Parser::parse_namespace_get_member(std::unique_ptr<Expression> node) {
    skip_token("::");
    auto child = std::make_unique<IdentifierExpr>(skip_token().text);
    return std::make_unique<NameSpaceGetMemberExpr>(std::move(node),std::move(child));
}

std::unique_ptr<GetItemExpr> Parser::parse_get_item(std::unique_ptr<Expression> node) {
    skip_token("[");
    auto param = parse_params(TokenType::RBracket);
    skip_token("]");
    return std::make_unique<GetItemExpr>(std::move(node),std::move(param));
}

std::vector<std::unique_ptr<Expression>> Parser::parse_params(const TokenType endswith){
    std::vector<std::unique_ptr<Expression>> params;
    while (curr_token().type != endswith) {
        params.emplace_back(parse_expression());
        if (curr_token().type == TokenType::Comma) skip_token(",");
    }
    return params;
}