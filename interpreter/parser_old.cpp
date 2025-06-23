#include "parser.hpp"
#include <memory>
#include <functional>
#include <iostream>

std::unique_ptr<Expression> Parser::parse_expression(const std::vector<Token>& tokens, size_t& i) {
    // Safety check
    if (i >= tokens.size() || tokens[i].type == TokenType::EndOfFile) {
        std::cerr << "Error: Attempting to parse expression at end of input" << std::endl;
        return nullptr;
    }
    
    // Priority support: comparison > addition/subtraction > multiplication/division > power > unary
    // Recursive descent expression parsing
    // expr = comparison
    // comparison = addition ( (==|!=|<|<=|>|>=) addition )*
    // addition = term ( (+|-) term )*
    // term = factor ( (*|/|//|%) factor )*
    // factor = power
    // power = unary ( ^ unary )*
    // unary = (!|-)unary | primary
    // primary = number | string | identifier | (expr)

    // Forward declarations for the parsing functions
    std::function<std::unique_ptr<Expression>(size_t&)> parse_comparison;
    std::function<std::unique_ptr<Expression>(size_t&)> parse_addition;
    std::function<std::unique_ptr<Expression>(size_t&)> parse_term;
    std::function<std::unique_ptr<Expression>(size_t&)> parse_power;
    std::function<std::unique_ptr<Expression>(size_t&)> parse_unary;

    // Parse unary operator
    parse_unary = [&](size_t& idx) -> std::unique_ptr<Expression> {
        if (idx >= tokens.size()) return nullptr;
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
        } else if (tokens[idx].type == TokenType::String) {
            return std::make_unique<LiteralExpr>(tokens[idx++].text);        } else if (tokens[idx].type == TokenType::LParen) {
            ++idx;
            auto expr = parse_expression(tokens, idx); // Recursive call to the main parser
            if (idx < tokens.size() && tokens[idx].type == TokenType::RParen) ++idx;
            return expr;
        } else if (tokens[idx].type == TokenType::Identifier) {
            std::string name = tokens[idx++].text;
            // Support function calls
            if (idx < tokens.size() && tokens[idx].type == TokenType::LParen) {
                ++idx; // consume '('
                std::vector<std::unique_ptr<Expression>> args;                if (idx < tokens.size() && tokens[idx].type != TokenType::RParen) {
                    while (true) {
                        auto arg_expr = parse_expression(tokens, idx); // Recursive call
                        if (!arg_expr) {
                            std::cerr << "Error: Invalid expression for argument in function call '" << name << "'" << std::endl;
                            return nullptr;
                        }
                        args.push_back(std::move(arg_expr));

                        if (idx < tokens.size() && tokens[idx].type == TokenType::Comma) {
                            ++idx;
                        } else if (idx < tokens.size() && tokens[idx].type == TokenType::RParen) {
                            break;
                        } else {
                            std::cerr << "Error: Expected ',' or ')' after argument in function call '" << name << "'" << std::endl;
                            return nullptr;
                        }
                    }
                }
                if (idx < tokens.size() && tokens[idx].type == TokenType::RParen) ++idx;
                return std::make_unique<CallExpr>(name, std::move(args));
            }
            return std::make_unique<IdentifierExpr>(name);
        }
        return nullptr;
    };
    
    // Power operation
    parse_power = [&](size_t& idx) -> std::unique_ptr<Expression> {
        auto left = parse_unary(idx);
        while (idx < tokens.size() && tokens[idx].type == TokenType::Caret) {
            std::string op = tokens[idx].text;
            ++idx;
            auto right = parse_unary(idx);
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    };
    
    // Multiplication, division, integer division, modulo
    parse_term = [&](size_t& idx) -> std::unique_ptr<Expression> {
        auto left = parse_power(idx);
        while (idx < tokens.size() && (tokens[idx].type == TokenType::Star || tokens[idx].type == TokenType::Slash || tokens[idx].type == TokenType::DoubleSlash || tokens[idx].type == TokenType::Percent)) {
            std::string op = tokens[idx].text;
            ++idx;
            auto right = parse_power(idx);
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    };
    
    // Addition and subtraction
    parse_addition = [&](size_t& idx) -> std::unique_ptr<Expression> {
        auto left = parse_term(idx);
        while (idx < tokens.size() && (tokens[idx].type == TokenType::Plus || tokens[idx].type == TokenType::Minus)) {
            std::string op = tokens[idx].text;
            ++idx;
            auto right = parse_term(idx);
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    };

    // Comparison operators
    parse_comparison = [&](size_t& idx) -> std::unique_ptr<Expression> {
        auto left = parse_addition(idx);
        while (idx < tokens.size() && (tokens[idx].type == TokenType::Equal || tokens[idx].type == TokenType::NotEqual ||
               tokens[idx].type == TokenType::Less || tokens[idx].type == TokenType::LessEqual ||
               tokens[idx].type == TokenType::Greater || tokens[idx].type == TokenType::GreaterEqual)) {
            std::string op = tokens[idx].text;
            ++idx;
            auto right = parse_addition(idx);
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    };    // The main call starts the chain from the lowest precedence
    auto result = parse_comparison(i);
    return result;
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
        if (!expr) {
            std::cerr << "Error: Missing expression in variable declaration for '" << name << "'" << std::endl;
            return nullptr;
        }
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<VarDeclStmt>(name, std::move(expr));
    } else if (tokens[i].type == TokenType::Identifier && tokens[i+1].type == TokenType::Assign) {
        std::string name = tokens[i].text;
        i += 2;
        auto expr = parse_expression(tokens, i);
        if (!expr) {
            std::cerr << "Error: Missing expression in assignment to '" << name << "'" << std::endl;
            return nullptr;
        }
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<AssignStmt>(name, std::move(expr));    } else if (tokens[i].type == TokenType::Print) {
        ++i;  // Skip print token
        
        if (i < tokens.size() && tokens[i].type == TokenType::LParen) {
            ++i;  // Skip opening parenthesis
            
            // Check for empty print() or print() with an expression
            if (i < tokens.size() && tokens[i].type != TokenType::RParen && tokens[i].type != TokenType::EndOfFile) {
                auto expr = parse_expression(tokens, i);
                if (!expr) {
                    std::cerr << "Error: Invalid expression in print statement" << std::endl;
                    return nullptr;
                }
                
                if (i < tokens.size() && tokens[i].type == TokenType::RParen) {
                    ++i;  // Skip closing parenthesis
                }
                
                if (i < tokens.size() && tokens[i].type == TokenType::Semicolon) {
                    ++i;  // Skip semicolon
                }
                
                return std::make_unique<PrintStmt>(std::move(expr));
            } else {
                // Empty print()
                if (i < tokens.size() && tokens[i].type == TokenType::RParen) {
                    ++i;  // Skip closing parenthesis
                }
                
                if (i < tokens.size() && tokens[i].type == TokenType::Semicolon) {
                    ++i;  // Skip semicolon
                }
                
                // Create a empty string literal for empty print
                auto emptyExpr = std::make_unique<LiteralExpr>("");
                return std::make_unique<PrintStmt>(std::move(emptyExpr));
            }
        } else {
            std::cerr << "Error: Expected '(' after 'print'" << std::endl;
            return nullptr;
        }
    } else if (tokens[i].type == TokenType::Return) {
        ++i;
        auto expr = parse_expression(tokens, i);
        if (!expr) {
            std::cerr << "Error: Missing expression in return statement" << std::endl;
            return nullptr;
        }
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<ReturnStmt>(std::move(expr));
    } else if (tokens[i].type == TokenType::Break) {
        ++i;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<BreakStmt>();
    } else if (tokens[i].type == TokenType::Continue) {
        ++i;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<ContinueStmt>();
    } else if (tokens[i].type == TokenType::Semicolon) {
        ++i; // Empty statement
        return nullptr;
    } else {        // Expression statement (including function calls)
        if (i < tokens.size() && tokens[i].type != TokenType::EndOfFile) {
            auto expr = parse_expression(tokens, i);
            if (i < tokens.size() && tokens[i].type == TokenType::Semicolon) ++i;
            if (expr) return std::make_unique<ExprStmt>(std::move(expr));
        }
    }
    // Function definition: func name(params) { block }
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
    // if statement
    if (tokens[i].type == TokenType::If && tokens[i+1].type == TokenType::LParen) {
        i += 2;
        auto cond = parse_expression(tokens, i);
        if (!cond) {
            std::cerr << "Error: Missing condition in if statement" << std::endl;
            return nullptr;
        }
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
    // while statement
    if (tokens[i].type == TokenType::While && tokens[i+1].type == TokenType::LParen) {
        i += 2;
        auto cond = parse_expression(tokens, i);
        if (!cond) {
            std::cerr << "Error: Missing condition in while statement" << std::endl;
            return nullptr;
        }
        if (tokens[i].type == TokenType::RParen) ++i;
        if (tokens[i].type == TokenType::LBrace) ++i;
        auto body = parse_block(tokens, i);
        if (tokens[i].type == TokenType::RBrace) ++i;
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    }
    // include "module"
    if (tokens[i].type == TokenType::Include && tokens[i+1].type == TokenType::String) {
        std::string mod = tokens[i+1].text;
        i += 2;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<IncludeStmt>(mod);
    }
    // break statement
    if (tokens[i].type == TokenType::Break) {
        ++i;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<BreakStmt>();
    }
    // continue statement
    if (tokens[i].type == TokenType::Continue) {
        ++i;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<ContinueStmt>();
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parse(const std::vector<Token>& tokens) {
    size_t i = 0;
    return parse_block(tokens, i);
}