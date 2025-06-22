#include "parser.hpp"
#include <memory>

std::unique_ptr<ASTNode> Parser::parse_expression(const std::vector<Token>& tokens, size_t& i) {
    if (tokens[i].type == TokenType::String) {
        return std::make_unique<LiteralNode>(tokens[i++].text);
    } else if (tokens[i].type == TokenType::Number) {
        return std::make_unique<LiteralNode>(tokens[i++].text);
    } else if (tokens[i].type == TokenType::Identifier) {
        return std::make_unique<IdentifierNode>(tokens[i++].text);
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parse(const std::vector<Token>& tokens) {
    size_t i = 0;
    if (tokens[i].type == TokenType::Print &&
        tokens[i+1].type == TokenType::LParen) {
        i += 2;
        auto expr = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::RParen && tokens[i+1].type == TokenType::Semicolon) {
            return std::make_unique<PrintNode>(std::move(expr));
        }
    } else if (tokens[i].type == TokenType::Var &&
               tokens[i+1].type == TokenType::Identifier &&
               tokens[i+2].type == TokenType::Assign) {
        std::string name = tokens[i+1].text;
        i += 3;
        auto expr = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::Semicolon) {
            return std::make_unique<VarDeclNode>(name, std::move(expr));
        }
    }
    return nullptr;
}
