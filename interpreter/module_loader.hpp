/*
 * 模块加载器 - 基于纯C接口
 * 避免复杂C++对象跨DLL边界
 */

#ifndef LAMINA_MODULE_LOADER_HPP
#define LAMINA_MODULE_LOADER_HPP

#include "module_api.hpp"
#include "value.hpp"
#include <string>
#include <vector>

// 前向声明
class Interpreter;

class ModuleLoader {
private:
    void* m_handle;
    std::string m_path;
    LaminaModuleExports* m_exports;
    
    // Value转换工具
    static LaminaValue valueToLamina(const Value& val);
    static Value laminaToValue(const LaminaValue& val);
    static std::vector<LaminaValue> vectorToLamina(const std::vector<Value>& vals);
    static std::vector<Value> laminaToVector(const LaminaValue* vals, int count);

public:
    ModuleLoader(const std::string& path);
    ~ModuleLoader();
    
    bool isLoaded() const { return m_handle != nullptr && m_exports != nullptr; }
    bool registerToInterpreter(Interpreter& interpreter);
    
    // 安全的函数调用方法
    Value callModuleFunction(const std::string& func_name, const std::vector<Value>& args);
    
    const std::string& getPath() const { return m_path; }
    const LaminaModuleInfo* getModuleInfo() const { 
        return m_exports ? &m_exports->info : nullptr; 
    }
};

#endif // LAMINA_MODULE_LOADER_HPP
