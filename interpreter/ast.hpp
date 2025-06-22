#pragma once
#include <string>
#include <memory>

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct PrintNode : public ASTNode {
    std::unique_ptr<ASTNode> expr;
    PrintNode(std::unique_ptr<ASTNode> e) : expr(std::move(e)) {}
};

struct VarDeclNode : public ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> expr;
    VarDeclNode(const std::string& n, std::unique_ptr<ASTNode> e) : name(n), expr(std::move(e)) {}
};

struct IdentifierNode : public ASTNode {
    std::string name;
    IdentifierNode(const std::string& n) : name(n) {}
};

struct LiteralNode : public ASTNode {
    std::string value;
    LiteralNode(const std::string& v) : value(v) {}
};
