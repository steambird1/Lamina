#include "array.hpp"

Value range(const std::vector<Value>& args) {
    if (args.empty()) return LAMINA_NULL;

    const int start = args.size() > 1
                ?std::get<int>(args[0].data)
                :0;
    const int end = args.size() > 1
                    ?std::get<int>(args[1].data)
                    :std::get<int>(args[0].data);
    const int sep = args.size() > 2
                    ? std::get<int>(args[2].data)
                    : 1;
    std::vector<Value> vec;
    for (auto i = start; i < end; i+=sep) {
        vec.emplace_back(i);
    }
    return vec;
}

Value visit_array_by_int(const std::vector<Value>& args) {
    if (!args[0].is_array()) {
        L_ERR("First Arg Must Be A Array");
        return LAMINA_NULL;
    }

    const Value* current = &args[0];

    for (size_t i = 1; i < args.size(); ++i) {
        if (!args[i].is_int()) {
            L_ERR("Index argument must be an integer");
            return LAMINA_NULL;
        }
        int index = std::get<int>(args[i].data);

        if (!current->is_array()) {
            L_ERR("Cannot index non-array value at level " + std::to_string(i));
            return LAMINA_NULL;
        }

        const auto& arr = std::get<std::vector<Value>>(current->data);

        if (index < 0 || static_cast<size_t>(index) >= arr.size()) {
            L_ERR("Array Index Out Of Range at level " + std::to_string(i));
            return LAMINA_NULL;
        }

        current = &arr[index];
    }

    return *current;
}

Value visit_array_by_str(const std::vector<Value>& args) {
    if (!args[0].is_array() || !args[1].is_string()) {
        L_ERR("Invalid arguments (expected array and string)");
        return LAMINA_NULL;
    }

    const std::string target_key = std::get<std::string>(args[1].data);
    const auto& arr = std::get<std::vector<Value>>(args[0].data);
    Value result = LAMINA_NULL;
    bool found = false;


    if (!found && arr.size() % 2 == 0) {
        for (size_t i = 0; i < arr.size(); i += 2) {
            if (i + 1 >= arr.size()) break;

            const auto& key_elem = arr[i];
            const auto& value_elem = arr[i + 1];

            if (!key_elem.is_string()) continue;

            const std::string current_key = std::get<std::string>(key_elem.data);
            if (current_key == target_key) {
                result = value_elem;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        L_ERR("Key '" + target_key + "' not found in array");
        return LAMINA_NULL;
    }

    return result;
}