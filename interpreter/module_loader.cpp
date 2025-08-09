/*
 * 模块加载器实现
 */

#include "module_loader.hpp"
#include "interpreter.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

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
    
    // 检查模块签名
    auto signature_func = (const char*(*)())GetProcAddress((HMODULE)m_handle, "lamina_module_signature");
    std::cout << "Signature function pointer: " << (void*)signature_func << std::endl;
    if (signature_func) {
        const char* signature = signature_func();
        std::cout << "Found signature: " << signature << std::endl;
        std::cout << "Expected signature: " << LAMINA_MODULE_SIGNATURE << std::endl;
        if (strcmp(signature, LAMINA_MODULE_SIGNATURE) != 0) {
            std::cerr << "Invalid module signature in: " << path << std::endl;
            FreeLibrary((HMODULE)m_handle);
            m_handle = nullptr;
            return;
        }
    } else {
        std::cerr << "Module signature function not found in: " << path << std::endl;
        FreeLibrary((HMODULE)m_handle);
        m_handle = nullptr;
        return;
    }
    
    // 获取模块入口函数
    auto init_func = (LaminaModuleInit)GetProcAddress((HMODULE)m_handle, "lamina_module_init");
    if (!init_func) {
        std::cerr << "Module init function not found in: " << path << std::endl;
        FreeLibrary((HMODULE)m_handle);
        m_handle = nullptr;
        return;
    }
#else
    m_handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!m_handle) {
        std::cerr << "Failed to load module: " << path << " Error: " << dlerror() << std::endl;
        return;
    }
    
    // 检查模块签名
    auto signature_func = (const char*(*)())dlsym(m_handle, "lamina_module_signature");
    if (signature_func) {
        const char* signature = signature_func();
        if (strcmp(signature, LAMINA_MODULE_SIGNATURE) != 0) {
            std::cerr << "Invalid module signature in: " << path << std::endl;
            dlclose(m_handle);
            m_handle = nullptr;
            return;
        }
    } else {
        std::cerr << "Module signature function not found in: " << path << std::endl;
        dlclose(m_handle);
        m_handle = nullptr;
        return;
    }
    
    // 获取模块入口函数
    auto init_func = (LaminaModuleInit)dlsym(m_handle, "lamina_module_init");
    if (!init_func) {
        std::cerr << "Module init function not found in: " << path << std::endl;
        dlclose(m_handle);
        m_handle = nullptr;
        return;
    }
#endif
    
    // 调用模块初始化函数
    try {
        m_exports = init_func();
        if (!m_exports) {
            std::cerr << "Module init function returned null: " << path << std::endl;
#ifdef _WIN32
            FreeLibrary((HMODULE)m_handle);
#else
            dlclose(m_handle);
#endif
            m_handle = nullptr;
            return;
        }
        
        std::cout << "Module loaded successfully: " << m_exports->info.namespace_name 
                  << " v" << m_exports->info.version << std::endl;
                  
    } catch (const std::exception& e) {
        std::cerr << "Exception during module init: " << e.what() << std::endl;
#ifdef _WIN32
        FreeLibrary((HMODULE)m_handle);
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
        m_exports = nullptr;
    }
}

ModuleLoader::~ModuleLoader() {
    if (m_handle) {
#ifdef _WIN32
        FreeLibrary((HMODULE)m_handle);
#else
        dlclose(m_handle);
#endif
    }
}

bool ModuleLoader::registerToInterpreter(Interpreter& interpreter) {
    if (!isLoaded()) return false;
    
    const std::string& ns_name = m_exports->info.namespace_name;
    
    // 注册域
    interpreter.register_namespace(ns_name);
    
    // 注册函数到命名空间
    for (int i = 0; i < m_exports->function_count; ++i) {
        const auto& func_entry = m_exports->functions[i];
        std::string func_name = func_entry.name;
        
        // 使用更安全的跨平台函数调用方式
        auto wrapper = [this, func_name](const std::vector<Value>& args) -> Value {
            return callModuleFunction(func_name, args);
        };
        
        interpreter.register_namespace_function(ns_name, func_name, wrapper);
        std::cout << "Function '" << ns_name << "::" << func_name << "' registered" << std::endl;
    }
    
    // 注册变量
    for (int i = 0; i < m_exports->variable_count; ++i) {
        const auto& var_entry = m_exports->variables[i];
        Value val = laminaToValue(var_entry.value);
        interpreter.register_namespace_variable(ns_name, var_entry.name, val);
    }
    
    return true;
}

