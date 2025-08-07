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


// Lamina 插件标准入口，参考 PLUGIN_GUIDE.md
#ifdef _WIN32
extern "C" __declspec(dllexport)
#else
extern "C"
#endif
void _entry(Interpreter& interpreter) {
    interpreter.builtin_functions["json_decode"] = json_decode;
}
