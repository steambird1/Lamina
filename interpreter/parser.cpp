#include "parser.hpp"

#include "interpreter.hpp"
#include "lexer.hpp"
#include "utils/color_style.hpp"

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens){};

std::string Parser::get_module_name() const {
    return module_name_;
}

std::string Parser::get_module_version() const {
    return module_version_;
}

Token Parser::skip_token(const std::string& want_skip) {
    if (curr_tok_idx_ < tokens_.size()) {
        auto& tok = tokens_[curr_tok_idx_];
        if (!want_skip.empty() and tok.text != want_skip) {
            std::cerr << ConClr::RED
            << "There should be '" << want_skip << "' , but you given '"
            << tok.text << "'" << ConClr::RESET << std::endl;
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
    const Token tok = curr_token();
    if (tok.type == TokenType::Semicolon) {
        skip_token(";");
        return;
    }
    if (tok.type == TokenType::EndOfFile) {
        return;
    }
    std::cerr << ConClr::RED << "End of line must be ';', got '" << tok.text << "'" << ConClr::RESET << std::endl;
    throw ReturnException("");
}

void Parser::must_token(const std::string& text, const std::string& waring) const {
    if (const auto tok = this->curr_token();
        tok.text != text) {
        std::cerr << ConClr::RED << "The word'" << tok.text << "' cause error that : \n"
                  << waring
                  << ConClr::RESET << std::endl;
    }
}
std::vector<std::unique_ptr<Statement>> Parser::parse_program() {
    std::vector<std::unique_ptr<Statement>>
        stmts = {};
    while (curr_token().type != TokenType::EndOfFile) {
        if (auto s = parse_stmt();
            s != nullptr
        ) {
            stmts.push_back(std::move(s));
        }
    }
    return stmts;
}

std::unique_ptr<Statement> Parser::parse_stmt() {
    auto tok = curr_token();

    if (tok.type == TokenType::If) {
        skip_token("if");
        return parse_if();
    }
    if (tok.type == TokenType::While) {
        skip_token("while");
        return parse_while();
    }
    if (tok.type == TokenType::Func) {
        skip_token("func");
        return parse_func();
    }
    if (tok.type == TokenType::Var) {
        skip_token("var");
        return parse_var();
    }
    if (tok.type == TokenType::Struct) {
        skip_token("struct");
        return parse_struct();
    }

    if (tok.type == TokenType::Return) {
        skip_token("return");
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<ReturnStmt>(std::move(expr));
    }
    if (tok.type == TokenType::Break) {
        skip_token("break");
        skip_end_of_ln();
        return std::make_unique<BreakStmt>();
    }
    if (tok.type == TokenType::Continue) {
        skip_token("continue");
        skip_end_of_ln();
        return std::make_unique<ContinueStmt>();
    }
    if (tok.type == TokenType::Include) {
        skip_token("include");
        const auto path = skip_token().text;
        skip_end_of_ln();
        return std::make_unique<IncludeStmt>(path);
    }
    if (tok.type == TokenType::Loop) {
    auto expr = std::make_unique<LiteralExpr>("true", Value::Type::Bool);
    skip_token("{");
    auto stmts = parse_block(true);
    skip_token("}");
    return std::make_unique<WhileStmt>(std::move(expr), std::move(stmts));
    }
    if (tok.type == TokenType::Define) {
        skip_token("define");
        const auto name = skip_token().text;
        skip_token("=");
        auto value = parse_a_token();
        skip_end_of_ln();
        if (const auto string_val = dynamic_cast<LiteralExpr*>(value.get());
            name == "module_name" and string_val->type == Value::Type::String
        ) {
            module_name_ = string_val->value;
            return nullptr;
        }
        if (const auto string_val = dynamic_cast<LiteralExpr*>(value.get());
            name == "module_version" and string_val->type == Value::Type::String
        ) {
            module_version_ = string_val->value;
            return nullptr;
        }

        return std::make_unique<DefineStmt>(name, std::move(value));
    }
    if (tok.type == TokenType::Bigint) {
        skip_token("bigint");
        const auto name = skip_token().text;
        skip_token("=");
        auto value = parse_expression();
        skip_end_of_ln();
        return std::make_unique<BigIntDeclStmt>(name, std::move(value));
    }
    if (tok.type == TokenType::Identifier
        and curr_tok_idx_ + 1 < tokens_.size()
        and tokens_[curr_tok_idx_ + 1].type == TokenType::Assign
    ) {
        const auto name = skip_token().text;
        skip_token("=");
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<AssignStmt>(name, std::move(expr));
    }
    if (auto expr = parse_expression();
        expr != nullptr
    ) {
        skip_end_of_ln();
        return std::make_unique<ExprStmt>(std::move(expr));
    }
    while (tok.type != TokenType::Semicolon) {
        skip_token(";");
        tok = curr_token();
    }
    skip_token();
    return nullptr;
}