// 安全的跨平台函数调用实现
Value ModuleLoader::callModuleFunction(const std::string& func_name, const std::vector<Value>& args) {
    if (!isLoaded()) {
        std::cerr << "Module not loaded" << std::endl;
        return Value();
    }
    
    // 查找函数
    LaminaFunctionEntry* target_func = nullptr;
    for (int i = 0; i < m_exports->function_count; ++i) {
        if (func_name == m_exports->functions[i].name) {
            target_func = const_cast<LaminaFunctionEntry*>(&m_exports->functions[i]);
            break;
        }
    }
    
    if (!target_func || !target_func->func) {
        std::cerr << "Function '" << func_name << "' not found" << std::endl;
        return Value();
    }
    
    // 转换参数 - 使用更安全的方式
    std::vector<LaminaValue> lamina_args;
    std::vector<std::string> string_storage; // 保证字符串生命周期
    
    lamina_args.reserve(args.size());
    string_storage.reserve(args.size());
    
    for (const auto& arg : args) {
        if (arg.is_string()) {
            string_storage.push_back(arg.to_string());
            lamina_args.push_back(LAMINA_MAKE_STRING(string_storage.back().c_str()));
        } else if (arg.is_int()) {
            lamina_args.push_back(LAMINA_MAKE_INT(static_cast<int>(arg.as_number())));
        } else if (arg.is_float()) {
            lamina_args.push_back(LAMINA_MAKE_DOUBLE(arg.as_number()));
        } else if (arg.is_bool()) {
            lamina_args.push_back(LAMINA_MAKE_BOOL(arg.as_bool()));
        } else {
            lamina_args.push_back(LAMINA_MAKE_NULL());
        }
    }
    
    // 执行函数调用 - 添加异常保护
    try {
        LaminaValue result = target_func->func(
            lamina_args.empty() ? nullptr : lamina_args.data(), 
            static_cast<int>(lamina_args.size())
        );
        
        return laminaToValue(result);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in module function '" << func_name << "': " << e.what() << std::endl;
        return Value();
    } catch (...) {
        std::cerr << "Unknown exception in module function '" << func_name << "'" << std::endl;
        return Value();
    }
}// 静态工具函数实现
LaminaValue ModuleLoader::valueToLamina(const Value& val) {
    if (val.is_null()) {
        return LAMINA_MAKE_NULL();
    } else if (val.is_bool()) {
        return LAMINA_MAKE_BOOL(val.as_bool());
    } else if (val.is_int()) {
        return LAMINA_MAKE_INT(static_cast<int>(val.as_number()));
    } else if (val.is_float()) {
        return LAMINA_MAKE_DOUBLE(val.as_number());
    } else if (val.is_string()) {
        // 注意：字符串生命周期由调用者管理
        // 这个方法主要用于非字符串类型，字符串由vectorToLamina特殊处理
        return LAMINA_MAKE_NULL();
    }
    return LAMINA_MAKE_NULL();
}

Value ModuleLoader::laminaToValue(const LaminaValue& val) {
    switch (val.type) {
        case LAMINA_TYPE_NULL:
            return Value();
        case LAMINA_TYPE_BOOL:
            return Value(val.data.bool_val != 0);
        case LAMINA_TYPE_INT:
            return Value(val.data.int_val);
        case LAMINA_TYPE_DOUBLE:
            return Value(val.data.double_val);
        case LAMINA_TYPE_STRING:
            return Value(std::string(val.data.string_val ? val.data.string_val : ""));
        default:
            return Value();
    }
}

std::vector<LaminaValue> ModuleLoader::vectorToLamina(const std::vector<Value>& vals) {
    std::cout << "DEBUG: vectorToLamina start, size=" << vals.size() << std::endl;
    std::vector<LaminaValue> result;
    static thread_local std::vector<std::string> string_storage;
    string_storage.clear(); // 清理之前的字符串
    
    result.reserve(vals.size());
    for (const auto& val : vals) {
        std::cout << "DEBUG: processing value, is_string=" << val.is_string() << std::endl;
        if (val.is_string()) {
            // 为字符串参数创建持久存储
            string_storage.push_back(val.to_string());
            LaminaValue lv = LAMINA_MAKE_STRING(string_storage.back().c_str());
            result.push_back(lv);
        } else {
            result.push_back(valueToLamina(val));
        }
    }
    std::cout << "DEBUG: vectorToLamina complete" << std::endl;
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
