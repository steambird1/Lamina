#include "interpreter.hpp"
#include "lamina_api/lamina.hpp"
#include "../extensions/standard/cas.hpp"
#include "lamina_api/symbolic.hpp"
#include <optional>
#include <iostream>

enum VALUE_TYPE : int {
    VALUE_IS_STRING = 1,
    VALUE_IS_ARRAY = 2,
    VALUE_IS_IRRATIONAL = 4,
    VALUE_IS_RATIONAL = 8,
    VALUE_IS_NUMERIC = 16,
    VALUE_IS_SYMBOLIC = 32,
    VALUE_IS_BIGINT = 64,
    VALUE_IS_INT = 128,
    VALUE_IS_FLOAT = 256,
    VALUE_IS_NOTSURE = 0
};

static std::shared_ptr<SymbolicExpr> GET_SYMBOLICEXPR(const Value *val, enum VALUE_TYPE type);
static int GET_VALUE_TYPE(const Value *val);
static Value HANDLE_BINARYEXPR_ADD(Value *l, Value *r);
static Value HANDLE_BINARYEXPR_STR_ADD_STR(Value *l, Value *r);

std::shared_ptr<SymbolicExpr> GET_SYMBOLICEXPR(const Value *val, int type)
    {
        switch (type & (~int(VALUE_IS_NUMERIC))) {
            case VALUE_IS_SYMBOLIC:
                return std::get<std::shared_ptr<SymbolicExpr>>(val->data);
            case VALUE_IS_IRRATIONAL:
                return std::get<::Irrational>(val->data).to_symbolic();
            case VALUE_IS_RATIONAL:
                return SymbolicExpr::number(std::get<::Rational>(val->data));
            case VALUE_IS_BIGINT:
                return SymbolicExpr::number(std::get<::BigInt>(val->data));
            case VALUE_IS_INT:
                return SymbolicExpr::number(std::get<int>(val->data));
            case VALUE_IS_FLOAT:
                return SymbolicExpr::number(::Rational::from_double(std::get<double>(val->data)));
            default:
                return SymbolicExpr::number(0);
        }
    }

int GET_VALUE_TYPE(const Value *val)
    {
        if (val->is_string())
            return VALUE_IS_STRING;
        if (val->is_array())
            return VALUE_IS_ARRAY;
        if (val->is_irrational())
            return VALUE_IS_IRRATIONAL | VALUE_IS_NUMERIC;
        if (val->is_rational())
            return VALUE_IS_RATIONAL | VALUE_IS_NUMERIC;
        if (val->is_symbolic())
            return VALUE_IS_SYMBOLIC | VALUE_IS_NUMERIC;
        if (val->is_bigint())
            return VALUE_IS_BIGINT | VALUE_IS_NUMERIC;
        if (val->is_int())
            return VALUE_IS_INT | VALUE_IS_NUMERIC;
        if (val->is_float())
            return VALUE_IS_FLOAT | VALUE_IS_NUMERIC;
		if (val->is_numeric())
            return VALUE_IS_NUMERIC;	// Quite strange. Why do we need this?
        return VALUE_IS_NOTSURE;
    }

Value HANDLE_BINARYEXPR_ADD(Value *l, Value *r)
    {
        auto ltype = GET_VALUE_TYPE(l);
        auto rtype = GET_VALUE_TYPE(r);
        if (ltype & VALUE_IS_STRING && rtype & VALUE_IS_STRING) {
            return HANDLE_BINARYEXPR_STR_ADD_STR(l, r);
        } else if (ltype & VALUE_IS_STRING || rtype & VALUE_IS_STRING) {
            return Value(l->to_string() + r->to_string());
        } else if (ltype & VALUE_IS_ARRAY && rtype & VALUE_IS_ARRAY) {
            // Vector addition
            return l->vector_add(r);
        // 只要有一方是 Irrational 或 Symbolic，优先生成符号表达式
        } else if (((ltype & VALUE_IS_IRRATIONAL)
                    || (ltype & VALUE_IS_SYMBOLIC)
                    || (rtype & VALUE_IS_IRRATIONAL)
                    || (rtype & VALUE_IS_SYMBOLIC))
                && (ltype & VALUE_IS_NUMERIC)
                && (rtype & VALUE_IS_NUMERIC)) {
            std::shared_ptr<SymbolicExpr> leftExpr = GET_SYMBOLICEXPR(l, ltype);
            std::shared_ptr<SymbolicExpr> rightExpr = GET_SYMBOLICEXPR(r, rtype);
            return Value(SymbolicExpr::add(leftExpr, rightExpr)->simplify());
        } else if (ltype & VALUE_IS_NUMERIC && rtype & VALUE_IS_NUMERIC) {
            // BigInt 优先：如果任一为 BigInt，结果为 BigInt
            if (l->is_bigint() || r->is_bigint()) {
                ::BigInt lb = l->is_bigint() ? std::get<::BigInt>(l->data) : ::BigInt(l->as_number());
                ::BigInt rb = r->is_bigint() ? std::get<::BigInt>(r->data) : ::BigInt(r->as_number());
                return Value(lb + rb);
            }
            // If either operand is rational, use rational arithmetic
            if (l->is_rational() || r->is_rational()) {
                ::Rational result = l->as_rational() + r->as_rational();
                return Value(result);
            }

            double result = l->as_number() + r->as_number();// Return int if both operands are int and result is the whole
            if (l->is_int() && r->is_int()) {
                // check overflow and underflow
                if (static_cast<int>(result) == INT_MAX ||
                    static_cast<int>(result) == INT_MIN) {
                    return Value(BigInt(l->as_number()) + BigInt(r->as_number()));
                }
                return Value(static_cast<int>(result));
            }
            return Value(result);
        } else {
            L_ERR("Cannot add " + l->to_string() + " and " + r->to_string());
        }
    }

