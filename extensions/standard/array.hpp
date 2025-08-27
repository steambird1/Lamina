#ifndef ARRAY_HPP
#define ARRAY_HPP
#include "lamina.hpp"

Value range(const std::vector<Value>& args);
Value visit_array_by_int(const std::vector<Value>& args);
Value visit_array_by_str(const std::vector<Value>& args);

namespace Lamina {
    LAMINA_FUNC_WIT_ANY_ARGS("range", range)
    LAMINA_FUNC_WIT_ANY_ARGS("visit", visit_array_by_int)
    LAMINA_FUNC_WIT_ANY_ARGS("visit_by_str", visit_array_by_str)
}// namespace Lamina
#endif//ARRAY_HPP
