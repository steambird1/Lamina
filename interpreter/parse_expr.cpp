#include "lexer.hpp"
#include "new_parser.hpp"

#include <complex.h>
std::unique_ptr<Expression> NewParser::parse_expression() {

}

std::unique_ptr<Expression> NewParser::parse_comparison() {

}

std::unique_ptr<Expression> NewParser::parse_add_sub() {

}

std::unique_ptr<Expression> NewParser::parse_mul_div_mod() {

}

std::unique_ptr<Expression> NewParser::parse_power() {

}

std::unique_ptr<Expression> NewParser::parse_unary() {

}

std::unique_ptr<Expression> NewParser::parse_factor() {
    auto node = parse_a_token();

    while (curr_token().type != TokenType::EndOfFile) {
        if (curr_token().type == TokenType::Dot) {
            node = parse_get_member(std::move(node));
        }
        if (curr_token().type == TokenType::LParen) {
            node = parse_get_item(std::move(node));
        }
        if (curr_token().type == TokenType::LBracket) {
            node = parse_func_call(std::move(node));
        }
    }
    return node;
}
