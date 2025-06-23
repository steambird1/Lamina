#pragma once
#include <string>
#include <vector>
#include <memory>

// AST 基类
struct ASTNode {
    virtual ~ASTNode() = default;
};

// 表达式基类
struct Expression : public ASTNode {};

// 语句基类
struct Statement : public ASTNode {};

// 字面量
struct LiteralExpr : public Expression {
    std::string value;
    LiteralExpr(const std::string& v) : value(v) {}
};

// 标识符
struct IdentifierExpr : public Expression {
    std::string name;
    IdentifierExpr(const std::string& n) : name(n) {}
};

// 二元运算
struct BinaryExpr : public Expression {
    std::string op;
    std::unique_ptr<Expression> left, right;
    BinaryExpr(const std::string& o, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

// 一元运算
struct UnaryExpr : public Expression {
    std::string op;
    std::unique_ptr<Expression> operand;
    UnaryExpr(const std::string& o, std::unique_ptr<Expression> e)
        : op(o), operand(std::move(e)) {}
};

// 变量声明
struct VarDeclStmt : public Statement {
    std::string name;
    std::unique_ptr<Expression> expr;
    VarDeclStmt(const std::string& n, std::unique_ptr<Expression> e)
        : name(n), expr(std::move(e)) {}
};

// 赋值
struct AssignStmt : public Statement {
    std::string name;
    std::unique_ptr<Expression> expr;
    AssignStmt(const std::string& n, std::unique_ptr<Expression> e)
        : name(n), expr(std::move(e)) {}
};

// print 语句
struct PrintStmt : public Statement {
    std::unique_ptr<Expression> expr;
    PrintStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

// 复合语句块
struct BlockStmt : public Statement {
    std::vector<std::unique_ptr<Statement>> statements;
};

// if 语句
struct IfStmt : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStmt> thenBlock;
    std::unique_ptr<BlockStmt> elseBlock;
    IfStmt(std::unique_ptr<Expression> cond, std::unique_ptr<BlockStmt> thenB, std::unique_ptr<BlockStmt> elseB)
        : condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(std::move(elseB)) {}
};

// while 语句
struct WhileStmt : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStmt> body;
    WhileStmt(std::unique_ptr<Expression> cond, std::unique_ptr<BlockStmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// 函数定义
struct FuncDefStmt : public Statement {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<BlockStmt> body;
    FuncDefStmt(const std::string& n, const std::vector<std::string>& p, std::unique_ptr<BlockStmt> b)
        : name(n), params(p), body(std::move(b)) {}
};

// 函数调用
struct CallExpr : public Expression {
    std::string callee;
    std::vector<std::unique_ptr<Expression>> args;
    CallExpr(const std::string& c, std::vector<std::unique_ptr<Expression>> a)
        : callee(c), args(std::move(a)) {}
};

// return 语句
struct ReturnStmt : public Statement {
    std::unique_ptr<Expression> expr;
    ReturnStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

// include 语句
struct IncludeStmt : public Statement {
    std::string module;
    IncludeStmt(const std::string& m) : module(m) {}
};

// break 语句
struct BreakStmt : public Statement {
    BreakStmt() {}
};

// continue 语句
struct ContinueStmt : public Statement {
    ContinueStmt() {}
};

// 表达式语句
struct ExprStmt : public Statement {
    std::unique_ptr<Expression> expr;
    ExprStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};
