/* 
     CopyRight Shizuku Technologies
     2025.07.30 09:19
     @Dev Ange1PlsGreet
*/
#ifndef JSON_HPP
#define JSON_HPP
#include "lamina.hpp"
#include "json_parser.hpp"

Value json_decode(const std::vector<Value> &args);

namespace lamina {
     LAMINA_FUNC("json_decode", json_decode, 1)
}
#endif //JSON_HPP
