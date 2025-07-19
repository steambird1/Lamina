#include "interpreter.hpp"
#include "lamina.hpp"
#include "value.hpp"


inline Value sqrt(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          std::cerr << "Error: sqrt() requires numeric argument" << std::endl;
          return Value();
     }

     // For exact square root representation
     if (args[0].is_int()) {
          int val = std::get<int>(args[0].data);
          if (val < 0) {
               std::cerr << "Error: sqrt() of negative number" << std::endl;
               return Value();
          }
          if (val == 0 || val == 1) {
               return Value(val);
          }
          // Check if it's a perfect square
          int sqrt_val = static_cast<int>(std::sqrt(val));
          if (sqrt_val * sqrt_val == val) {
               return Value(sqrt_val);
          }
          // Return exact irrational representation
          return Value(::Irrational::sqrt(val));
     }

     // For other numeric types, use floating point
     double val = args[0].as_number();
     if (val < 0) {
          std::cerr << "Error: sqrt() of negative number" << std::endl;
          return Value();
     }
     return Value(std::sqrt(val));
}

inline Value pi(const std::vector<Value>& args) {
     return Value(::Irrational::pi());
}

inline Value e(const std::vector<Value>& args) {
     return Value(::Irrational::e());
}

inline Value abs(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          std::cerr << "Error: abs() requires numeric argument" << std::endl;
          return Value();
     }
     double val = args[0].as_number();
     return Value(std::abs(val));
}

inline Value sin(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          std::cerr << "Error: sin() requires numeric argument" << std::endl;
          return Value();
     }
     return Value(std::sin(args[0].as_number()));
}

inline Value cos(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          std::cerr << "Error: cos() requires numeric argument" << std::endl;
          return Value();
     }
     return Value(std::cos(args[0].as_number()));
}

inline Value tan(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          std::cerr << "Error: tan() requires numeric argument" << std::endl;
          return Value();
     }
     return Value(std::tan(args[0].as_number()));
}

inline Value log(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          error_and_exit("log() requires numeric argument");
     }
     double val = args[0].as_number();
     if (val <= 0) {
          error_and_exit("log() requires positive argument");
     }
     return Value(std::log(val));
}

inline Value round(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          error_and_exit("round() requires numeric argument");
     }
     return Value(static_cast<int>(std::round(args[0].as_number())));
}

inline Value floor(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          error_and_exit("floor() requires numeric argument");
     }
     return Value(static_cast<int>(std::floor(args[0].as_number())));
}

inline Value ceil(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          error_and_exit("ceil() requires numeric argument");
     }
     return Value(static_cast<int>(std::ceil(args[0].as_number())));
}

inline Value dot(const std::vector<Value>& args) {
     return args[0].dot_product(args[1]);
}

inline Value cross(const std::vector<Value>& args) {
     return args[0].cross_product(args[1]);
}

inline Value norm(const std::vector<Value>& args) {
     return args[0].magnitude();
}

inline Value normalize(const std::vector<Value>& args) {
     return args[0].normalize();
}

inline Value det(const std::vector<Value>& args) {
     return args[0].determinant();
}

inline Value size(const std::vector<Value>& args) {
     if (args[0].is_array()) {
          const auto& arr = std::get<std::vector<Value>>(args[0].data);
          return Value(static_cast<int>(arr.size()));
     }
     else if (args[0].is_matrix()) {
          const auto& mat = std::get<std::vector<std::vector<Value>>>(args[0].data);
          return Value(static_cast<int>(mat.size()));
     }
     else if (args[0].is_string()) {
          const auto& str = std::get<std::string>(args[0].data);
          return Value(static_cast<int>(str.length()));
     }
     return Value(1); // Scalar values have size 1
}

inline Value idiv(const std::vector<Value>& args) {
     if (!args[0].is_numeric() || !args[1].is_numeric()) {
          error_and_exit("idiv() requires numeric arguments");
     }
     double divisor = args[1].as_number();
     if (divisor == 0.0) {
          error_and_exit("Integer division by zero");
     }
     double dividend = args[0].as_number();
     return Value(static_cast<int>(dividend / divisor));
}

inline Value fraction(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          std::cerr << "Error: fraction() requires numeric argument" << std::endl;
          return Value();
     }

     // If already a rational, return as is
     if (args[0].is_rational()) {
          return args[0];
     }

     // Convert double to rational approximation
     double val = args[0].as_number();
     try {
          Rational rat = Rational::from_double(val);
          return Value(rat);
     } catch (const std::exception& e) {
          std::cerr << "Error: Cannot convert to fraction: " << e.what() << std::endl;
          return Value();
     }
}

inline Value decimal(const std::vector<Value>& args) {
     if (!args[0].is_numeric()) {
          std::cerr << "Error: decimal() requires numeric argument" << std::endl;
          return Value();
     }

     // Convert to double representation
     double val = args[0].as_number();
     return Value(val);
}
namespace lamina {
     LAMINA_FUNC("sqrt", sqrt, 1);
     LAMINA_FUNC("pi", pi, 0);
     LAMINA_FUNC("e", e, 0);
     LAMINA_FUNC("abs", abs, 1);
     LAMINA_FUNC("sin", sin, 1);
     LAMINA_FUNC("cos", cos, 1);
     LAMINA_FUNC("tan", tan, 1);
     LAMINA_FUNC("log", log, 1);
     LAMINA_FUNC("round", round, 1);
     LAMINA_FUNC("floor", floor, 1);
     LAMINA_FUNC("ceil", ceil, 1);
     LAMINA_FUNC("dot", dot, 2);
     LAMINA_FUNC("cross", cross, 2);
     LAMINA_FUNC("norm", norm, 1);
     LAMINA_FUNC("normalize", normalize, 1);
     LAMINA_FUNC("det", det, 1);
     LAMINA_FUNC("size", size, 1);
     LAMINA_FUNC("idiv", idiv, 2);
     LAMINA_FUNC("fraction", fraction, 1);
     LAMINA_FUNC("decimal", decimal, 1);
}
