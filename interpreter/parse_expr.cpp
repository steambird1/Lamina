#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

std::unique_ptr<Expression> Parser::parse_expression() {
    return parse_comparison();
}

std::unique_ptr<Expression> Parser::parse_comparison() {
    auto node = parse_add_sub();
    while (
        curr_token().type == TokenType::Equal
        or curr_token().type == TokenType::NotEqual
        or curr_token().type == TokenType::Greater
        or curr_token().type == TokenType::Less
        or curr_token().type == TokenType::GreaterEqual
        or curr_token().type == TokenType::LessEqual
        or curr_token().type == TokenType::Assign
    ) {
        if (curr_token().type == TokenType::Assign) throw RuntimeError("'==' can not use in expression, Maybe you mean '=='");
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_add_sub();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_add_sub() {
    auto node = parse_mul_div_mod();
    while (
        curr_token().type == TokenType::Plus
        or curr_token().type == TokenType::Minus
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_mul_div_mod();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_mul_div_mod() {
    auto node = parse_power();
    while (
        curr_token().type == TokenType::Star
        or curr_token().type == TokenType::Slash
        or curr_token().type == TokenType::Percent
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_power() {
    auto node = parse_unary();
    if (curr_token().type == TokenType::Caret) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();  // 右结合
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_unary() {
    if (curr_token().type == TokenType::Minus) {
        auto tok = curr_token();
        auto operand = parse_unary();
        return std::make_unique<UnaryExpr>("-", std::move(operand));
    }
    return parse_factor();
}

std::unique_ptr<Expression> Parser::parse_factor() {
    auto node = parse_a_token();

    while (true) {
        if (curr_token().type == TokenType::ExclamationMark) {
            skip_token("!");
            node = std::make_unique<UnaryExpr>("!", std::move(node));
        }
        if (curr_token().type == TokenType::Dot) {
            node = parse_get_member(std::move(node));
        }
        else if (curr_token().type == TokenType::DoubleColon) {
            node = parse_namespace_get_member(std::move(node));
        }
        else if (curr_token().type == TokenType::LBracket) {
            node = parse_get_item(std::move(node));
        }
        else if (curr_token().type == TokenType::LParen) {
            node = parse_func_call(std::move(node));
        }
        else {
            break;
        }
    }
    return node;
}