Value HANDLE_BINARYEXPR_STR_ADD_STR(Value *l, Value *r)
    {
        std::string ls = l->to_string();
        std::string rs = r->to_string();
        try {
            // Try to parse both strings as CAS expressions and combine them symbolically
            LaminaCAS::Parser pl(ls);
            auto e1 = pl.parse();
            LaminaCAS::Parser pr(rs);
            auto e2 = pr.parse();
            // attempt structural merge: if e1 and e2 are Multiply(Number, Variable) or vice versa,
            // then combine coefficients
            auto try_extract_coeff_var = [](const LaminaCAS::Expr* ex) -> std::optional<std::pair<double, std::string>> {
                // Multiply(Number, Variable)
                if (auto mul = dynamic_cast<const LaminaCAS::Multiply*>(ex)) {
                    const LaminaCAS::Expr* left = mul->clone().release();
                    const LaminaCAS::Expr* right = nullptr; // will be set below
                                                            // We can't easily access internals (unique_ptr ownership), so inspect via toString heuristic
                    std::string s = mul->toString();
                    // fallback to string parsing
                    // very simple: look for pattern like "<num> * <var>"
                    size_t star = s.find('*');
                    if (star != std::string::npos) {
                        std::string a = s.substr(0, star);
                        std::string b = s.substr(star+1);
                        auto trim = [](std::string &str) {
                            while (!str.empty() && std::isspace((unsigned char)str.front())) str.erase(str.begin());
                            while (!str.empty() && std::isspace((unsigned char)str.back())) str.pop_back();
                        };
                        trim(a); trim(b);
                        try {
                            double va = std::stod(a);
                            // b is var
                            return std::make_pair(va, b);
                        } catch (...) {}
                        try {
                            double vb = std::stod(b);
                            return std::make_pair(vb, a);
                        } catch (...) {}
                    }
                }
                // Number alone
                if (auto num = dynamic_cast<const LaminaCAS::Number*>(ex)) {
                    return std::make_pair(num->getValue(), std::string());
                }
                // Variable alone
                if (auto var = dynamic_cast<const LaminaCAS::Variable*>(ex)) {
                    return std::make_pair(1.0, var->toString());
                }
                return std::nullopt;
            };

            auto e1ptr = e1.get();
            auto e2ptr = e2.get();
            auto p1 = try_extract_coeff_var(e1ptr);
            auto p2 = try_extract_coeff_var(e2ptr);
            if (p1 && p2 && !p1->second.empty() && p1->second == p2->second) {
                double sum = p1->first + p2->first;
                std::string coeff = (std::floor(sum) == sum) ? std::to_string((int)sum) : std::to_string(sum);
                return Value(std::string("(" + coeff + " * " + p1->second + ")"));
            }

            auto combined = std::make_unique<LaminaCAS::Add>(std::move(e1), std::move(e2));
            auto simplified = combined->simplify();

            // Attempt a lightweight combine for simple like-terms of form a*x
            auto try_parse_coeff_var = [](const std::string &s) -> std::optional<std::pair<double,std::string>> {
                std::string t = s;
                // trim
                auto trim = [](std::string &str) {
                    while (!str.empty() && std::isspace((unsigned char)str.front())) str.erase(str.begin());
                    while (!str.empty() && std::isspace((unsigned char)str.back())) str.pop_back();
                };
                trim(t);
                // remove surrounding parentheses
                if (!t.empty() && t.front() == '(' && t.back() == ')') {
                    t = t.substr(1, t.size()-2);
                    trim(t);
                }
                // find '*'
                size_t star = t.find('*');
                if (star == std::string::npos) {
                    // maybe it's just a variable or a number
                    try {
                        double v = std::stod(t);
                        return std::make_pair(v, std::string());
                    } catch (...) {
                        // not a number -> treat as var with coeff 1
                        if (!t.empty() && std::isalpha((unsigned char)t[0])) return std::make_pair(1.0, t);
                        return std::nullopt;
                    }
                }
                std::string a = t.substr(0, star);
                std::string b = t.substr(star+1);
                trim(a); trim(b);
                // Try number*var or var*number
                try {
                    double va = std::stod(a);
                    // b should be variable
                    if (!b.empty() && std::isalpha((unsigned char)b[0])) return std::make_pair(va, b);
                } catch (...) {}
                try {
                    double vb = std::stod(b);
                    if (!a.empty() && std::isalpha((unsigned char)a[0])) return std::make_pair(vb, a);
                } catch (...) {}
                return std::nullopt;
            };

            auto lhs = try_parse_coeff_var(simplified->toString());
            // Also try parsing original operands when combined simplifier didn't flatten
            if (!lhs) {
                // fallback: try parse e1 and e2 original strings
                auto p1 = try_parse_coeff_var(ls);
                auto p2 = try_parse_coeff_var(rs);
                if (p1 && p2 && !p1->second.empty() && p1->second == p2->second) {
                    double sum = p1->first + p2->first;
                    // format int if whole
                    std::string coeff;
                    if (std::floor(sum) == sum) coeff = std::to_string((int)sum);
                    else coeff = std::to_string(sum);
                    return Value(std::string("(" + coeff + " * " + p1->second + ")"));
                }
            } else {
                // lhs is present, but we need to ensure it is of form a and var
                // Try to decompose simplified string as a sum
                std::string sstr = simplified->toString();
                // if simplified is (a + b) attempt to extract parts
                size_t plus = sstr.find('+');
                if (plus != std::string::npos) {
                    // try to parse left and right parts
                    size_t pos = sstr.find('+');
                    std::string leftpart = sstr.substr(0,pos);
                    std::string rightpart = sstr.substr(pos+1);
                    auto p1 = try_parse_coeff_var(leftpart);
                    auto p2 = try_parse_coeff_var(rightpart);
                    if (p1 && p2 && !p1->second.empty() && p1->second == p2->second) {
                        double sum = p1->first + p2->first;
                        std::string coeff = (std::floor(sum) == sum) ? std::to_string((int)sum) : std::to_string(sum);
                        return Value(std::string("(" + coeff + " * " + p1->second + ")"));
                    }
                }
            }

            return Value(simplified->toString());
        } catch (...) {
            // parsing failed for one or both strings; fall back to normal concatenation
        }
    }

