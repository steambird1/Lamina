/* 
     CopyRight Shizuku Technologies
     2025.07.20 23:14
     @Dev Ange1PlsGreet
*/
#ifndef TIMES_HPP
#define TIMES_HPP
#include "lamina.hpp"

Value get_time(const std::vector<Value>& args);
Value get_date(const std::vector<Value>& args);
Value get_format_date(const std::vector<Value>& args);

namespace lamina {
    LAMINA_FUNC("get_time", get_time, 0);
    LAMINA_FUNC("get_date", get_date, 0);
    LAMINA_FUNC("format_date", get_format_date, 1);
}

#endif //TIMES_HPP
