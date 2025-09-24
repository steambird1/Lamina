#include "lexer.hpp"
#include "parser.hpp"
std::unique_ptr<BlockStmt> Parser::parse_block(bool is_global) {
    std::vector<std::unique_ptr<Statement>> stmts;
    while (curr_token().type != TokenType::RBrace) {
        stmts.emplace_back(parse_stmt());
        if (curr_token().type == TokenType::Semicolon) skip_token(";");
    }
    return std::make_unique<BlockStmt>(std::move(stmts));
}

std::unique_ptr<Statement> Parser::parse_if() {
    auto expr = parse_expression();
    skip_token("{");
    auto stmts = parse_block(true);
    skip_token("}");
    std::unique_ptr<BlockStmt> else_stmts = nullptr;
    if (curr_token().type == TokenType::Else) {
        skip_token("else");
        if (curr_token().type == TokenType::If) {
            std::vector<std::unique_ptr<Statement>> if_branch = {};
            if_branch.emplace_back(std::move(stmts));
            else_stmts = std::make_unique<BlockStmt>(std::move(if_branch));
        } else {
            skip_token("{");
            else_stmts = parse_block(true);
            skip_token("}");
        }
    }
    return std::make_unique<IfStmt>(std::move(expr),std::move(stmts),std::move(else_stmts));
}

std::unique_ptr<Statement> Parser::parse_func() {
    auto name = skip_token().text;
    std::vector<std::string> params;

    if (curr_token().type == TokenType::LParen) {
        skip_token("(");
        while (curr_token().type != TokenType::RParen) {
            params.emplace_back(skip_token().text);
            if (curr_token().type == TokenType::Comma) skip_token(",");
        }
        skip_token(")");
    }

    skip_token("{");
    auto stmts = parse_block(true);
    skip_token("}");
    return std::make_unique<FuncDefStmt>(name, std::move(params), std::move(stmts));
}

std::unique_ptr<Statement> Parser::parse_var() {
    auto name = skip_token().text;

    skip_token("=");
    auto expr = parse_expression();
    skip_end_of_ln();
    return std::make_unique<VarDeclStmt>(name, std::move(expr));
}

std::unique_ptr<Statement> Parser::parse_struct() {
    std::vector<std::unique_ptr<IdentifierExpr>> includes{};
    auto name = skip_token().text;
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> init_vec{};
    skip_token("{");
    while (curr_token().type != TokenType::RBrace) {

        if (curr_token().type == TokenType::TripleDot) {
            skip_token("...");
            auto other_struct = std::make_unique<IdentifierExpr>(skip_token().text);
            includes.emplace_back(std::move(other_struct));
            continue;
        }
        auto key = skip_token().text;
        skip_token("=");
        auto val = parse_expression();
        skip_end_of_ln();
        init_vec.emplace_back(std::move(key), std::move(val));
    }
    skip_token("}");
    skip_end_of_ln();
    return std::make_unique<StructDeclStmt>(name, std::move(init_vec),std::move(includes));
}

std::unique_ptr<Statement> Parser::parse_while() {
    auto expr = parse_expression();
    skip_token("{");
    auto stmts = parse_block(true);
    skip_token("}");
    return std::make_unique<WhileStmt>(std::move(expr), std::move(stmts));
}