Value Interpreter::eval_LiteralExpr(const LiteralExpr* node) {
    if (node->type == Value::Type::Int) {
        // Check if it contains scientific notation (e or E) or decimal point
        const std::string value = node->value;
        if (value.find('.') != std::string::npos ||
            value.find('e') != std::string::npos ||
            value.find('E') != std::string::npos) {
            // Parse as double for floating point numbers and scientific notation
            double d = std::stod(node->value);
            return Value(d);
        }
        // 先尝试用 int 解析，只有溢出时才用 BigInt
        try {
            int i = std::stoi(node->value);
            return Value(i);
        } catch (const std::out_of_range&) {
            // int 溢出，使用 BigInt
            ::BigInt big(node->value);
            return Value(big);
        }
    }
    // Check for boolean literals
    if (node->value == "true") return Value(true);
    if (node->value == "false") return Value(false);
    if (node->value == "null") return Value(nullptr);
    // Otherwise it's a string
    return Value(node->value);
}

Value Interpreter::eval_CallExpr(const CallExpr* call) {
    // Prepare parameter
    std::vector<Value> args{};

    // Calculate parameter
    for (const auto& arg: call->args) {
        if (!arg) {}
        args.push_back(eval(arg.get()));
    }

    const auto left = eval(call->callee.get());
    if (!left.is_lambda() and !left.is_lmCppFunction()) {
        std::cerr << "Left type '" << left.to_string() << "' is not a callable object " << std::endl;
        return LAMINA_NULL;
    }

    if (std::holds_alternative<std::shared_ptr<LambdaDeclExpr>>(left.data)) {
        // get function
        std::shared_ptr<LambdaDeclExpr> func;
        func = std::get<std::shared_ptr<LambdaDeclExpr>>(left.data);

        // get arguments
        if (call->args.size() != func->params.size()) {
            std::cerr << "function " << func->name << " required " <<
                func->params.size() << " , got "  << call->args.size() << std::endl;
            return {};
        }
        // User function
        return Interpreter::call_function(func.get(), args);
    }

    if (std::holds_alternative<std::shared_ptr<LmCppFunction>>(left.data)) {
        push_frame("<cpp function>", " ");

        Value result;
        std::shared_ptr<LmCppFunction> func;
        func = std::get<std::shared_ptr<LmCppFunction>>(left.data);
        try { result = func->function(args); }
        catch (...) {
            pop_frame();
            throw;
        }
        pop_frame();
        return result;
    }

    std::cerr << "Type is not a callable object " << std::endl;
    return {};
}

