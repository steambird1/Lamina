#include "random.hpp"
#include <random>

Value rand(const std::vector<Value>& args) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<long double> dis(0.0L, 1.0L);
    return LAMINA_DOUBLE(dis(gen));
}

Value randint(const std::vector<Value>& args) {
    int min;
    int max;

    static std::random_device rd;
    static std::mt19937 gen(rd());

    if (args[0].is_numeric() && args[1].is_numeric()) {
        min = std::stoi((args[0].to_string()));
        max = std::stoi((args[1].to_string()));
        std::uniform_int_distribution<> dis(min, max);
        int randomNumber = dis(gen);
        return LAMINA_INT(randomNumber);
    } else {
        L_ERR("randint() requires two numeric arguments");
        return LAMINA_NULL;
    }
}

Value randstr(const std::vector<Value>& args) {

    if (args.size() != 1 || !args[0].is_numeric()) {
        L_ERR("randstr() requires exactly one numeric argument");
        return LAMINA_NULL;
    }

    int length = std::stoi(args[0].to_string());
    if (length < 0) {
        L_ERR("randstr() length argument must be non-negative");
        return LAMINA_NULL;
    }

    static const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.size() - 1);

    std::string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }

    return LAMINA_STRING(result.c_str());
}
