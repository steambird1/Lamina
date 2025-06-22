#include "parser.hpp"
#include <memory>
#include <functional>

std::unique_ptr<Expression> Parser::parse_expression(const std::vector<Token>& tokens, size_t& i) {
    // 支持优先级：! > ^ > * / // % > + -
    // 递归下降表达式解析
    // expr = term ((+|-) term)*
    // term = factor ((*|/|//|%) factor)*
    // factor = power
    // power = unary (^ unary)*
    // unary = (!|-)unary | primary
    // primary = number | identifier | (expr)

    // 解析一元运算符
    std::function<std::unique_ptr<Expression>(size_t&)> parse_unary;
    parse_unary = [&](size_t& idx) -> std::unique_ptr<Expression> {
        if (tokens[idx].type == TokenType::Bang) {
            ++idx;
            auto operand = parse_unary(idx);
            return std::make_unique<UnaryExpr>("!", std::move(operand));
        } else if (tokens[idx].type == TokenType::Minus) {
            ++idx;
            auto operand = parse_unary(idx);
            return std::make_unique<UnaryExpr>("-", std::move(operand));
        }
        // primary
        if (tokens[idx].type == TokenType::Number) {
            return std::make_unique<LiteralExpr>(tokens[idx++].text);
        } else if (tokens[idx].type == TokenType::LParen) {
            ++idx;
            auto expr = parse_expression(tokens, idx);
            if (tokens[idx].type == TokenType::RParen) ++idx;
            return expr;
        } else if (tokens[idx].type == TokenType::Identifier) {
            return std::make_unique<IdentifierExpr>(tokens[idx++].text);
        }
        return nullptr;
    };
    // 幂运算
    auto parse_power = [&](size_t& idx) -> std::unique_ptr<Expression> {
        auto left = parse_unary(idx);
        while (tokens[idx].type == TokenType::Caret) {
            std::string op = tokens[idx].text;
            ++idx;
            auto right = parse_unary(idx);
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    };
    // 乘除整除取模
    auto parse_term = [&](size_t& idx) -> std::unique_ptr<Expression> {
        auto left = parse_power(idx);
        while (tokens[idx].type == TokenType::Star || tokens[idx].type == TokenType::Slash || tokens[idx].type == TokenType::DoubleSlash || tokens[idx].type == TokenType::Percent) {
            std::string op = tokens[idx].text;
            ++idx;
            auto right = parse_power(idx);
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    };
    // 加减
    auto left = parse_term(i);
    while (tokens[i].type == TokenType::Plus || tokens[i].type == TokenType::Minus) {
        std::string op = tokens[i].text;
        ++i;
        auto right = parse_term(i);
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<BlockStmt> Parser::parse_block(const std::vector<Token>& tokens, size_t& i) {
    auto block = std::make_unique<BlockStmt>();
    while (tokens[i].type != TokenType::EndOfFile) {
        auto stmt = parse_statement(tokens, i);
        if (stmt) block->statements.push_back(std::move(stmt));
        else break;
    }
    return block;
}

std::unique_ptr<Statement> Parser::parse_statement(const std::vector<Token>& tokens, size_t& i) {
    if (tokens[i].type == TokenType::Var && tokens[i+1].type == TokenType::Identifier && tokens[i+2].type == TokenType::Assign) {
        std::string name = tokens[i+1].text;
        i += 3;
        auto expr = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<VarDeclStmt>(name, std::move(expr));
    } else if (tokens[i].type == TokenType::Identifier && tokens[i+1].type == TokenType::Assign) {
        std::string name = tokens[i].text;
        i += 2;
        auto expr = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<AssignStmt>(name, std::move(expr));
    } else if (tokens[i].type == TokenType::Print && tokens[i+1].type == TokenType::LParen) {
        i += 2;
        auto expr = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::RParen) ++i;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<PrintStmt>(std::move(expr));
    } else if (tokens[i].type == TokenType::Identifier && tokens[i+1].type == TokenType::LParen) {
        // 预留函数调用
        // ...
    } else if (tokens[i].type == TokenType::Return) {
        ++i;
        auto expr = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<ReturnStmt>(std::move(expr));
    } else if (tokens[i].type == TokenType::Semicolon) {
        ++i; // 空语句
        return nullptr;
    } else {
        // 表达式语句
        auto expr = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::Semicolon) ++i;
        if (expr) return std::make_unique<PrintStmt>(std::move(expr)); // 这里可自定义为 ExprStmt
    }
    // 函数定义 func name (params) { block }
    if (tokens[i].type == TokenType::Func && tokens[i+1].type == TokenType::Identifier && tokens[i+2].type == TokenType::LParen) {
        std::string name = tokens[i+1].text;
        i += 3;
        std::vector<std::string> params;
        while (tokens[i].type != TokenType::RParen) {
            if (tokens[i].type == TokenType::Identifier) {
                params.push_back(tokens[i].text);
                ++i;
                if (tokens[i].type == TokenType::Comma) ++i;
            } else {
                break;
            }
        }
        if (tokens[i].type == TokenType::RParen) ++i;
        if (tokens[i].type == TokenType::LBrace) ++i;
        auto body = parse_block(tokens, i);
        if (tokens[i].type == TokenType::RBrace) ++i;
        return std::make_unique<FuncDefStmt>(name, params, std::move(body));
    }
    // if 语句
    if (tokens[i].type == TokenType::If && tokens[i+1].type == TokenType::LParen) {
        i += 2;
        auto cond = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::RParen) ++i;
        if (tokens[i].type == TokenType::LBrace) ++i;
        auto thenBlock = parse_block(tokens, i);
        if (tokens[i].type == TokenType::RBrace) ++i;
        std::unique_ptr<BlockStmt> elseBlock;
        if (tokens[i].type == TokenType::Else) {
            ++i;
            if (tokens[i].type == TokenType::LBrace) ++i;
            elseBlock = parse_block(tokens, i);
            if (tokens[i].type == TokenType::RBrace) ++i;
        }
        return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock), std::move(elseBlock));
    }
    // while 语句
    if (tokens[i].type == TokenType::While && tokens[i+1].type == TokenType::LParen) {
        i += 2;
        auto cond = parse_expression(tokens, i);
        if (tokens[i].type == TokenType::RParen) ++i;
        if (tokens[i].type == TokenType::LBrace) ++i;
        auto body = parse_block(tokens, i);
        if (tokens[i].type == TokenType::RBrace) ++i;
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    }
    // for 语句（简单 C 风格）
    if (tokens[i].type == TokenType::For && tokens[i+1].type == TokenType::LParen) {
        // 这里只做结构预留，具体实现可后续完善
        // ...
    }
    // 函数调用表达式
    if (tokens[i].type == TokenType::Identifier && tokens[i+1].type == TokenType::LParen) {
        std::string callee = tokens[i].text;
        i += 2;
        std::vector<std::unique_ptr<Expression>> args;
        while (tokens[i].type != TokenType::RParen) {
            args.push_back(parse_expression(tokens, i));
            if (tokens[i].type == TokenType::Comma) ++i;
            else break;
        }
        if (tokens[i].type == TokenType::RParen) ++i;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<PrintStmt>(std::make_unique<CallExpr>(callee, std::move(args)));
    }
    // include "module"
    if (tokens[i].type == TokenType::Include && tokens[i+1].type == TokenType::String) {
        std::string mod = tokens[i+1].text;
        i += 2;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<IncludeStmt>(mod);
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parse(const std::vector<Token>& tokens) {
    size_t i = 0;
    return parse_block(tokens, i);
}
