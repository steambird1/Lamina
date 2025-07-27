#include "times.hpp"
#include <chrono>
#include <sstream>

Value get_time(const std::vector<Value>& args) {
     auto now = std::chrono::system_clock::now();
     auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
     return LAMINA_INT(timestamp);
}

Value get_date(const std::vector<Value>& args) {
     auto now = std::chrono::system_clock::now();
     auto time = std::chrono::system_clock::to_time_t(now);
     std::tm* localTime = std::localtime(&time);

     std::ostringstream oss;
     oss << std::put_time(localTime, "%Y-%m-%d");
     std::string dateStr = oss.str();

     return Value(dateStr);
}

Value get_format_date(const std::vector<Value>& args) {
     if (args.empty() || !args[0].is_string()) {
          L_ERR("get_format_date() requires exactly one string argument");
          return LAMINA_NULL;
     }

     std::string formatStr = args[0].to_string();

     for (char c : formatStr) {
          if (c != 'Y' && c != 'm' && c != 'd' && c != '-') {
               L_ERR("Invalid format string. Only 'Y', 'm', 'd', '-' characters are allowed.");
               return LAMINA_NULL;
          }
     }

     std::stringstream ss(formatStr);
     std::string item;
     std::vector<std::string> parts;

     while (std::getline(ss, item, '-')) {
          parts.push_back(item);
     }

     std::string format;
     for (const auto& part : parts) {
          format += part;
     }

     auto now = std::chrono::system_clock::now();
     auto time = std::chrono::system_clock::to_time_t(now);
     std::tm* localTime = std::localtime(&time);

     std::string stdFormat;
     for (char c : format) {
          if (c == 'Y') stdFormat += "%Y";
          else if (c == 'm') stdFormat += "%m";
          else if (c == 'd') stdFormat += "%d";
          else stdFormat += c;
     }

     std::ostringstream oss;
     oss << std::put_time(localTime, stdFormat.c_str());
     std::string result = oss.str();

     return Value(result);
}
