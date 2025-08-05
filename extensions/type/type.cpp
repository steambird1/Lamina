#include "type.hpp"

namespace lamina::type {
    State state_instance;
    // 开启检查
    Value enable(const std::vector<Value> &args) {
        if (!args[0].is_int()) {
            L_ERR("mode declare must be int!");
        }
        state_instance.mode = 0;

        int mode = args[0].as_number();
        if (mode == 1) {
            state_instance.mode = 1;
        } else {
            state_instance.mode = 0;
        }
        return Value(nullptr);
    }

    // 判断是否允许使用Maybe
    bool is_allow_maybe() {
        if (state_instance.mode == 1) {
            return true;
        } else {
            return false;
        }
    }

}

namespace lamina::maybe {

    Value type(const std::vector<Value>& args) {
        // 第一个参数为类型，第二个参数为值
        // 如果匹配类型，下面允许调用强转函数
        // 不匹配禁止调用
        if (!type::is_allow_maybe()) {
            L_ERR("maybe is not allowed!");
        }

        if (!args[0].is_string()) {
            L_ERR("First args must be string!");
        }

        std::string str = args[0].to_string();
        bool ret = check_type_same(str, args);
        if (ret) {
            return Value(args[1]);
        } else {
            L_ERR("Type not match! Cannot Force Convert Type!");
            return Value(); // 永远不会执行，但消除编译器警告
        }
    }

