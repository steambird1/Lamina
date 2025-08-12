/* 
     专门用于动态库加载的模块类
*/
#pragma once
#include <iostream>
#ifdef __linux__
#include <dlfcn.h>
#include <link.h>
#endif
#include "interpreter.hpp"
#include <functional>
#include <vector>

class ModuleLoader {
public:
    using EntryFunction = std::function<void(Interpreter&)>;
    ModuleLoader(const std::string& soPath);
    ~ModuleLoader();
    void* findSymbol(const std::string& symbolName);
    bool isLoaded() const;
    void* m_handle;
    std::vector<EntryFunction> findEntryFunctions();
};
