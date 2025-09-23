#include "new_parser.hpp"

#include "color_style.hpp"
#include "lexer.hpp"

NewParser::NewParser(const std::vector<Token>& tokens) : tokens_(tokens){};

Token NewParser::skip_token() {
    if (curr_tok_idx_ < tokens_.size()) {
        auto& tok = tokens_[curr_tok_idx_];
        curr_tok_idx_++;
        return tok;
    }
    return {TokenType::EndOfFile, "", 0, 0};
}

Token NewParser::curr_token() const {
    if (curr_tok_idx_ < tokens_.size()) {
        auto& tok = tokens_[curr_tok_idx_];
        return tok;
    }
    return {TokenType::EndOfFile, "", 0, 0};
}

void NewParser::must_token(const std::string& text, const std::string& waring) const {
    if (const auto tok = this->curr_token();
        tok.text != text) {
        std::cerr << ConClr::RED << "The word'" << tok.text << "' cause error that : \n"
                  << waring
                  << ConClr::RESET << std::endl;
    }
}
std::vector<std::unique_ptr<ASTNode>> NewParser::parse_program() {
    std::vector<std::unique_ptr<ASTNode>>
        stmts = {};
    while (curr_tok_idx_ < tokens_.size()) {
        stmts.emplace_back(parse_stmt());
    }
    return stmts;
}

std::unique_ptr<Statement> NewParser::parse_stmt(){
    const auto tok = curr_token();

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
        const auto& expr = parse_expression();
        return std::make_unique<ReturnStmt>(std::move(expr));
    }
    if (tok.type == TokenType::Break) {
        skip_token();
        return std::make_unique<BreakStmt>();
    }
    if (tok.type == TokenType::Continue) {
        skip_token();
        return std::make_unique<ContinueStmt>();
    }
    if (tok.type == TokenType::Include) {
        skip_token();
        const auto path = curr_token().text;
        return std::make_unique<IncludeStmt>(path);
    }
    if (tok.type == TokenType::Define) {
        skip_token();
        const auto name = curr_token().text;
        skip_token();
        const auto value = parse_expression();
        return std::make_unique<DefineStmt>(name, value);
    }
    if (tok.type == TokenType::Identifier
        and curr_tok_idx_ + 2 < tokens_.size()
        and tokens_[curr_tok_idx_ + 2].type == TokenType::Assign
    ) {
        const auto name = skip_token().text;
        skip_token();
        const auto expr = parse_expression();
        return std::make_unique<AssignStmt>(name, expr);
    }
    if (const auto expr = parse_expression();
        expr != nullptr
    ) {
        return std::make_unique<ExprStmt>(std::move(expr));
    }
    return nullptr;
}