#include "parser.hpp"
#include <memory>
#include <iostream>

std::unique_ptr<Expression> Parser::parse_expression(const std::vector<Token>& tokens, size_t& i) {
    // Safety check
    if (i >= tokens.size() || tokens[i].type == TokenType::EndOfFile) {
        std::cerr << "Error: Attempting to parse expression at end of input" << std::endl;
        return nullptr;
    }
    
    return parse_comparison(tokens, i);
}

std::unique_ptr<Expression> Parser::parse_comparison(const std::vector<Token>& tokens, size_t& i) {
    auto left = parse_addition(tokens, i);
    while (i < tokens.size() && (tokens[i].type == TokenType::Equal || tokens[i].type == TokenType::NotEqual ||
           tokens[i].type == TokenType::Less || tokens[i].type == TokenType::LessEqual ||
           tokens[i].type == TokenType::Greater || tokens[i].type == TokenType::GreaterEqual)) {
        std::string op = tokens[i].text;
        ++i;
        auto right = parse_addition(tokens, i);
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expression> Parser::parse_addition(const std::vector<Token>& tokens, size_t& i) {
    auto left = parse_term(tokens, i);
    while (i < tokens.size() && (tokens[i].type == TokenType::Plus || tokens[i].type == TokenType::Minus)) {
        std::string op = tokens[i].text;
        ++i;
        auto right = parse_term(tokens, i);
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expression> Parser::parse_term(const std::vector<Token>& tokens, size_t& i) {
    auto left = parse_power(tokens, i);
    while (i < tokens.size() && (tokens[i].type == TokenType::Star || tokens[i].type == TokenType::Slash || 
           tokens[i].type == TokenType::DoubleSlash || tokens[i].type == TokenType::Percent)) {
        std::string op = tokens[i].text;
        ++i;
        auto right = parse_power(tokens, i);
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expression> Parser::parse_power(const std::vector<Token>& tokens, size_t& i) {
    auto left = parse_unary(tokens, i);
    while (i < tokens.size() && tokens[i].type == TokenType::Caret) {
        std::string op = tokens[i].text;
        ++i;
        auto right = parse_unary(tokens, i);
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expression> Parser::parse_unary(const std::vector<Token>& tokens, size_t& i) {
    if (i >= tokens.size()) return nullptr;
    
    if (tokens[i].type == TokenType::Bang) {
        ++i;
        auto operand = parse_unary(tokens, i);
        return std::make_unique<UnaryExpr>("!", std::move(operand));
    } else if (tokens[i].type == TokenType::Minus) {
        ++i;
        auto operand = parse_unary(tokens, i);
        return std::make_unique<UnaryExpr>("-", std::move(operand));
    }
    
    return parse_primary(tokens, i);
}

std::unique_ptr<Expression> Parser::parse_primary(const std::vector<Token>& tokens, size_t& i) {
    if (i >= tokens.size()) return nullptr;
    
    if (tokens[i].type == TokenType::Number) {
        return std::make_unique<LiteralExpr>(tokens[i++].text);
    } else if (tokens[i].type == TokenType::String) {
        return std::make_unique<LiteralExpr>(tokens[i++].text);
    } else if (tokens[i].type == TokenType::LParen) {
        ++i;
        auto expr = parse_expression(tokens, i);
        if (i < tokens.size() && tokens[i].type == TokenType::RParen) ++i;
        return expr;
    } else if (tokens[i].type == TokenType::Identifier) {
        std::string name = tokens[i++].text;
        // Support function calls
        if (i < tokens.size() && tokens[i].type == TokenType::LParen) {
            ++i; // consume '('
            std::vector<std::unique_ptr<Expression>> args;
            if (i < tokens.size() && tokens[i].type != TokenType::RParen) {
                while (true) {
                    auto arg_expr = parse_expression(tokens, i);
                    if (!arg_expr) {
                        std::cerr << "Error: Invalid expression for argument in function call '" << name << "'" << std::endl;
                        return nullptr;
                    }
                    args.push_back(std::move(arg_expr));

                    if (i < tokens.size() && tokens[i].type == TokenType::Comma) {
                        ++i;
                    } else if (i < tokens.size() && tokens[i].type == TokenType::RParen) {
                        break;
                    } else {
                        std::cerr << "Error: Expected ',' or ')' after argument in function call '" << name << "'" << std::endl;
                        return nullptr;
                    }
                }
            }
            if (i < tokens.size() && tokens[i].type == TokenType::RParen) ++i;
            return std::make_unique<CallExpr>(name, std::move(args));
        }
        return std::make_unique<IdentifierExpr>(name);
    }
    return nullptr;
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
        return std::make_unique<AssignStmt>(name, std::move(expr));
    } else if (tokens[i].type == TokenType::Print) {
        ++i;  // Skip print token
        
        if (i < tokens.size() && tokens[i].type == TokenType::LParen) {
            ++i;  // Skip opening parenthesis
            
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
    } else {
        // Expression statement (including function calls)
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
