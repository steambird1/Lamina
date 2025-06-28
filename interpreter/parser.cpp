#include "parser.hpp"
#include <memory>
#include <iostream>
#include <string>
#include <stdexcept>

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
           tokens[i].type == TokenType::Percent)) {
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
    
    // Handle prefix unary operators
    if (tokens[i].type == TokenType::Minus) {
        ++i;
        auto operand = parse_unary(tokens, i);
        return std::make_unique<UnaryExpr>("-", std::move(operand));
    }
    
    // Parse primary expression first
    auto expr = parse_primary(tokens, i);
    
    // Handle postfix unary operators (like factorial)
    while (i < tokens.size() && tokens[i].type == TokenType::Bang) {
        ++i;  // consume '!'
        expr = std::make_unique<UnaryExpr>("!", std::move(expr));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parse_primary(const std::vector<Token>& tokens, size_t& i) {
    if (i >= tokens.size()) return nullptr;
      if (tokens[i].type == TokenType::Number) {
        return std::make_unique<LiteralExpr>(tokens[i++].text);
    } else if (tokens[i].type == TokenType::String) {
        return std::make_unique<LiteralExpr>(tokens[i++].text);
    } else if (tokens[i].type == TokenType::True) {
        ++i;
        return std::make_unique<LiteralExpr>("true");
    } else if (tokens[i].type == TokenType::False) {
        ++i;
        return std::make_unique<LiteralExpr>("false");
    } else if (tokens[i].type == TokenType::Null) {
        ++i;
        return std::make_unique<LiteralExpr>("null");
    } else if (tokens[i].type == TokenType::LBracket) {
        // Parse array literal [expr1, expr2, ...]
        ++i; // Skip '['
        std::vector<std::unique_ptr<Expression>> elements;
        
        while (i < tokens.size() && tokens[i].type != TokenType::RBracket) {
            auto elem = parse_expression(tokens, i);
            elements.push_back(std::move(elem));
            
            if (tokens[i].type == TokenType::Comma) {
                ++i; // Skip ','
            } else if (tokens[i].type != TokenType::RBracket) {
                std::cerr << "Error: Expected ',' or ']' in array literal" << std::endl;
                return nullptr;
            }
        }
        
        if (i >= tokens.size() || tokens[i].type != TokenType::RBracket) {
            std::cerr << "Error: Unterminated array literal, expected ']'" << std::endl;
            return nullptr;
        }
        
        ++i; // Skip ']'
        return std::make_unique<ArrayExpr>(std::move(elements));
    } else if (tokens[i].type == TokenType::Identifier) {
        std::string name = tokens[i].text;
        ++i;
        
        // Function call
        if (i < tokens.size() && tokens[i].type == TokenType::LParen) {
            ++i; // Skip '('
            std::vector<std::unique_ptr<Expression>> args;
            
            while (i < tokens.size() && tokens[i].type != TokenType::RParen) {
                auto arg = parse_expression(tokens, i);
                args.push_back(std::move(arg));
                
                if (tokens[i].type == TokenType::Comma) {
                    ++i; // Skip ','
                } else if (tokens[i].type != TokenType::RParen) {
                    std::cerr << "Error: Expected ',' or ')' in function call" << std::endl;
                    return nullptr;
                }
            }
            
            if (i >= tokens.size() || tokens[i].type != TokenType::RParen) {
                std::cerr << "Error: Unterminated function call, expected ')'" << std::endl;
                return nullptr;
            }
            
            ++i; // Skip ')'
            return std::make_unique<CallExpr>(name, std::move(args));
        }
        
        // Variable reference
        return std::make_unique<VarExpr>(name);
    } else if (tokens[i].type == TokenType::LParen) {
        ++i; // Skip '('
        auto expr = parse_expression(tokens, i);
        
        if (i >= tokens.size() || tokens[i].type != TokenType::RParen) {
            std::cerr << "Error: Unterminated parenthesized expression, expected ')'" << std::endl;
            return nullptr;
        }
        
        ++i; // Skip ')'
        return expr;
    }
    
    std::cerr << "Error: Unexpected token in expression: " << tokens[i].text << std::endl;
    return nullptr;
}

// 辅助函数：打印token上下文，用于错误报告
static void print_context(const std::vector<Token>& tokens, size_t pos, int context_size = 5) {
    std::cerr << "上下文: ";
    size_t context_start = pos > context_size ? pos - context_size : 0;
    size_t context_end = std::min(pos + context_size, tokens.size() - 1);
    
    for (size_t j = context_start; j <= context_end; j++) {
        if (j == pos) {
            std::cerr << "[" << tokens[j].text << "] "; // 高亮显示当前token
        } else {
            std::cerr << tokens[j].text << " ";
        }
    }
    std::cerr << std::endl;
    
    // 打印行号和列号指示器
    std::cerr << "位置: ";
    for (size_t j = context_start; j <= context_end; j++) {
        if (j == pos) {
            std::cerr << "行" << tokens[j].line << "列" << tokens[j].column << " ";
        } else {
            std::cerr << std::string(tokens[j].text.length() + 1, ' ');
        }
    }
    std::cerr << std::endl;
}

std::unique_ptr<BlockStmt> Parser::parse_block(const std::vector<Token>& tokens, size_t& i, bool is_global) {
    auto block = std::make_unique<BlockStmt>();
    
    // 记录当前块的起始位置，便于调试
    int start_line = (i < tokens.size()) ? tokens[i].line : -1;
    int start_col = (i < tokens.size()) ? tokens[i].column : -1;
    
    // 使用非静态变量记录嵌套深度，避免多次解析干扰
    static thread_local int current_block_depth = 0;
    current_block_depth++;
    
    std::cerr << "Debug - 开始解析块: " 
              << (is_global ? "全局块" : "局部块") 
              << ", 深度=" << current_block_depth 
              << ", 位置=" << start_line << ":" << start_col 
              << ", 当前token=" << (i < tokens.size() ? "'" + tokens[i].text + "'" : "EOF")
              << std::endl;
    
    // 记录起始标记的位置用于错误报告
    size_t block_start_index = i;
    
    while (i < tokens.size()) {
        // 首先检查是否到达块结束
        if (!is_global && tokens[i].type == TokenType::RBrace) {
            // 局部块遇到 } 结束
            std::cerr << "Debug - 块正常结束: 深度=" << current_block_depth 
                      << ", 位置=" << tokens[i].line << ":" << tokens[i].column 
                      << ", 语句数=" << block->statements.size()
                      << std::endl;
            current_block_depth--;
            i++; // 消费右花括号
            return block;
        }
        
        // 检查是否到达文件结尾
        if (tokens[i].type == TokenType::EndOfFile) {
            if (!is_global) {
                // 局部块未正常结束就遇到EOF，提供更详细的错误信息
                std::cerr << "Error: 缺少右花括号 '}' - 块开始于行" << start_line 
                          << "列" << start_col
                          << "，在文件结束前未闭合" << std::endl;
                
                // 输出开始块的前后上下文，帮助定位问题
                std::cerr << "块开始处上下文:" << std::endl;
                size_t context_start = block_start_index > 5 ? block_start_index - 5 : 0;
                size_t context_end = std::min(block_start_index + 5, tokens.size() - 1);
                for (size_t j = context_start; j <= context_end; j++) {
                    if (j == block_start_index) {
                        std::cerr << "[" << tokens[j].text << "] ";
                    } else {
                        std::cerr << tokens[j].text << " ";
                    }
                }
                std::cerr << std::endl;
                
                // 显示块内容摘要
                std::cerr << "块包含 " << block->statements.size() << " 条语句" << std::endl;
                
                current_block_depth--;
                // 抛出异常而非直接退出，让调用者有机会恢复
                throw std::runtime_error("解析错误: 块未闭合，缺少右花括号");
            } else {
                // 全局块遇到EOF，正常结束
                std::cerr << "Debug - 全局块在EOF处结束: 深度=" << current_block_depth 
                          << ", 语句数=" << block->statements.size() << std::endl;
                current_block_depth--;
                return block;
            }
        }
        
        // 解析块中的语句
        auto stmt = parse_statement(tokens, i);
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        } else if (i < tokens.size() && tokens[i].type != TokenType::EndOfFile) {
            // 只有在未到文件末尾且解析失败时才报错
            std::cerr << "Error: 在token " << i 
                      << " (行" << tokens[i].line 
                      << "列" << tokens[i].column 
                      << ")处有无效或意外的语句: " 
                      << tokens[i].text << std::endl;
                      
            // 尝试恢复：跳过当前token继续解析
            i++;
            continue;
        } else {
            // 到达文件末尾，此时应该是全局块
            if (!is_global) {
                std::cerr << "Error: 意外的文件结束，缺少块闭合，开始于行" << start_line << std::endl;
                current_block_depth--;
                throw std::runtime_error("解析错误: 块未闭合，缺少右花括号");
            }
            break;
        }
        
        // 检查进度，避免死循环
        if (i >= tokens.size()) {
            if (!is_global) {
                std::cerr << "Error: 意外的文件结束，缺少块闭合，开始于行" << start_line << std::endl;
                current_block_depth--;
                throw std::runtime_error("解析错误: 块未闭合，缺少右花括号");
            }
            break;
        }
    }
    
    std::cerr << "Debug - 块解析完成: 深度=" << current_block_depth 
              << ", 语句数=" << block->statements.size() << std::endl;
    
    // 确保在返回前减少深度计数
    current_block_depth--;
    return block;
}

// 2参数重载，兼容老代码
std::unique_ptr<BlockStmt> Parser::parse_block(const std::vector<Token>& tokens, size_t& i) {
    return parse_block(tokens, i, false);
}

std::unique_ptr<Statement> Parser::parse_while(const std::vector<Token>& tokens, size_t& i) {
    if (!(i < tokens.size() && tokens[i].type == TokenType::While)) {
        return nullptr;
    }
    
    int while_line = tokens[i].line;
    int while_col = tokens[i].column;
    size_t while_start_index = i; // 记录while语句开始位置
    
    std::cerr << "Debug - 开始解析while循环, 行 " << while_line << " 列 " << while_col << std::endl;
    
    ++i; // 跳过while
    
    // 检查是否有左括号
    if (i >= tokens.size() || tokens[i].type != TokenType::LParen) {
        std::cerr << "Error: while语句后缺少左括号 '('" << std::endl;
        print_context(tokens, i > 0 ? i - 1 : 0);
        
        // 尝试恢复：寻找左括号或左大括号
        while (i < tokens.size() && 
              tokens[i].type != TokenType::LParen && 
              tokens[i].type != TokenType::LBrace) {
            ++i;
        }
        
        if (i >= tokens.size() || tokens[i].type != TokenType::LParen) {
            return nullptr; // 找不到可恢复的点
        }
    }
    
    ++i; // 跳过左括号
    
    // 解析条件
    if (i >= tokens.size()) {
        std::cerr << "Error: while语句意外结束，缺少条件表达式" << std::endl;
        print_context(tokens, while_start_index);
        return nullptr;
    }
    
    auto cond = parse_expression(tokens, i);
    if (!cond) {
        std::cerr << "Error: while语句缺少有效的条件表达式" << std::endl;
        print_context(tokens, i);
        
        // 尝试恢复：创建一个恒为真的条件
        cond = std::make_unique<LiteralExpr>("true");
        
        // 尝试寻找右括号
        while (i < tokens.size() && tokens[i].type != TokenType::RParen) {
            ++i;
        }
        
        if (i >= tokens.size()) {
            return nullptr; // 找不到右括号，无法继续
        }
    }
    
    // 检查右括号
    if (i >= tokens.size()) {
        std::cerr << "Error: while语句条件后意外结束，缺少右括号" << std::endl;
        print_context(tokens, while_start_index);
        return nullptr;
    }
    
    if (tokens[i].type == TokenType::RParen) {
        ++i; // 消费右括号
    } else {
        std::cerr << "Error: while语句条件缺少右括号 ')'，在行" << tokens[i-1].line << "之后" << std::endl;
        print_context(tokens, i);
        
        // 尝试恢复：寻找左大括号
        while (i < tokens.size() && tokens[i].type != TokenType::LBrace) {
            ++i;
        }
        
        if (i >= tokens.size()) {
            return nullptr; // 找不到左大括号，无法继续
        }
    }
    
    // 检查左大括号
    if (i >= tokens.size()) {
        std::cerr << "Error: while语句右括号后意外结束，缺少循环体" << std::endl;
        print_context(tokens, while_start_index);
        return nullptr;
    }
    
    if (tokens[i].type == TokenType::LBrace) {
        ++i; // 消费左大括号
    } else {
        std::cerr << "Error: while语句缺少左大括号 '{'" << std::endl;
        print_context(tokens, i);
        
        // 尝试恢复：创建一个空块并返回
        auto emptyBody = std::make_unique<BlockStmt>();
        return std::make_unique<WhileStmt>(std::move(cond), std::move(emptyBody));
    }
    
    // 解析循环体
    std::cerr << "Debug - 解析while循环体" << std::endl;
    std::unique_ptr<BlockStmt> body;
    
    try {
        body = parse_block(tokens, i, false);
        std::cerr << "Debug - while循环体解析完成，包含" << body->statements.size() << "条语句" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: 解析while循环体时出错: " << e.what() << std::endl;
        // 创建空块作为恢复措施
        body = std::make_unique<BlockStmt>();
        
        // 尝试定位到下一个语句开始
        while (i < tokens.size() && 
               tokens[i].type != TokenType::Semicolon &&
               tokens[i].type != TokenType::RBrace) {
            ++i;
        }
        
        if (i < tokens.size() && tokens[i].type == TokenType::RBrace) {
            ++i; // 消费右大括号
        }
    }
    
    std::cerr << "Debug - while循环解析完成，定义于行 " << while_line 
              << ", 结束于行 " << (i < tokens.size() ? tokens[i-1].line : -1) << std::endl;
    
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Statement> Parser::parse_statement(const std::vector<Token>& tokens, size_t& i) {
    // 处理函数定义和if语句之前，检查并解析while语句
    if (tokens[i].type == TokenType::While) {
        return parse_while(tokens, i);
    }

    if (tokens[i].type == TokenType::Define && i+2 < tokens.size() && 
        tokens[i+1].type == TokenType::Identifier) {
        std::string name = tokens[i+1].text;
        i += 2;
        auto expr = parse_expression(tokens, i);
        if (!expr) {
            std::cerr << "Error: Missing expression in define statement for '" << name << "'" << std::endl;
            return nullptr;
        }
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<DefineStmt>(name, std::move(expr));
    } else if (tokens[i].type == TokenType::Bigint && i+1 < tokens.size() && 
               tokens[i+1].type == TokenType::Identifier) {
        std::string name = tokens[i+1].text;
        i += 2;
        std::unique_ptr<Expression> init_value = nullptr;
        if (i < tokens.size() && tokens[i].type == TokenType::Assign) {
            ++i;
            init_value = parse_expression(tokens, i);
            if (!init_value) {
                std::cerr << "Error: Missing expression in bigint declaration for '" << name << "'" << std::endl;
                return nullptr;
            }
        }
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<BigIntDeclStmt>(name, std::move(init_value));
    } else if (tokens[i].type == TokenType::Var && tokens[i+1].type == TokenType::Identifier && tokens[i+2].type == TokenType::Assign) {
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
            
            std::vector<std::unique_ptr<Expression>> exprs;
            
            if (i < tokens.size() && tokens[i].type != TokenType::RParen && tokens[i].type != TokenType::EndOfFile) {
                // Parse multiple comma-separated expressions
                while (true) {
                    auto expr = parse_expression(tokens, i);
                    if (!expr) {
                        std::cerr << "Error: Invalid expression in print statement" << std::endl;
                        return nullptr;
                    }
                    exprs.push_back(std::move(expr));
                    
                    if (i < tokens.size() && tokens[i].type == TokenType::Comma) {
                        ++i; // consume comma
                    } else if (i < tokens.size() && tokens[i].type == TokenType::RParen) {
                        break;
                    } else {
                        std::cerr << "Error: Expected ',' or ')' in print statement" << std::endl;
                        return nullptr;
                    }
                }
            }
            
            if (i < tokens.size() && tokens[i].type == TokenType::RParen) {
                ++i;  // Skip closing parenthesis
            }
            
            if (i < tokens.size() && tokens[i].type == TokenType::Semicolon) {
                ++i;  // Skip semicolon
            }
            
            if (exprs.empty()) {
                // Empty print()
                auto emptyExpr = std::make_unique<LiteralExpr>("");
                return std::make_unique<PrintStmt>(std::move(emptyExpr));
            } else {
                return std::make_unique<PrintStmt>(std::move(exprs));
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
    if (tokens[i].type == TokenType::Func && i+2 < tokens.size() && 
        tokens[i+1].type == TokenType::Identifier && tokens[i+2].type == TokenType::LParen) {
        std::string name = tokens[i+1].text;
        int func_line = tokens[i].line;
        int func_col = tokens[i].column;
        std::cerr << "Debug - 开始解析函数: " << name << " 在行 " << func_line << "列" << func_col << std::endl;
        
        size_t func_start_index = i; // 记录函数开始位置用于错误报告
        i += 3; // 跳过 func name(
        
        // 收集参数
        std::vector<std::string> params;
        while (i < tokens.size() && tokens[i].type != TokenType::RParen) {
            if (tokens[i].type == TokenType::Identifier) {
                params.push_back(tokens[i].text);
                ++i;
                if (i < tokens.size() && tokens[i].type == TokenType::Comma) {
                    ++i;
                } else if (i < tokens.size() && tokens[i].type != TokenType::RParen) {
                    std::cerr << "Error: 函数 '" << name << "' 的参数列表中缺少逗号，找到: " 
                              << tokens[i].text << " 在行" << tokens[i].line << std::endl;
                    // 尝试恢复：寻找右括号或逗号
                    while (i < tokens.size() && 
                          tokens[i].type != TokenType::RParen && 
                          tokens[i].type != TokenType::Comma) {
                        ++i;
                    }
                    if (i < tokens.size() && tokens[i].type == TokenType::Comma) {
                        ++i; // 消费逗号并继续
                    }
                }
            } else {
                std::cerr << "Error: 函数 '" << name << "' 的参数列表中有无效的标记: " 
                          << tokens[i].text << " 在行" << tokens[i].line << std::endl;
                
                // 尝试恢复：寻找下一个逗号或右括号
                while (i < tokens.size() && 
                      tokens[i].type != TokenType::RParen && 
                      tokens[i].type != TokenType::Comma) {
                    ++i;
                }
                if (i < tokens.size() && tokens[i].type == TokenType::Comma) {
                    ++i; // 消费逗号并继续
                }
            }
        }
        
        if (i >= tokens.size()) {
            std::cerr << "Error: 函数 '" << name << "' 在解析参数时意外结束，开始于行" << func_line << std::endl;
            // 输出函数开始处的上下文
            print_context(tokens, func_start_index);
            return nullptr;
        }
        
        if (tokens[i].type == TokenType::RParen) {
            ++i; // 消费右括号
        } else {
            std::cerr << "Error: 函数 '" << name << "' 的参数列表缺少右括号，在行" << tokens[i-1].line << "列" << tokens[i-1].column << "之后" << std::endl;
            print_context(tokens, i);
            // 尝试恢复：寻找左大括号
            while (i < tokens.size() && tokens[i].type != TokenType::LBrace) {
                ++i;
            }
            if (i >= tokens.size()) {
                return nullptr; // 没找到左大括号，放弃
            }
        }
        
        if (i >= tokens.size()) {
            std::cerr << "Error: 函数 '" << name << "' 在右括号后意外结束" << std::endl;
            return nullptr;
        }
        
        if (tokens[i].type == TokenType::LBrace) {
            ++i; // 消费左大括号
            std::cerr << "Debug - 开始解析函数 '" << name << "' 的函数体，行" << tokens[i-1].line << "列" << tokens[i-1].column << std::endl;
        } else {
            std::cerr << "Error: 函数 '" << name << "' 定义缺少左大括号 '{'，在行" << tokens[i-1].line << "之后" << std::endl;
            print_context(tokens, i);
            return nullptr;
        }
        
        // 解析函数体
        try {
            auto body = parse_block(tokens, i, false);
            
            std::cerr << "Debug - 函数 '" << name << "' 函数体解析完成" << std::endl;
            
            // 注意：现在parse_block会处理右花括号，所以不需要再次检查
            return std::make_unique<FuncDefStmt>(name, params, std::move(body));
        }
        catch (const std::exception& e) {
            std::cerr << "Error: 解析函数 '" << name << "' 的函数体时出错: " << e.what() << std::endl;
            // 尝试恢复：寻找右大括号或函数定义结束标记
            while (i < tokens.size() && tokens[i].type != TokenType::RBrace) {
                ++i;
            }
            if (i < tokens.size()) {
                ++i; // 消费右大括号
            }
            // 返回不完整的函数定义
            auto empty_body = std::make_unique<BlockStmt>();
            return std::make_unique<FuncDefStmt>(name, params, std::move(empty_body));
        }
    }

    // if statement
    if (tokens[i].type == TokenType::If && i+1 < tokens.size()) {
        int if_line = tokens[i].line;
        int if_col = tokens[i].column;
        size_t if_start_index = i; // 记录if语句开始位置
        
        std::cerr << "Debug - 开始解析if语句, 行 " << if_line << " 列 " << if_col << std::endl;
        
        ++i; // 跳过if
        
        // 检查是否有左括号
        if (i >= tokens.size() || tokens[i].type != TokenType::LParen) {
            std::cerr << "Error: if语句后缺少左括号 '('" << std::endl;
            print_context(tokens, i > 0 ? i - 1 : 0);
            
            // 尝试恢复：寻找左括号或左大括号
            while (i < tokens.size() && 
                  tokens[i].type != TokenType::LParen && 
                  tokens[i].type != TokenType::LBrace) {
                ++i;
            }
            
            if (i >= tokens.size() || tokens[i].type != TokenType::LParen) {
                return nullptr; // 找不到可恢复的点
            }
        }
        
        ++i; // 跳过左括号
        
        // 解析条件
        if (i >= tokens.size()) {
            std::cerr << "Error: if语句意外结束，缺少条件表达式" << std::endl;
            print_context(tokens, if_start_index);
            return nullptr;
        }
        
        auto cond = parse_expression(tokens, i);
        if (!cond) {
            std::cerr << "Error: if语句缺少有效的条件表达式" << std::endl;
            print_context(tokens, i);
            
            // 尝试恢复：寻找右括号
            while (i < tokens.size() && tokens[i].type != TokenType::RParen) {
                ++i;
            }
            
            if (i >= tokens.size()) {
                return nullptr; // 找不到右括号，无法继续
            }
        }
        
        // 检查右括号
        if (i >= tokens.size()) {
            std::cerr << "Error: if语句条件后意外结束，缺少右括号" << std::endl;
            print_context(tokens, if_start_index);
            return nullptr;
        }
        
        if (tokens[i].type == TokenType::RParen) {
            ++i; // 消费右括号
        } else {
            std::cerr << "Error: if语句条件缺少右括号 ')'，在行" << tokens[i-1].line << "之后" << std::endl;
            print_context(tokens, i);
            
            // 尝试恢复：寻找左大括号
            while (i < tokens.size() && tokens[i].type != TokenType::LBrace) {
                ++i;
            }
            
            if (i >= tokens.size()) {
                return nullptr; // 找不到左大括号，无法继续
            }
        }
        
        // 检查左大括号
        if (i >= tokens.size()) {
            std::cerr << "Error: if语句右括号后意外结束，缺少函数体" << std::endl;
            print_context(tokens, if_start_index);
            return nullptr;
        }
        
        if (tokens[i].type == TokenType::LBrace) {
            ++i; // 消费左大括号
        } else {
            std::cerr << "Error: if语句缺少左大括号 '{'" << std::endl;
            print_context(tokens, i);
            
            // 尝试恢复：创建一个空块并返回
            auto emptyThenBlock = std::make_unique<BlockStmt>();
            return std::make_unique<IfStmt>(std::move(cond), std::move(emptyThenBlock), nullptr);
        }
        
        // 解析then块
        std::cerr << "Debug - 解析if的then块" << std::endl;
        std::unique_ptr<BlockStmt> thenBlock;
        
        try {
            thenBlock = parse_block(tokens, i, false);
            std::cerr << "Debug - if的then块解析完成，包含" << thenBlock->statements.size() << "条语句" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: 解析if的then块时出错: " << e.what() << std::endl;
            // 创建空块作为恢复措施
            thenBlock = std::make_unique<BlockStmt>();
            
            // 尝试定位到else关键字或接下来的语句
            while (i < tokens.size() && 
                   tokens[i].type != TokenType::Else && 
                   tokens[i].type != TokenType::Semicolon &&
                   tokens[i].type != TokenType::RBrace) {
                ++i;
            }
            
            if (i >= tokens.size()) {
                return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock), nullptr);
            }
        }
        
        // 检查是否有else块
        std::unique_ptr<BlockStmt> elseBlock;
        if (i < tokens.size() && tokens[i].type == TokenType::Else) {
            int else_line = tokens[i].line;
            ++i; // 消费else关键字
            std::cerr << "Debug - 开始解析else块，行" << else_line << std::endl;
            
            if (i >= tokens.size()) {
                std::cerr << "Error: else关键字后意外结束" << std::endl;
                return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock), nullptr);
            }
            
            // 检查左大括号
            if (tokens[i].type == TokenType::LBrace) {
                ++i; // 消费左大括号
            } else {
                std::cerr << "Error: else块缺少左大括号 '{'" << std::endl;
                print_context(tokens, i);
                
                // 返回没有else块的if语句
                return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock), nullptr);
            }
            
            // 解析else块
            try {
                elseBlock = parse_block(tokens, i, false);
                std::cerr << "Debug - else块解析完成，包含" << elseBlock->statements.size() << "条语句" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: 解析else块时出错: " << e.what() << std::endl;
                // 创建空块作为恢复措施
                elseBlock = std::make_unique<BlockStmt>();
            }
        }
        
        std::cerr << "Debug - if语句解析完成，定义于行 " << if_line 
                  << ", 结束于行 " << (i < tokens.size() ? tokens[i-1].line : -1) << std::endl;
                  
        return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock), std::move(elseBlock));
    }
    
    // include "module"
    if (tokens[i].type == TokenType::Include && tokens[i+1].type == TokenType::String) {
        std::string mod = tokens[i+1].text;
        i += 2;
        if (tokens[i].type == TokenType::Semicolon) ++i;
        return std::make_unique<IncludeStmt>(mod);
    }
    
    // 末尾加更详细的调试输出和错误处理
    if (i < tokens.size()) {
        // 遇到无法识别的token，提供更详细的上下文
        std::cerr << "错误: 无法识别的语法元素 '" << tokens[i].text 
                  << "' (类型=" << static_cast<int>(tokens[i].type) 
                  << ") 位于行 " << tokens[i].line 
                  << " 列 " << tokens[i].column << std::endl;
                  
        // 打印上下文
        std::cerr << "上下文: ";
        size_t context_start = i > 5 ? i - 5 : 0;
        size_t context_end = std::min(i + 5, tokens.size() - 1);
        
        for (size_t j = context_start; j <= context_end; j++) {
            if (j == i) {
                std::cerr << "[" << tokens[j].text << "] ";
            } else {
                std::cerr << tokens[j].text << " ";
            }
        }
        std::cerr << std::endl;
        
        // 尝试提供可能的错误原因
        if (tokens[i].type == TokenType::RBrace) {
            std::cerr << "提示: 发现额外的右花括号 '}'，可能是块嵌套问题" << std::endl;
        } else if (tokens[i].type == TokenType::RParen) {
            std::cerr << "提示: 发现额外的右括号 ')'，检查表达式或条件语句" << std::endl;
        } else if (tokens[i].type == TokenType::Else) {
            std::cerr << "提示: 'else'关键字前缺少完整的if语句" << std::endl;
        }
    } else {
        std::cerr << "错误: 解析器在文件末尾意外结束" << std::endl;
    }
    
    // 返回空指针表示解析失败
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parse(const std::vector<Token>& tokens) {
    size_t i = 0;
    try {
        std::cerr << "Debug - 开始解析文件，总token数：" << tokens.size() << std::endl;
        auto result = parse_block(tokens, i, true); // 主 block
        
        // 验证是否消费了所有token
        if (i < tokens.size() && tokens[i].type != TokenType::EndOfFile) {
            std::cerr << "警告: 解析完成后还有未处理的token，从位置 " << i << " 开始" << std::endl;
            std::cerr << "未处理的第一个token: " << tokens[i].text 
                      << " (行" << tokens[i].line << ")" << std::endl;
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "解析错误: " << e.what() << std::endl;
        return nullptr;
    }
}
