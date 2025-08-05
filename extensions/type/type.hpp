#ifndef TYPE_HPP
#define TYPE_HPP
#include "lamina.hpp"

namespace lamina::type {
    struct State {
        int mode;
    };

    Value enable(const std::vector<Value>& args);
    bool is_allow_maybe();
    LAMINA_FUNC("declare", enable, 1);
};

namespace lamina::maybe {
    Value type(const std::vector<Value>& args);
    static bool check_type_same(std::string type, const std::vector<Value> &args);

    typedef enum {
        TYPE_INTEGER,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_BOOLEAN,
        TYPE_MATRIX,
        TYPE_ARRAY,
        TYPE_BIGINT,
        TYPE_IRRATIONAL,
        TYPE_RATIONAL,
        UNKNOWN
    } type_enum;
    LAMINA_FUNC("type", type, 2);
}

namespace lamina::force_convert {
    Value to_int(const std::vector<Value>& args);
    Value to_float(const std::vector<Value>& args);
    Value to_string(const std::vector<Value>& args);
    Value to_boolean(const std::vector<Value>& args);
    Value to_matrix(const std::vector<Value>& args);
    Value to_array(const std::vector<Value>& args);
    Value to_bigint(const std::vector<Value>& args);
    Value to_irrational(const std::vector<Value>& args);
    Value to_rational(const std::vector<Value>& args);

    LAMINA_FUNC("int", to_int, 1);
    LAMINA_FUNC("float", to_float, 1);
    LAMINA_FUNC("string", to_string, 1);
    LAMINA_FUNC("boolean", to_boolean, 1);
    LAMINA_FUNC("matrix", to_matrix, 1);
    LAMINA_FUNC("array", to_array, 1);
    LAMINA_FUNC("bigint", to_bigint, 1);
    LAMINA_FUNC("irrational", to_irrational, 1);
    LAMINA_FUNC("rational", to_rational, 1);
}


#endif //TYPE_HPP