Value Interpreter::call_function(const LambdaDeclExpr* func, const std::vector<Value>& args) {
    if (func == nullptr) {
        std::cerr << "Error: Function at '" << func << "' is null" << std::endl;
        return Value("<func error>");
    }

    Interpreter::push_frame(func->name, "<script>", 0);   // Add to call stack

    Interpreter::push_scope();// Create scope here
    // Pass arguments
    for (size_t j = 0; j < func->params.size(); ++j) {
        set_variable(func->params[j], args[j]);
    }
    // Execute function body, capture return
    try {
        for (const auto& stmt: func->body->statements) {
            try {
                Interpreter::execute(stmt);
            } catch (const ReturnException& re) {
                // Normal return handling
                Interpreter::pop_frame();
                Interpreter::pop_scope();
                return re.value;
            } catch (const RuntimeError& re) {
                // If the error doesn't have a stack trace yet, capture current state
                RuntimeError enriched(re.message);
                if (re.stack_trace.empty()) {
                    enriched.stack_trace = get_stack_trace();
                } else {
                    enriched.stack_trace = re.stack_trace;  // Preserve existing trace
                }
                Interpreter::pop_frame();
                Interpreter::pop_scope();
                throw enriched;
            } catch (const std::exception& e) {
                // Wrap standard exception as RuntimeError
                RuntimeError enriched("In function '" + func->name + "': " + std::string(e.what()));
                enriched.stack_trace = get_stack_trace();   // Get stack trace before cleanup
                Interpreter::pop_frame();
                Interpreter::pop_scope();
                throw enriched;
            }
        }
    } catch (const ReturnException& re) {
        // Ensure cleanup in any case
        Interpreter::pop_frame();
        Interpreter::pop_scope();
        return re.value;
    }
    Interpreter::pop_frame();
    Interpreter::pop_scope();
    return Value(); // Default value when no return
}

