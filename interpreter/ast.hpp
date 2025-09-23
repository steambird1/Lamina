#pragma once
#include "value.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

// AST 基类
struct ASTNode {
    virtual ~ASTNode() = default;
};

// 表达式基类
struct Expression : public ASTNode {
    std::string source;// 保存表达式源码
};

// 语句基类
struct Statement : public ASTNode {};

// 字面量
struct LiteralExpr final : public Expression {
    Value::Type type;
    std::string value;
    LiteralExpr(std::string  v, const Value::Type type) : type(type), value(std::move(v)) {}
};

// 标识符
struct IdentifierExpr final : public Expression {
    std::string name;
    explicit IdentifierExpr(std::string  n) : name(std::move(n)) {}
};

// 变量引用
struct VarExpr final : public Expression {
    std::string name;
    explicit VarExpr(std::string  n) : name(std::move(n)) {}
};

// 二元运算
struct BinaryExpr final : public Expression {
    std::string op;
    std::unique_ptr<Expression> left, right;
    BinaryExpr(std::string  o, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
};

// 一元运算
struct UnaryExpr final : public Expression {
    std::string op;
    std::unique_ptr<Expression> operand;
    UnaryExpr(std::string  o, std::unique_ptr<Expression> e)
        : op(std::move(o)), operand(std::move(e)) {}
};

// 变量声明
struct VarDeclStmt final : public Statement {
    std::string name;
    std::unique_ptr<Expression> expr;
    VarDeclStmt(std::string  n, std::unique_ptr<Expression> e)
        : name(std::move(n)), expr(std::move(e)) {}
};

// 赋值
struct AssignStmt final : public Statement {
    std::string name;
    std::unique_ptr<Expression> expr;
    AssignStmt(std::string  n, std::unique_ptr<Expression> e)
        : name(std::move(n)), expr(std::move(e)) {}
};


// 复合语句块
struct BlockStmt final : public Statement {
    std::vector<std::unique_ptr<Statement>> statements;
};

// if 语句
struct IfStmt final : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStmt> thenBlock;
    std::unique_ptr<BlockStmt> elseBlock;
    IfStmt(std::unique_ptr<Expression> cond, std::unique_ptr<BlockStmt> thenB, std::unique_ptr<BlockStmt> elseB)
        : condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(std::move(elseB)) {}
};

// while 语句
struct WhileStmt final : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStmt> body;
    WhileStmt(std::unique_ptr<Expression> cond, std::unique_ptr<BlockStmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// 函数定义
struct FuncDefStmt final : public Statement {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<BlockStmt> body;
    FuncDefStmt(std::string  n, const std::vector<std::string>& p, std::unique_ptr<BlockStmt> b)
        : name(std::move(n)), params(p), body(std::move(b)) {}
};

struct GetMemberStmt;
// 函数调用
struct CallExpr final : public Expression {
    std::variant<
        std::unique_ptr<GetMemberStmt>,
        std::unique_ptr<IdentifierExpr>
    > callee;
    std::vector<std::unique_ptr<Expression>> args;
    CallExpr(std::unique_ptr<IdentifierExpr> c, std::vector<std::unique_ptr<Expression>> a)
        : callee(std::move(c)), args(std::move(a)) {}
    CallExpr(std::unique_ptr<GetMemberStmt> c, std::vector<std::unique_ptr<Expression>> a)
        : callee(std::move(c)), args(std::move(a)) {}
};

// 命名空间函数调用
struct NamespaceCallExpr final : public Expression {
    std::string namespace_name;
    std::string function_name;
    std::vector<std::unique_ptr<Expression>> args;
    NamespaceCallExpr(std::string  ns, std::string  fn, std::vector<std::unique_ptr<Expression>> a)
        : namespace_name(std::move(ns)), function_name(std::move(fn)), args(std::move(a)) {}
};

// 数组字面量
struct ArrayExpr final : public Expression {
    std::vector<std::unique_ptr<Expression>> elements;
    explicit ArrayExpr(std::vector<std::unique_ptr<Expression>> elems)
        : elements(std::move(elems)) {}
};

// return 语句
struct ReturnStmt final : public Statement {
    std::unique_ptr<Expression> expr;
    explicit ReturnStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

// include 语句
struct IncludeStmt final : public Statement {
    std::string module;
    explicit IncludeStmt(std::string  m) : module(std::move(m)) {}
};

// 空语句
struct NullStmt final : public Statement {
    NullStmt() = default;
};

// break 语句
struct BreakStmt final : public Statement {
    BreakStmt() = default;
};

// continue 语句
struct ContinueStmt final : public Statement {
    ContinueStmt() = default;
};

// 表达式语句
struct ExprStmt final : public Statement {
    std::unique_ptr<Expression> expr;
    explicit ExprStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

// Struct声明
struct StructDeclStmt final : public Statement {
    std::string name;
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> init_vec;
    explicit StructDeclStmt(
            std::string  n,
            std::vector<std::pair<std::string, std::unique_ptr<Expression>>> v)
        : name(std::move(n)), init_vec(std::move(v)) {}
};

// 匿名Struct声明
struct LambdaStructDeclStmt final : public Statement {
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> init_vec;
    explicit LambdaStructDeclStmt(std::vector<std::pair<std::string, std::unique_ptr<Expression>>> v)
        : init_vec(std::move(v)) {}
};

// Define语句（用于设置常量，如递归深度）
struct DefineStmt final : public Statement {
    std::string name;
    std::unique_ptr<Expression> value;
    DefineStmt(std::string  n, std::unique_ptr<Expression> v)
        : name(std::move(n)), value(std::move(v)) {}
};

// BigInt变量声明
struct BigIntDeclStmt final : public Statement {
    std::string name;
    std::unique_ptr<Expression> init_value;
    explicit BigIntDeclStmt(std::string  n, std::unique_ptr<Expression> v = nullptr)
        : name(std::move(n)), init_value(std::move(v)) {}
};

// 获取成员
struct GetMemberStmt final : public Statement {
    std::variant<
        std::unique_ptr<GetMemberStmt>,
        std::unique_ptr<IdentifierExpr>
    > father;
    std::unique_ptr<IdentifierExpr> child;
    GetMemberStmt(std::unique_ptr<GetMemberStmt> f, std::unique_ptr<IdentifierExpr> c)
        :father(std::move(f)), child(std::move(c)) {}
    GetMemberStmt(std::unique_ptr<IdentifierExpr> f, std::unique_ptr<IdentifierExpr> c)
        :father(std::move(f)), child(std::move(c)) {}
};

// 获取项
struct GetItemStmt final : public Statement {
    std::variant<
        std::unique_ptr<GetMemberStmt>,
        std::unique_ptr<IdentifierExpr>
    > father;
    std::vector<std::unique_ptr<Expression>> params;
    GetItemStmt(std::unique_ptr<GetMemberStmt> f, std::vector<std::unique_ptr<Expression>> p)
        : father(std::move(f)), params(std::move(p)) {}
    GetItemStmt(std::unique_ptr<IdentifierExpr> c, std::vector<std::unique_ptr<Expression>> p)
        : father(std::move(c)), params(std::move(p)) {}
};

// 声明匿名函数
struct LambdaDeclStmt final : public Expression {
    std::vector<std::unique_ptr<Expression>> params;
    std::unique_ptr<BlockStmt> body;
    LambdaDeclStmt(const std::vector<std::unique_ptr<Expression>>& p, std::unique_ptr<BlockStmt> b)
        : params(p), body(std::move(b)) {}
};