#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <iostream>
#include <cmath>
#include <exception>
#include <fstream>
#include <sstream>

// Exception for return statements
class ReturnException : public std::exception {
public:
    Value value;
    ReturnException(const Value& v) : value(v) {}
};

// Exception for break statements
class BreakException : public std::exception {
public:
    BreakException() {}
};

// Exception for continue statements
class ContinueException : public std::exception {
public:
    ContinueException() {}
};

// Scope stack operations
void Interpreter::push_scope() {
    variable_stack.emplace_back();
}

void Interpreter::pop_scope() {
    if (variable_stack.size() > 1) variable_stack.pop_back();
}

Value Interpreter::get_variable(const std::string& name) const {
    for (auto it = variable_stack.rbegin(); it != variable_stack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    std::cerr << "Error: Undefined variable '" << name << "'" << std::endl;
    return Value("<undefined>");
}

void Interpreter::set_variable(const std::string& name, const Value& val) {
    if (!variable_stack.empty()) {
        (*variable_stack.rbegin())[name] = val;
    }
}

void Interpreter::execute(const std::unique_ptr<Statement>& node) {
    if (!node) return;
      if (auto* p = dynamic_cast<PrintStmt*>(node.get())) {
        if (p->expr) {
            Value val = eval(p->expr.get());
            std::cout << val.to_string() << std::endl;
        } else {
            std::cerr << "Error: Null expression in print statement" << std::endl;
            std::cout << "<error>" << std::endl;
        }
    }    else if (auto* v = dynamic_cast<VarDeclStmt*>(node.get())) {
        if (v->expr) {
            Value val = eval(v->expr.get());
            set_variable(v->name, val);
        } else {
            std::cerr << "Error: Null expression in variable declaration of '" << v->name << "'" << std::endl;
            set_variable(v->name, Value("<undefined>"));
        }
    }
    else if (auto* a = dynamic_cast<AssignStmt*>(node.get())) {
        if (!a->expr) {
            std::cerr << "Error: Null expression in assignment to '" << a->name << "'" << std::endl;
            return;
        }
        Value val = eval(a->expr.get());
        set_variable(a->name, val);
    }
    else if (auto* ifs = dynamic_cast<IfStmt*>(node.get())) {
        if (!ifs->condition) {
            std::cerr << "Error: Null condition in if statement" << std::endl;
            return;
        }
        Value cond = eval(ifs->condition.get());
        bool cond_true = cond.type == Value::Type::Int ? std::get<int>(cond.data) != 0 : false;
        if (cond_true) {
            for (auto& stmt : ifs->thenBlock->statements) execute(stmt);
        } else if (ifs->elseBlock) {
            for (auto& stmt : ifs->elseBlock->statements) execute(stmt);
        }
    } 
    else if (auto* ws = dynamic_cast<WhileStmt*>(node.get())) {
        if (!ws->condition) {
            std::cerr << "Error: Null condition in while statement" << std::endl;
            return;
        }
        while (true) {
            Value cond = eval(ws->condition.get());
            bool cond_true = cond.type == Value::Type::Int ? std::get<int>(cond.data) != 0 : false;
            if (!cond_true) break;
            try {
                for (auto& stmt : ws->body->statements) execute(stmt);
            } catch (BreakException&) {
                break; // Exit the loop
            } catch (ContinueException&) {
                continue; // Skip to next iteration
            }
        }
    } 
    else if (auto* func = dynamic_cast<FuncDefStmt*>(node.get())) {
        functions[func->name] = func;
    } 
    else if (auto* block = dynamic_cast<BlockStmt*>(node.get())) {
        for (auto& stmt : block->statements) execute(stmt);
    } 
    else if (auto* ret = dynamic_cast<ReturnStmt*>(node.get())) {
        Value val = eval(ret->expr.get());
        throw ReturnException(val);
    } 
    else if (auto* breakStmt = dynamic_cast<BreakStmt*>(node.get())) {
        throw BreakException();
    } 
    else if (auto* contStmt = dynamic_cast<ContinueStmt*>(node.get())) {
        throw ContinueException();
    } 
    else if (auto* includeStmt = dynamic_cast<IncludeStmt*>(node.get())) {
        if (!load_module(includeStmt->module)) {
            std::cerr << "Error: Failed to include module '" << includeStmt->module << "'" << std::endl;
        }
    }    else if (auto* exprstmt = dynamic_cast<ExprStmt*>(node.get())) {
        if (exprstmt->expr) {
            eval(exprstmt->expr.get());
        } else {
            std::cerr << "Error: Empty expression statement" << std::endl;
        }
    }
}

Value Interpreter::eval(const ASTNode* node) {
    if (!node) {
        // Avoid printing the same error message repeatedly
        static bool errorPrinted = false;
        if (!errorPrinted) {
            std::cerr << "Error: Attempted to evaluate null expression" << std::endl;
            errorPrinted = true;
        }
        return Value("<error>");
    }
    
    if (auto* lit = dynamic_cast<const LiteralExpr*>(node)) {
        // Check if it's a number
        try {
            int i = std::stoi(lit->value);
            return Value(i);
        } catch (...) {
            return Value(lit->value);
        }
    } 
    else if (auto* id = dynamic_cast<const IdentifierExpr*>(node)) {
        return get_variable(id->name);
    } 
    else if (auto* bin = dynamic_cast<const BinaryExpr*>(node)) {
        Value l = eval(bin->left.get());
        Value r = eval(bin->right.get());
        
        // String concatenation
        if (bin->op == "+") {
            if (l.type == Value::Type::String || r.type == Value::Type::String) {
                return Value(l.to_string() + r.to_string());
            } else if (l.type == Value::Type::Int && r.type == Value::Type::Int) {
                return Value(std::get<int>(l.data) + std::get<int>(r.data));
            }
        }
          // Type checking for arithmetic operations
        if ((bin->op == "-" || bin->op == "*" || bin->op == "/" || 
             bin->op == "//" || bin->op == "%" || bin->op == "^")) {
            // Check left operand
            if (l.type != Value::Type::Int) {
                std::cerr << "Error: Left operand of '" << bin->op << "' must be an integer, got " 
                          << (l.type == Value::Type::String ? "string" : 
                             l.type == Value::Type::Array ? "array" : "unknown type") << std::endl;
                return Value("<type error>");
            }
            // Check right operand
            if (r.type != Value::Type::Int) {
                std::cerr << "Error: Right operand of '" << bin->op << "' must be an integer, got " 
                          << (r.type == Value::Type::String ? "string" : 
                             r.type == Value::Type::Array ? "array" : "unknown type") << std::endl;
                return Value("<type error>");
            }
        }
        
        int li = l.type == Value::Type::Int ? std::get<int>(l.data) : 0;
        int ri = r.type == Value::Type::Int ? std::get<int>(r.data) : 0;
        
        if (bin->op == "-") return Value(li - ri);
        if (bin->op == "*") return Value(li * ri);
        
        if (bin->op == "/") {
            if (ri == 0) {
                std::cerr << "Error: Division by zero" << std::endl;
                return Value("<div by zero>");
            }
            return Value(li / ri);
        }
        
        if (bin->op == "//") {
            if (ri == 0) {
                std::cerr << "Error: Integer division by zero" << std::endl;
                return Value("<div by zero>");
            }
            return Value(li / ri);
        }
        
        if (bin->op == "%") {
            if (ri == 0) {
                std::cerr << "Error: Modulo by zero" << std::endl;
                return Value("<mod by zero>");
            }
            return Value(li % ri);
        }
        
        if (bin->op == "^") {
            if (ri < 0) {
                std::cerr << "Error: Negative exponent not supported" << std::endl;
                return Value("<neg exp>");
            }
            int res = 1;
            for (int j = 0; j < ri; ++j) res *= li;
            return Value(res);
        }
        
        // Comparison operators
        if (bin->op == "==" || bin->op == "!=" || bin->op == "<" || 
            bin->op == "<=" || bin->op == ">" || bin->op == ">=") {
            
            // Handle different type combinations
            if (l.type == Value::Type::Int && r.type == Value::Type::Int) {
                int li = std::get<int>(l.data);
                int ri = std::get<int>(r.data);
                
                if (bin->op == "==") return Value(li == ri ? 1 : 0);
                if (bin->op == "!=") return Value(li != ri ? 1 : 0);
                if (bin->op == "<") return Value(li < ri ? 1 : 0);
                if (bin->op == "<=") return Value(li <= ri ? 1 : 0);
                if (bin->op == ">") return Value(li > ri ? 1 : 0);
                if (bin->op == ">=") return Value(li >= ri ? 1 : 0);
            }
            else if (l.type == Value::Type::String && r.type == Value::Type::String) {
                std::string ls = std::get<std::string>(l.data);
                std::string rs = std::get<std::string>(r.data);
                
                if (bin->op == "==") return Value(ls == rs ? 1 : 0);
                if (bin->op == "!=") return Value(ls != rs ? 1 : 0);
                if (bin->op == "<") return Value(ls < rs ? 1 : 0);
                if (bin->op == "<=") return Value(ls <= rs ? 1 : 0);
                if (bin->op == ">") return Value(ls > rs ? 1 : 0);
                if (bin->op == ">=") return Value(ls >= rs ? 1 : 0);
            }
            else {
                // Type mismatch - only equality/inequality make sense
                if (bin->op == "==") return Value(0); // Different types are never equal
                if (bin->op == "!=") return Value(1); // Different types are always not equal
                
                std::cerr << "Error: Cannot compare " 
                          << (l.type == Value::Type::String ? "string" : 
                             l.type == Value::Type::Array ? "array" : "integer")
                          << " with "
                          << (r.type == Value::Type::String ? "string" : 
                             r.type == Value::Type::Array ? "array" : "integer")
                          << " using operator '" << bin->op << "'" << std::endl;
                return Value("<comparison error>");
            }
        }
        
        std::cerr << "Error: Unknown binary operator '" << bin->op << "'" << std::endl;
        return Value("<unknown op>");
    } 
    else if (auto* unary = dynamic_cast<const UnaryExpr*>(node)) {
        Value v = eval(unary->operand.get());
        
        if (v.type != Value::Type::Int) {
            std::cerr << "Error: Unary operator requires integer operand" << std::endl;
            return Value("<type error>");
        }
        
        int vi = std::get<int>(v.data);
        
        if (unary->op == "-") return Value(-vi);
        if (unary->op == "!") {
            if (vi < 0) {
                std::cerr << "Error: Factorial of negative number" << std::endl;
                return Value("<factorial error>");
            }
            int res = 1;
            for (int j = 1; j <= vi; ++j) res *= j;
            return Value(res);
        }
        
        std::cerr << "Error: Unknown unary operator '" << unary->op << "'" << std::endl;
        return Value("<unknown op>");
    }    // Support function calls
    else if (auto* call = dynamic_cast<const CallExpr*>(node)) {
        // Check if function exists
        auto it = functions.find(call->callee);
        if (it != functions.end()) {
            FuncDefStmt* func = it->second;
            if (!func) {
                std::cerr << "Error: Function object for '" << call->callee << "' is null" << std::endl;
                return Value("<func error>");
            }
            push_scope();
            
            // Check parameter count
            if (call->args.size() > func->params.size()) {
                std::cerr << "Warning: Too many arguments provided to function '" << call->callee 
                          << "'. Expected " << func->params.size() << ", got " << call->args.size() << std::endl;
            }
              // Pass arguments
            for (size_t j = 0; j < func->params.size(); ++j) {
                if (j < call->args.size()) {
                    if (call->args[j]) {
                        set_variable(func->params[j], eval(call->args[j].get()));
                    } else {
                        std::cerr << "Error: Null argument " << j+1 << " in call to function '" 
                                  << call->callee << "'" << std::endl;
                        set_variable(func->params[j], Value("<null arg>"));
                    }
                } else {
                    std::cerr << "Warning: Missing argument '" << func->params[j] 
                              << "' in call to function '" << call->callee << "'" << std::endl;
                    set_variable(func->params[j], Value("<undefined>"));
                }
            }
            
            // Execute function body, capture return
            try {
                for (const auto& stmt : func->body->statements) execute(stmt);
            } catch (const ReturnException& re) {
                pop_scope();
                return re.value;
            }
            pop_scope();
            return Value(); // Default value when no return
        }
        std::cerr << "Error: Call to undefined function '" << call->callee << "'" << std::endl;
        return Value("<undefined function>");
    }
    
    std::cerr << "Error: Unsupported expression type" << std::endl;
    return Value("<type error>");
}

// Load and execute module
bool Interpreter::load_module(const std::string& module_name) {
    // Check if already loaded to avoid circular imports
    if (loaded_modules.find(module_name) != loaded_modules.end()) {
        return true;  // Already loaded
    }
    
    // Build possible file paths
    std::string filename = module_name;
    if (filename.find(".lm") == std::string::npos) {
        filename += ".lm";  // Automatically add extension
    }

    // Try different paths: current directory, examples directory, etc.
    std::vector<std::string> search_paths = {"", "./", "./examples/", "../examples/"};
    
    std::ifstream file;
    std::string full_path;
    for (const auto& path : search_paths) {
        full_path = path + filename;
        file.open(full_path);
        if (file) break;
    }

    if (!file) {
        std::cerr << "Error: Cannot load module '" << module_name << "'" << std::endl;
        std::cerr << "  Searched in: ";
        for (size_t i = 0; i < search_paths.size(); ++i) {
            std::cerr << search_paths[i] + filename;
            if (i < search_paths.size() - 1) std::cerr << ", ";
        }
        std::cerr << std::endl;
        return false;
    }

    // Read file into string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // Record module as loaded
    loaded_modules.insert(module_name);

    // Lexical and syntax analysis
    auto tokens = Lexer::tokenize(source);
    auto ast = Parser::parse(tokens);
    
    if (!ast) {
        std::cerr << "Error: Failed to parse module '" << module_name << "'" << std::endl;
        loaded_modules.erase(module_name); // Remove from loaded modules on failure
        return false;
    }

    // Execute module code (should be a block statement)
    auto* block = dynamic_cast<BlockStmt*>(ast.get());
    if (block) {
        // Create a new scope for module execution
        push_scope();
        try {
            for (auto& stmt : block->statements) {
                execute(stmt);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Exception while executing module '" << module_name << "': " << e.what() << std::endl;
            pop_scope();
            return false;
        }
        pop_scope();
        return true;
    }

    std::cerr << "Error: Module '" << module_name << "' does not contain valid code" << std::endl;
    return false;
}
