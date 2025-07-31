/* 
     CopyRight Shizuku Technologies
     2025.07.30 09:19
     @Dev Ange1PlsGreet
*/
#include "json.hpp"

Value json_decode(const std::vector<Value> &args) {
     if (!args[0].is_string()) {
          L_ERR("Json Data Must Be A String");
     }

     auto data = parse_json(args[0].to_string());
     return data;
}