Value Interpreter::eval_BinaryExpr(const BinaryExpr* bin) {
    Value l = eval(bin->left.get());
    Value r = eval(bin->right.get());

    // Handle arithmetic operations
    // Just handle them in the f**king different functions
    if (bin->op == "+") {
<<<<<<< HEAD
        return HANDLE_BINARYEXPR_ADD(&l, &r);
=======
		if (l.is_infinity() || r.is_infinity()) {
			return l;
		}
        // String concatenation
        if (l.is_string() || r.is_string()) {
            return Value(l.to_string() + r.to_string());
        }
        // Vector addition
        if (l.is_array() && r.is_array()) {
            return l.vector_add(r);
        }
        // 只要有一方是 Irrational 或 Symbolic，优先生成符号表达式
		// Debug output:
		//std::cerr << "Adding: l type " << int(l.type) << "; r type " << int(r.type) << std::endl;
        if ((l.is_irrational() || l.is_symbolic() || r.is_irrational() || r.is_symbolic()) && l.is_numeric() && r.is_numeric()) {
            std::shared_ptr<SymbolicExpr> leftExpr;
            std::shared_ptr<SymbolicExpr> rightExpr;
            if (l.is_symbolic()) {
                leftExpr = std::get<std::shared_ptr<SymbolicExpr>>(l.data);
            } else if (l.is_irrational()) {
                leftExpr = std::get<::Irrational>(l.data).to_symbolic();
            } else if (l.is_rational()) {
                leftExpr = SymbolicExpr::number(std::get<::Rational>(l.data));
            } else if (l.is_bigint()) {
                leftExpr = SymbolicExpr::number(std::get<::BigInt>(l.data));
            } else if (l.is_int()) {
                leftExpr = SymbolicExpr::number(std::get<int>(l.data));
            } else if (l.is_float()) {
                leftExpr = SymbolicExpr::number(::Rational::from_double(std::get<double>(l.data)));
            } else {
                leftExpr = SymbolicExpr::number(0);
            }
            if (r.is_symbolic()) {
                rightExpr = std::get<std::shared_ptr<SymbolicExpr>>(r.data);
            } else if (r.is_irrational()) {
                rightExpr = std::get<::Irrational>(r.data).to_symbolic();
            } else if (r.is_rational()) {
                rightExpr = SymbolicExpr::number(std::get<::Rational>(r.data));
            } else if (r.is_bigint()) {
                rightExpr = SymbolicExpr::number(std::get<::BigInt>(r.data));
            } else if (r.is_int()) {
                rightExpr = SymbolicExpr::number(std::get<int>(r.data));
            } else if (r.is_float()) {
                rightExpr = SymbolicExpr::number(::Rational::from_double(std::get<double>(r.data)));
            } else {
                rightExpr = SymbolicExpr::number(0);
            }
            return Value(SymbolicExpr::add(leftExpr, rightExpr)->simplify());
        }
        // Numeric addition with irrational and rational number support
        if (l.is_numeric() && r.is_numeric()) {
            // BigInt 优先：如果任一为 BigInt，结果为 BigInt
            if (l.is_bigint() || r.is_bigint()) {
                ::BigInt lb = l.is_bigint() ? std::get<::BigInt>(l.data) : ::BigInt(l.as_number());
                ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());
                return Value(lb + rb);
            }
            // If either operand is rational, use rational arithmetic
            if (l.is_rational() || r.is_rational()) {
                ::Rational result = l.as_rational() + r.as_rational();
                return Value(result);
            }

            double result = l.as_number() + r.as_number();// Return int if both operands are int and result is the whole
            if (l.is_int() && r.is_int()) {
                // check overflow and underflow
                if (static_cast<int>(result) == INT_MAX ||
                    static_cast<int>(result) == INT_MIN) {
                    return Value(BigInt(l.as_number()) + BigInt(r.as_number()));
                }
                return Value(static_cast<int>(result));
            }
            return Value(result);
        } else {
            error_and_exit("Cannot add " + l.to_string() + " and " + r.to_string());
        }
>>>>>>> a5d98f47f1d3eba474d6d9de372f35b311daa1f3
    }
    // Arithmetic operations (require numeric operands or vector operations)
    if (bin->op == "-" || bin->op == "*" || bin->op == "/" ||
        bin->op == "%" || bin->op == "^") {
		
		if (l.is_infinity() || r.is_infinity()) {
			error_and_exit("Error: Infinity cannot participate in evaluations");
		}
			
        // Special handling for multiplication
        if (bin->op == "*") {
            // Vector and matrix operations
            if (l.is_array() && r.is_array()) {
                // Try dot product for same-size vectors
                const auto& la = std::get<std::vector<Value>>(l.data);
                const auto& ra = std::get<std::vector<Value>>(r.data);
                if (la.size() == ra.size()) {
                    return l.dot_product(r);
                }
            }
            // Matrix multiplication
            if (l.is_matrix() && r.is_matrix()) {
                return l.matrix_multiply(r);
            }
            // Scalar multiplication for vectors
            if (l.is_array() && r.is_numeric()) {
                return l.scalar_multiply(r.as_number());
            }
            if (l.is_numeric() && r.is_array()) {
                return r.scalar_multiply(l.as_number());
            }
            // 只要有一方是 Irrational 或 Symbolic，优先生成符号表达式
            if ((l.is_irrational() || r.is_irrational() || l.is_symbolic() || r.is_symbolic()) && l.is_numeric() && r.is_numeric()) {
                std::shared_ptr<SymbolicExpr> leftExpr;
                std::shared_ptr<SymbolicExpr> rightExpr;
                // 强制所有 Irrational 都转为 SymbolicExpr
                if (l.is_symbolic()) {
                    leftExpr = std::get<std::shared_ptr<SymbolicExpr>>(l.data);
                } else {
                    leftExpr = from_number_to_symbolic(l);
                }
                if (r.is_symbolic()) {
                    rightExpr = std::get<std::shared_ptr<SymbolicExpr>>(r.data);
                } else {
                    rightExpr = from_number_to_symbolic(r);
                }
                return Value(SymbolicExpr::multiply(leftExpr, rightExpr)->simplify());
            }
            // Regular multiplication (both must be numeric)
            if (l.is_numeric() && r.is_numeric()) {
                // BigInt 优先：如果任一为 BigInt，结果为 BigInt
                if (l.is_bigint() || r.is_bigint()) {
                    ::BigInt lb = l.is_bigint() ? std::get<::BigInt>(l.data) : ::BigInt(l.as_number());
                    ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());
                    return Value(lb * rb);
                }
                // If either operand is irrational, use irrational arithmetic
                if (l.is_irrational() || r.is_irrational()) {
                    ::Irrational result = l.as_irrational() * r.as_irrational();
                    return Value(result);
                }
                // If either operand is rational, use rational arithmetic
                if (l.is_rational() || r.is_rational()) {
                    ::Rational result = l.as_rational() * r.as_rational();
                    return Value(result);
                }

                double result = l.as_number() * r.as_number();
                if (l.is_int() && r.is_int()) {
                    // check overflow and underflow
                    if (static_cast<int>(result) == INT_MAX ||
                        static_cast<int>(result) == INT_MIN) {
                        return Value(BigInt(l.as_number()) * BigInt(r.as_number()));
                    }
                    return Value(static_cast<int>(result));
                }
                return Value(result);
            }
            // Error case
            L_ERR("Cannot multiply " + l.to_string() + " and " + r.to_string());
        }

        // Special handling for minus
        if (bin->op == "-") {
            // Vector and matrix operations
            if (l.is_array() && r.is_array()) {
                // Try minus for same-size vectors
                const auto& la = std::get<std::vector<Value>>(l.data);
                const auto& ra = std::get<std::vector<Value>>(r.data);
                return l.vector_minus(r);// An exception can be raised inside
            }
            // Matrix multiplication
            if (l.is_matrix() && r.is_matrix()) {
                L_ERR("Arithmetic operation '-' requires numeric or vector operands");
            }
            // Scalar multiplication for vectors
            if ((l.is_array() && r.is_numeric()) || (l.is_numeric() && r.is_array())) {
                L_ERR("Arithmetic operation '-' requires same-type operands");
            }
            // 只要有一方是 Irrational 或 Symbolic，优先生成符号表达式
            if ((l.is_irrational() || r.is_irrational() || l.is_symbolic() || r.is_symbolic()) && l.is_numeric() && r.is_numeric()) {
                std::shared_ptr<SymbolicExpr> leftExpr;
                std::shared_ptr<SymbolicExpr> rightExpr;
                // 强制所有 Irrational 都转为 SymbolicExpr
                if (l.is_symbolic()) {
                    leftExpr = std::get<std::shared_ptr<SymbolicExpr>>(l.data);
                } else {
                    leftExpr = from_number_to_symbolic(l);
                }
                if (r.is_symbolic()) {
                    rightExpr = std::get<std::shared_ptr<SymbolicExpr>>(r.data);
                } else {
                    rightExpr = from_number_to_symbolic(r);
                }
                return Value(SymbolicExpr::add(leftExpr, SymbolicExpr::multiply(SymbolicExpr::number(-1), rightExpr)->simplify())->simplify());
            }
            // Regular multiplication (both must be numeric)
            if (l.is_numeric() && r.is_numeric()) {
                // BigInt 优先：如果任一为 BigInt，结果为 BigInt
                if (l.is_bigint() || r.is_bigint()) {
                    ::BigInt lb = l.is_bigint() ? std::get<::BigInt>(l.data) : ::BigInt(l.as_number());
                    ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());
                    return Value(lb - rb);
                }
                // If either operand is irrational, use irrational arithmetic
                if (l.is_irrational() || r.is_irrational()) {
                    ::Irrational result = l.as_irrational() - r.as_irrational();
                    return Value(result);
                }
                // If either operand is rational, use rational arithmetic
                if (l.is_rational() || r.is_rational()) {
                    ::Rational result = l.as_rational() - r.as_rational();
                    return Value(result);
                }

                double result = l.as_number() - r.as_number();
                if (l.is_int() && r.is_int()) {
                    // check overflow and underflow
                    if (static_cast<int>(result) == INT_MAX ||
                        static_cast<int>(result) == INT_MIN) {
                        return Value(BigInt(l.as_number()) - BigInt(r.as_number()));
                    }
                    return Value(static_cast<int>(result));
                }
                return (l.is_int() && r.is_int()) ? Value(static_cast<int>(result)) : Value(result);
            }
            // Error case
            L_ERR("Cannot decrease " + l.to_string() + " by " + r.to_string());
        }

        // Other arithmetic operations require both operands to be numeric
        if (!l.is_numeric() || !r.is_numeric()) {
            L_ERR("Arithmetic operation '" + bin->op + "' requires numeric operands");
        }

        // For division, always use rational arithmetic for precise results
        if (bin->op == "/") {
            // 只要有一方是 Irrational 或 Symbolic，优先生成符号表达式
            if ((l.is_irrational() || l.is_symbolic() || r.is_irrational() || r.is_symbolic()) && l.is_numeric() && r.is_numeric()) {
                std::shared_ptr<SymbolicExpr> leftExpr;
                std::shared_ptr<SymbolicExpr> rightExpr;
                if (l.is_symbolic()) {
                    leftExpr = std::get<std::shared_ptr<SymbolicExpr>>(l.data);
                } else {
                    leftExpr = from_number_to_symbolic(l);
                }
                if (r.is_symbolic()) {
                    rightExpr = std::get<std::shared_ptr<SymbolicExpr>>(r.data);
                } else {
                    rightExpr = from_number_to_symbolic(r);
                }
                auto divExpr = SymbolicExpr::multiply(leftExpr, SymbolicExpr::power(rightExpr, SymbolicExpr::number(-1)));
                return Value(divExpr->simplify());
            }
            // BigInt 优先：如果任一为 BigInt，结果为 BigInt（如果整除）或 Rational
            if (l.is_bigint() || r.is_bigint()) {
                ::BigInt lb = l.is_bigint() ? std::get<::BigInt>(l.data) : ::BigInt(l.as_number());
                ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());
                if (rb.is_zero()) {
                    L_ERR("Division by zero");
                }
                // 对于BigInt除法，如果能整除则返回BigInt，否则返回Rational
                try {
                    ::BigInt quotient = lb / rb;
                    ::BigInt remainder = lb - (quotient * rb);
                    if (remainder.is_zero()) {
                        return Value(quotient);
                    } else {
                        // 不能整除，返回有理数
                        return Value(::Rational(lb, rb));
                    }
                } catch (...) {
                    // 如果BigInt运算失败，回退到Rational
                    return Value(::Rational(lb, rb));
                }
            }
            // If either operand is irrational, use irrational arithmetic
			// TODO:这部分代码存在问题
            if (l.is_irrational() || r.is_irrational()) {
                ::Irrational lr = l.as_irrational();
                ::Irrational rr = r.as_irrational();
                if (rr.is_zero()) {
                    L_ERR("Division by zero");
                }
                return Value(lr / rr);
            }

            ::Rational lr = l.as_rational();
            ::Rational rr = r.as_rational();
            if (rr.is_zero()) {
                L_ERR("Division by zero");
            }
            return Value(lr / rr);
        }

        if (bin->op == "%") {
            if ((l.is_irrational() || l.is_symbolic() || r.is_irrational() || r.is_symbolic() || l.is_rational() || r.is_rational()) && l.is_numeric() && r.is_numeric()) {
                // 有小数，使用小数取模
                double ld = l.as_number();
                double rd = r.as_number();
                if (rd == 0.0) {
                    L_ERR("Modulo by zero");
                }
                return Value(ld - rd * std::floor(ld / rd));    // 保持结果非负
            }
            if (l.is_bigint() || r.is_bigint()) {
                // 有BigInt，使用BigInt内置方法
                ::BigInt lb = l.is_bigint() ? std::get<::BigInt>(l.data) : ::BigInt(l.as_number());
                ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());
                return Value(lb % rb);
            }
            // 都为int
            return Value(static_cast<int>(l.as_number()) % static_cast<int>(r.as_number()));
        }

        if (bin->op == "^") {
            if ((l.is_irrational() || l.is_symbolic() || r.is_irrational() || r.is_symbolic()) && l.is_numeric() && r.is_numeric()) {
                // 只要有一方是 Irrational 或 Symbolic，优先生成符号表达式
                std::shared_ptr<SymbolicExpr> leftExpr;
                std::shared_ptr<SymbolicExpr> rightExpr;
                if (l.is_symbolic()) {
                    leftExpr = std::get<std::shared_ptr<SymbolicExpr>>(l.data);
                } else {
                    leftExpr = from_number_to_symbolic(l);
                }
                if (r.is_symbolic()) {
                    rightExpr = std::get<std::shared_ptr<SymbolicExpr>>(r.data);
                } else {
                    rightExpr = from_number_to_symbolic(r);
                }
                auto exponentExpr = SymbolicExpr::power(leftExpr, rightExpr);
                return Value(exponentExpr->simplify());
            }
            if (l.is_rational() && (r.is_bigint() || r.is_int())) {
                // 如果底数为Rational，指数为整数，结果为Rational
                ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());
                return Value(l.as_rational().power(rb));
            }
            if ((l.is_bigint() || r.is_int()) && (r.is_bigint() || r.is_int())) {
                // 如果底数为整数，指数为整数，结果为BigInt
                ::BigInt lb = l.is_bigint() ? std::get<::BigInt>(l.data) : ::BigInt(l.as_number());
                ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());
				if (rb < ::BigInt(0)) {
					return Value(l.as_rational().reciprocal().power(::BigInt(0) - rb));
				} else {
					return Value(lb.power(rb));
				}
            }
			if ((l.is_int() || l.is_bigint() || l.is_rational()) && r.is_rational()) {
				// 底数和指数均为Rational，考虑符号表达式
				::Rational lb;
				if (l.is_int()) lb = ::Rational(l.as_number());
				else if (l.is_bigint()) lb = ::Rational(std::get<::BigInt>(l.data));
				else lb = std::get<::Rational>(l.data);
				return Value(SymbolicExpr::power(SymbolicExpr::number(lb), SymbolicExpr::number(std::get<::Rational>(r.data)))->simplify());
			}
            // 有小数，采用小数幂
            double ld = l.as_number();
            double rd = r.as_number();

            return Value(std::pow(ld, rd));
        }

        L_ERR("Unknown Arithmetic Operation.");
    }

    // Comparison operators
    if (bin->op == "==" || bin->op == "!=" || bin->op == "<" ||
        bin->op == "<=" || bin->op == ">" || bin->op == ">=") {
        // Handle different type combinations
		if (l.is_infinity() && r.is_infinity()) {
			int lt = std::get<int>(l.data), rt = std::get<int>(r.data);
			if (bin->op == "==") return lt == rt;
			else if (bin->op == "!=") return lt != rt;
			else if (bin->op == "<") return lt < rt;
			else if (bin->op == "<=") return lt <= rt;
			else if (bin->op == ">") return lt > rt;
			else if (bin->op == ">=") return lt >= rt;
			else return false;
		}
		if (l.is_infinity()) {
			if (bin->op == "==") return false;
			if (bin->op == "!=") return true;
			if (bin->op == ">" || bin->op == ">=") return (std::get<int>(l.data) > 0);
			else return !(std::get<int>(l.data) > 0);
		}
		if (r.is_infinity()) {
			if (bin->op == "==") return false;
			if (bin->op == "!=") return true;
			if (bin->op == "<" || bin->op == "<=") return (std::get<int>(r.data) > 0);
			else return !(std::get<int>(r.data) > 0);
		}
        if (l.is_numeric() && r.is_numeric()) {
            // BigInt 比较优先
            if (l.is_bigint() || r.is_bigint()) {
                ::BigInt lb = l.is_bigint() ? std::get<::BigInt>(l.data) : ::BigInt(l.as_number());
                ::BigInt rb = r.is_bigint() ? std::get<::BigInt>(r.data) : ::BigInt(r.as_number());

                // 使用字符串比较来判断大小（这是一个简化的实现）
                std::string ls = lb.to_string();
                std::string rs = rb.to_string();

                if (bin->op == "==") return Value(ls == rs);
                if (bin->op == "!=") return Value(ls != rs);

                // 对于大小比较，需要考虑符号和长度
                bool lb_neg = ls[0] == '-';
                bool rb_neg = rs[0] == '-';

                if (lb_neg && !rb_neg) {
                    // 左负右正
                    if (bin->op == "<") return Value(true);
                    if (bin->op == "<=") return Value(true);
                    if (bin->op == ">") return Value(false);
                    if (bin->op == ">=") return Value(false);
                } else if (!lb_neg && rb_neg) {
                    // 左正右负
                    if (bin->op == "<") return Value(false);
                    if (bin->op == "<=") return Value(false);
                    if (bin->op == ">") return Value(true);
                    if (bin->op == ">=") return Value(true);
                } else {
                    // 同号比较：比较绝对值的长度和字典序
                    std::string labs = lb_neg ? ls.substr(1) : ls;
                    std::string rabs = rb_neg ? rs.substr(1) : rs;

                    bool abs_less;
                    if (labs.length() != rabs.length()) {
                        abs_less = labs.length() < rabs.length();
                    } else {
                        abs_less = labs < rabs;
                    }

                    bool result_less = lb_neg ? !abs_less : abs_less;

                    if (bin->op == "<") return Value(result_less);
                    if (bin->op == "<=") return Value(result_less || ls == rs);
                    if (bin->op == ">") return Value(!result_less && ls != rs);
                    if (bin->op == ">=") return Value(!result_less);
                }
            } else {
                double ld = l.as_number();
                double rd = r.as_number();

                if (bin->op == "==") return Value(ld == rd);
                if (bin->op == "!=") return Value(ld != rd);
                if (bin->op == "<") return Value(ld < rd);
                if (bin->op == "<=") return Value(ld <= rd);
                if (bin->op == ">") return Value(ld > rd);
                if (bin->op == ">=") return Value(ld >= rd);
            }
        } else if (l.is_string() && r.is_string()) {
            std::string ls = std::get<std::string>(l.data);
            std::string rs = std::get<std::string>(r.data);

            if (bin->op == "==") return Value(ls == rs);
            if (bin->op == "!=") return Value(ls != rs);
            if (bin->op == "<") return Value(ls < rs);
            if (bin->op == "<=") return Value(ls <= rs);
            if (bin->op == ">") return Value(ls > rs);
            if (bin->op == ">=") return Value(ls >= rs);
        } else if (l.is_bool() && r.is_bool()) {
            bool lb = std::get<bool>(l.data);
            bool rb = std::get<bool>(r.data);

            if (bin->op == "==") return Value(lb == rb);
            if (bin->op == "!=") return Value(lb != rb);
            // For booleans, false < true
            if (bin->op == "<") return Value(lb < rb);
            if (bin->op == "<=") return Value(lb <= rb);
            if (bin->op == ">") return Value(lb > rb);
            if (bin->op == ">=") return Value(lb >= rb);
        } else {
            // Type mismatch - only equality/inequality make sense
            if (bin->op == "==") return Value(false);   // Different types are never equal
            if (bin->op == "!=") return Value(true);    // Different types are always not equal

            L_ERR("Cannot compare different types with operator '" + bin->op + "'");
            return Value();
        }
    }

    L_ERR("Unknown binary operator '" + bin->op + "'");
    return {};
}

