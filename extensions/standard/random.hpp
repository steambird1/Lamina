#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "lamina.hpp"
#include <random>

Value randint(const std::vector<Value>& args);
Value randstr(const std::vector<Value>& args);

namespace lamina {
    LAMINA_FUNC("randint", randint, 2);
    LAMINA_FUNC("randstr", randstr, 1);
}





#endif //RANDOM_HPP