    bool check_type_same(std::string type, const std::vector<Value> &args) {
        type_enum Type;

        if (type == "int") {
            Type = TYPE_INTEGER;
        } else if (type == "float") {
            Type = TYPE_FLOAT;
        } else if (type == "string") {
            Type = TYPE_STRING;
        } else if (type == "boolean") {
            Type = TYPE_BOOLEAN;
        } else if (type == "matrix") {
            Type = TYPE_MATRIX;
        } else if (type == "array") {
            Type = TYPE_ARRAY;
        } else if (type == "bigint") {
            Type = TYPE_BIGINT;
        } else if (type == "irrational") {
            Type = TYPE_IRRATIONAL;
        } else if (type == "rational") {
            Type = TYPE_RATIONAL;
        } else {
            Type = UNKNOWN;
        }

        switch (Type) {
            case TYPE_INTEGER:
                if (args[1].is_int()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_FLOAT:
                if (args[1].is_float()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_STRING:
                if (args[1].is_string()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_BOOLEAN:
                if (args[1].is_bool()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_MATRIX:
                if (args[1].is_matrix()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_ARRAY:
                if (args[1].is_array()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_BIGINT:
                if (args[1].is_bigint()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_IRRATIONAL:
                if (args[1].is_irrational()) {
                    return true;
                } else {
                    return false;
                }
            case TYPE_RATIONAL:
                if (args[1].is_rational()) {
                    return true;
                } else {
                    return false;
                }
            case UNKNOWN:
                L_ERR("Unknown Type!");
            default:
                return false;
        }
    }
}

namespace lamina::force_convert {
    Value to_int(const std::vector<Value> &args) {
        // 检查参数是否可以强转
        if (args[0].is_string()) {
            L_ERR("String Cannot Convert to Int!");
        } else if (args[0].is_int()) {
            L_ERR("Int Cannot Convert To Int!");
        } else if (args[0].is_irrational()) {
            /** 这么做是为了保留精度 **/
            L_ERR("Irrational Cannot Convert To Int!");
        } else if (args[0].is_rational()) {
            return Value(args[0].as_rational());
        } else if (args[0].is_bigint()) {
            L_ERR("Big Int Cannot Convert Int"); // 确保大整数的精度
        } else if(args[0].is_null()) {
            L_ERR("Null Cannot Convert Int");
        } else if (args[0].is_bool()) {
            if (args[0].as_number() == 0 || args[0].as_number() == 1) {
                return Value(args[0].as_number());
            } else {
                L_ERR("Invalid Bool Range!");
            }
        } else if (args[0].is_matrix()) {
            L_ERR("Matrix Cannot Convert to Int!");
        } else if (args[0].is_array()){
            L_ERR("Array Cannot Convert to Int!");
        } else {
            // 可以则转换
            return Value(std::stoi(args[0].to_string()));
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_float(const std::vector<Value> &args) {
        // 检查参数是否可以强转
        if (args[0].is_string()) {
            L_ERR("String Cannot Convert to Float!");
        } else if (args[0].is_float()) {
            L_ERR("Float Cannot Convert to Float!");
        } else if (args[0].is_bool()){
            L_ERR("Bool Cannot Convert to Float!");
        } else if (args[0].is_null()){
            L_ERR("Null Cannot Convert to Float!");
        } else if (args[0].is_bigint()) {
            L_ERR("is_bigint Cannot Convert to Float!");
        } else if (args[0].is_irrational()){ // 特殊处理
            return Value(args[0].as_number());
        } else if (args[0].is_rational()) {
            return Value(args[0].as_number());
        } else if (args[0].is_matrix()) {
            L_ERR("Matrix Cannot Convert to Float!");
        } else if (args[0].is_array()){
            L_ERR("Array Cannot Convert to Float!");
        } else {
            // 可以则转换
            return Value(std::stof(args[0].to_string()));
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_string(const std::vector<Value> &args) {
        // 检查参数是否可以强转
        if (args[0].is_string()) {
            L_ERR("String Cannot Convert to String!");
        } else {
            // 可以则转换
            return Value(args[0].to_string().c_str());
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_bigint(const std::vector<Value> &args) {
        if (args[0].is_bigint()) {
            L_ERR("Big Int Cannot Convert to Big Int!");
        } else if (args[0].is_irrational()){
            return Value(args[0].as_number());
        } else if (args[0].is_rational()) {
            return Value(args[0].as_number());
        } else if (args[0].is_string()) {
            L_ERR("String Cannot Convert to Big Int!");
        } else if (args[0].is_int()) {
            L_ERR("Int Cannot Convert to Big Int!");
        } else if (args[0].is_float()){
            return Value(args[0].as_number());
        } else if (args[0].is_bool()) {
            return Value(args[0].as_number());
        } else if (args[0].is_matrix()) {
            L_ERR("Matrix Cannot Convert to Int!");
        } else if (args[0].is_array()) {
            L_ERR("Array Cannot Convert to Int!");
        } else if (args[0].is_null()) {
            L_ERR("Null Cannot Convert to Int!");
        } else {
            return Value(args[0].as_number());
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_boolean(const std::vector<Value> &args) {
        if (args[0].is_bool()) {
            L_ERR("Bool Cannot Convert to Bool!");
        } else if (args[0].is_int()) {
            if (args[0].as_number() == 0 || args[0].as_number() == 1) {
                return Value(args[0].as_bool());
            } else {
                L_ERR("Invalid Boolean Range!");
            }
        } else {
            L_ERR("Boolean Convert Only Support Int!");
            return Value(); // 永远不会执行，但消除编译器警告
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_array(const std::vector<Value> &args) {
        if (args[0].is_array()) {
            L_ERR("Array Cannot Convert to Array!");
        } else if (args[0].is_matrix()) {
            // 手动转换成数组
            // 字符串如果是[[]]这种嵌套形式就转换成[]
            if (args[0].to_string().front() == '[' && args[0].to_string().back() == ']') {
                return Value(args[0].to_string().substr(1, args[0].to_string().size() - 2));
            }
        } else {
            L_ERR("Only Support Matrix To Array!");
            return Value(); // 永远不会执行，但消除编译器警告
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_matrix(const std::vector<Value> &args) {
        if (args[0].is_matrix()) {
            L_ERR("Matrix Cannot Convert to Matrix!");
        } else if (args[0].is_array()) {
            // 手动转换成数组
            // 字符串如果是[]这种嵌套形式就转换成[[]]
            if (args[0].to_string().front() == '[' && args[0].to_string().back() == ']') {
                return Value("[" + args[0].to_string() + "]");
            }
        } else {
            L_ERR("Only Support Array To Matrix!");
            return Value(); // 永远不会执行，但消除编译器警告
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_irrational(const std::vector<Value> &args) {
        if (args[0].is_float()) {
            return Value(args[0].as_irrational());
        } else {
            L_ERR("Float Cannot Convert to Irrational!");
            return Value(); // 永远不会执行，但消除编译器警告
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

    Value to_rational(const std::vector<Value> &args) {
        if (args[0].is_float()) {
            return Value(args[0].as_rational());
        } else {
            L_ERR("Float Cannot Convert to Rational!");
            return Value(); // 永远不会执行，但消除编译器警告
        }
        return Value(); // 永远不会执行，但消除编译器警告
    }

}