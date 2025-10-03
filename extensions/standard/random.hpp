#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "../../interpreter/lamina_api/lamina.hpp"
#include <vector>

Value rand(const std::vector<Value>& args);
Value randint(const std::vector<Value>& args);
Value randstr(const std::vector<Value>& args);

namespace lamina {
    LAMINA_FUNC("rand", rand, 0);
    LAMINA_FUNC("randint", randint, 2);
    LAMINA_FUNC("randstr", randstr, 1);
}// namespace lamina


#endif// RANDOM_HPP