Value Interpreter::eval_UnaryExpr(const UnaryExpr* unary) {
    Value v = eval(unary->operand.get());

    if (unary->op == "-") {
		if (v.is_infinity()) {
			v.data = Value::DataType(std::in_place_index<2>, 0-(std::get<int>(v.data)));
			return v;
		}
        if (v.type != Value::Type::Int && v.type != Value::Type::BigInt && v.type != Value::Type::Float) {
            RuntimeError error("Unary operator '-' requires integer, float or big integer operand");
            error.stack_trace = get_stack_trace();
            throw error;
        }
        if (v.type == Value::Type::Int) {
            int vi = std::get<int>(v.data);
            return Value(-vi);
        }
        if (v.type == Value::Type::Float) {
            float vf = std::get<double>(v.data);
            return Value(-vf);
        }
        // For BigInt, negate directly
        ::BigInt big_val = std::get<::BigInt>(v.data);
        return Value(big_val.negate());
    }

    if (unary->op == "!") {
        if (v.type != Value::Type::Int && v.type != Value::Type::BigInt) {
            RuntimeError error("Unary operator '!' requires integer or big integer operand");
            error.stack_trace = get_stack_trace();
            throw error;
        }
        int vi;
        if (v.type == Value::Type::Int) {
            vi = std::get<int>(v.data);
        } else {
            vi = std::get<::BigInt>(v.data).to_int();
        }

        if (vi < 0) {
            RuntimeError error("Cannot calculate factorial of negative number");
            error.stack_trace = get_stack_trace();
            throw error;
        }

        // Use BigInt for factorial if the number is large or result would be large
        if (vi > 20) {
            ::BigInt result(1);
            for (int j = 2; j <= vi; ++j) {
                result = result * ::BigInt(j);
            }
            return Value(result);
        }
        // Use regular int for small factorials
        int res = 1;
        for (int j = 1; j <= vi; ++j) res *= j;
        return Value(res);
    }

    std::cerr << "Error: Unknown unary operator '" << unary->op << "'" << std::endl;
    return Value("<unknown op>");
}
