#include "parser.hpp"

#include "color_style.hpp"
#include "lexer.hpp"

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens){};

Token Parser::skip_token(const std::string& want_skip) {
    if (curr_tok_idx_ < tokens_.size()) {
        auto& tok = tokens_[curr_tok_idx_];
        if (!want_skip.empty() and tok.text != want_skip) {
            std::cerr << ConClr::RED
            << "There should be '" << want_skip << "' , but you given "
            << tok.text << ConClr::RESET << std::endl;
            throw ReturnException("");
        }
        curr_tok_idx_++;
        return tok;
    }
    return {TokenType::EndOfFile, "", 0, 0};
}

Token Parser::curr_token() const {
    if (curr_tok_idx_ < tokens_.size()) {
        auto& tok = tokens_[curr_tok_idx_];
        return tok;
    }
    return {TokenType::EndOfFile, "", 0, 0};
}

void Parser::skip_end_of_ln() {
    const auto tok = skip_token();
    if (tok.type == TokenType::EndOfFile or tok.type == TokenType::Semicolon) return;
    std::cerr << ConClr::RED << "End of line must a ';'" << ConClr::RESET << std::endl;
}

void Parser::must_token(const std::string& text, const std::string& waring) const {
    if (const auto tok = this->curr_token();
        tok.text != text) {
        std::cerr << ConClr::RED << "The word'" << tok.text << "' cause error that : \n"
                  << waring
                  << ConClr::RESET << std::endl;
    }
}
std::vector<std::unique_ptr<ASTNode>> Parser::parse_program() {
    std::vector<std::unique_ptr<ASTNode>>
        stmts = {};
    while (curr_tok_idx_ < tokens_.size()) {
        stmts.emplace_back(parse_stmt());
    }
    return stmts;
}

std::unique_ptr<Statement> Parser::parse_stmt() {
    auto tok = curr_token();

    if (tok.type == TokenType::If) {
        skip_token();
        return parse_if();
    }
    if (tok.type == TokenType::While) {
        skip_token();
        return parse_while();
    }
    if (tok.type == TokenType::Func) {
        skip_token();
        return parse_func();
    }
    if (tok.type == TokenType::Var) {
        skip_token();
        return parse_var();
    }
    if (tok.type == TokenType::Struct) {
        skip_token();
        return parse_struct();
    }

    if (tok.type == TokenType::Return) {
        skip_token();
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<ReturnStmt>(std::move(expr));
    }
    if (tok.type == TokenType::Break) {
        skip_token();
        skip_end_of_ln();
        return std::make_unique<BreakStmt>();
    }
    if (tok.type == TokenType::Continue) {
        skip_token();
        skip_end_of_ln();
        return std::make_unique<ContinueStmt>();
    }
    if (tok.type == TokenType::Include) {
        skip_token();
        const auto path = curr_token().text;
        skip_end_of_ln();
        return std::make_unique<IncludeStmt>(path);
    }
    if (tok.type == TokenType::Define) {
        skip_token();
        const auto name = curr_token().text;
        skip_token();
        auto value = parse_expression();
        skip_end_of_ln();
        return std::make_unique<DefineStmt>(std::move(name), std::move(value));
    }
    if (tok.type == TokenType::Identifier
        and curr_tok_idx_ + 2 < tokens_.size()
        and tokens_[curr_tok_idx_ + 2].type == TokenType::Assign
    ) {
        const auto name = skip_token().text;
        skip_token();
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<AssignStmt>(std::move(name), std::move(expr));
    }
    if (auto expr = parse_expression();
        expr != nullptr
    ) {
        skip_end_of_ln();
        return std::make_unique<ExprStmt>(std::move(expr));
    }
    while (tok.type != TokenType::Semicolon) {
        skip_token();
        tok = curr_token();
    }
    skip_token();
    return nullptr;
}