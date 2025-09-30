/*
 * 模块加载器实现
 */

#include "module_loader.hpp"
#include "interpreter.hpp"
#include <cstring>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// Helper functions for conversion
LaminaValue ModuleLoader::valueToLamina(const Value& val) {
    LaminaValue result;
    if (val.type == Value::Type::Null) {
        result = LAMINA_MAKE_NULL();
    } else if (val.type == Value::Type::Int) {
        result = LAMINA_MAKE_INT(std::get<int>(val.data));
    } else if (val.type == Value::Type::Float) {
        result = LAMINA_MAKE_INT(static_cast<int>(val.as_number()));
    } else if (val.type == Value::Type::String) {
        result = LAMINA_MAKE_STRING(val.to_string().c_str());
    } else {
        result = LAMINA_MAKE_NULL();
    }
    return result;
}

Value ModuleLoader::laminaToValue(const LaminaValue& val) {
    Value result;
    switch (val.type) {
        case LAMINA_TYPE_NULL:
            result = Value();
            break;
        case LAMINA_TYPE_INT:
            result = Value(val.data.int_val);
            break;
        case LAMINA_TYPE_STRING:
            result = Value(std::string(val.data.string_val));
            break;
        default:
            result = Value();
            break;
    }
    return result;
}

std::vector<LaminaValue> ModuleLoader::vectorToLamina(const std::vector<Value>& vals) {
    std::vector<LaminaValue> result;
    result.reserve(vals.size());
    for (const auto& val: vals) {
        result.push_back(valueToLamina(val));
    }
    return result;
}

std::vector<Value> ModuleLoader::laminaToVector(const LaminaValue* vals, int count) {
    std::vector<Value> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(laminaToValue(vals[i]));
    }
    return result;
}

ModuleLoader::ModuleLoader(const std::string& path) : m_handle(nullptr), m_path(path), m_exports(nullptr) {
    std::cout << "Loading module: " << path << std::endl;

    // 基本安全检查
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Module file does not exist: " << path << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.close();

    if (fileSize < 1024) {
        std::cerr << "Module file too small: " << path << std::endl;
        return;
    }

    // 加载动态库
#ifdef _WIN32
    m_handle = LoadLibraryA(path.c_str());
    if (!m_handle) {
        std::cerr << "Failed to load module: " << path << " Error: " << GetLastError() << std::endl;
        return;
    }
#else
    m_handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!m_handle) {
        std::cerr << "Failed to load module: " << path << " Error: " << dlerror() << std::endl;
        return;
    }
#endif

    // 验证模块签名
    const char* (*sig_func)() = nullptr;
#ifdef _WIN32
    sig_func = reinterpret_cast<const char* (*) ()>(GetProcAddress(static_cast<HMODULE>(m_handle), "lamina_module_signature"));
#else
    sig_func = (const char* (*) ()) dlsym(m_handle, "lamina_module_signature");
#endif

    if (!sig_func) {
        std::cerr << "Module missing signature function: " << path << std::endl;
        unload();
        return;
    }

    if (const auto signature = sig_func(); !signature || strcmp(signature, "LAMINA_MODULE_V2") != 0) {
        std::cerr << "Invalid module signature: " << (signature ? signature : "null") << std::endl;
        unload();
        return;
    }

    // 获取初始化函数
    LaminaModuleExports* (*init_func)() = nullptr;
#ifdef _WIN32
    init_func = reinterpret_cast<LaminaModuleExports* (*) ()>(GetProcAddress(static_cast<HMODULE>(m_handle), "lamina_module_init"));
#else
    init_func = (LaminaModuleExports * (*) ()) dlsym(m_handle, "lamina_module_init");
#endif

    if (!init_func) {
        std::cerr << "Module missing init function: " << path << std::endl;
        unload();
        return;
    }

    // 调用初始化函数
    m_exports = init_func();
    if (!m_exports) {
        std::cerr << "Module initialization failed: " << path << std::endl;
        unload();
        return;
    }

    //   std::cout << "Module loaded successfully: " << path << std::endl;
    //   std::cout << "  Namespace: " << m_exports->info.namespace_name << std::endl;
    //   std::cout << "  Version: " << m_exports->info.version << std::endl;
    //   std::cout << "  Description: " << m_exports->info.description << std::endl;
    //   std::cout << "  Functions: " << m_exports->function_count << std::endl;
}

ModuleLoader::~ModuleLoader() {
    unload();
}

void ModuleLoader::unload() {
    if (m_handle) {
#ifdef _WIN32
        FreeLibrary((HMODULE) m_handle);
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
    }
    m_exports = nullptr;
}

Value ModuleLoader::callModuleFunction(const std::string& full_func_name, const std::vector<Value>& args) const {
    if (!isLoaded()) {
        std::cerr << "ERROR: Module not loaded" << std::endl;
        return {};
    }

    // 处理带命名空间的函数名
    std::string actual_name = full_func_name;
    size_t dot_pos = full_func_name.find('.');
    if (dot_pos != std::string::npos) {
        std::string ns = full_func_name.substr(0, dot_pos);
        actual_name = full_func_name.substr(dot_pos + 1);
        if (ns != m_exports->info.namespace_name) {
            std::cerr << "ERROR: Namespace mismatch. Expected '" << m_exports->info.namespace_name
                      << "' but got '" << ns << "'" << std::endl;
            return {};
        }
    }

    // 查找函数
    const LaminaFunctionEntry* target_func = nullptr;
    for (int i = 0; i < m_exports->function_count; ++i) {
        if (m_exports->functions[i].name &&
            std::string(m_exports->functions[i].name) == actual_name) {
            target_func = const_cast<LaminaFunctionEntry*>(&m_exports->functions[i]);
            break;
        }
    }

    if (!target_func) {
        std::cerr << "ERROR: Function '" << actual_name << "' not found in module" << std::endl;
        return {};
    }

    if (!target_func->func) {
        std::cerr << "ERROR: Function '" << actual_name << "' has null pointer" << std::endl;
        return {};
    }

    // 转换参数
    const std::vector<LaminaValue> lamina_args = vectorToLamina(args);

    // 调用函数
    try {
        const LaminaValue result = target_func->func(
                lamina_args.empty() ? nullptr : lamina_args.data(),
                static_cast<int>(lamina_args.size()));

        // 转换返回值
        Value ret_val = laminaToValue(result);
        return ret_val;
    } catch (...) {
        std::cerr << "ERROR: Exception during function call" << std::endl;
        return {};
    }
}

bool ModuleLoader::registerToInterpreter(Interpreter& interpreter) const {
    if (!isLoaded()) {
        std::cerr << "ERROR: Cannot register unloaded module" << std::endl;
        return false;
    }

    // Module functions will be called through the call_module_function method
    // when the interpreter encounters a function call with dot notation
    return true;
}
