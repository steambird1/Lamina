#pragma once
#include "lamina_api/value.hpp"
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstddef>
#include <cstring>  // 用于 strstr/strcmp

// 平台宏适配
#ifdef _WIN32
#include <windows.h>
#include <imagehlp.h>  // Windows 符号表解析
#include <tlhelp32.h>
// 链接 ImageHlp 库（Windows 必需）
#pragma comment(lib, "imagehlp.lib")

#define DYLIB_HANDLE HMODULE
#define DYLIB_LOAD(path) LoadLibraryA(path.c_str())
#define DYLIB_GETSYM(handle, name) reinterpret_cast<void*>(GetProcAddress(handle, name))
#define DYLIB_UNLOAD(handle) FreeLibrary(handle)
#define DYLIB_ERROR() GetLastError()
#else
#include <dlfcn.h>
#include <link.h>  
#include <elf.h>

#ifndef STT_DEBUG
#define STT_DEBUG 0x11
#endif

#define DYLIB_HANDLE void*
#define DYLIB_LOAD(path) dlopen(path.c_str(), RTLD_LAZY | RTLD_NOLOAD)  // 加载后不重复加载
#define DYLIB_GETSYM(handle, name) dlsym(handle, name)
#define DYLIB_UNLOAD(handle) dlclose(handle)
#define DYLIB_ERROR() dlerror()
#endif

// 目标函数指针类型
using ModuleFunc = Value (*)(const std::vector<Value>&);

#ifdef _WIN32
// Windows：获取动态库的所有导出符号名
inline std::vector<std::string> get_win_dylib_symbols(DYLIB_HANDLE hModule) {
    std::vector<std::string> symbols;
    if (!hModule) return symbols;

    // 获取模块文件路径
    char module_path[MAX_PATH] = {0};
    GetModuleFileNameA(hModule, module_path, MAX_PATH);

    // 初始化符号解析器
    LOADED_IMAGE loaded_img = {nullptr};
    if (!MapAndLoad(module_path, nullptr, &loaded_img, FALSE, TRUE)) {
        std::cerr << "Failed to map module: " << GetLastError() << std::endl;
        return symbols;
    }

    // 获取导出表（PE 文件结构：IMAGE_EXPORT_DIRECTORY）
    const PIMAGE_NT_HEADERS nt_headers = ImageNtHeader(loaded_img.MappedAddress);
    const auto export_dir =
        static_cast<PIMAGE_EXPORT_DIRECTORY>(
            ImageRvaToVa(nt_headers, loaded_img.MappedAddress, 
                        nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, 
                        nullptr)
        );
    if (!export_dir) {
        std::cerr << "No export directory found" << std::endl;
        UnMapAndLoad(&loaded_img);
        return symbols;
    }

    // 遍历导出符号名（AddressOfNames 是符号名的 RVA 数组）
    const auto* name_rvas = static_cast<DWORD*>(
        ImageRvaToVa(nt_headers, loaded_img.MappedAddress, export_dir->AddressOfNames, nullptr)
    );
    for (DWORD i = 0; i < export_dir->NumberOfNames; ++i) {
        auto sym_name = static_cast<const char*>(
            ImageRvaToVa(nt_headers, loaded_img.MappedAddress, name_rvas[i], nullptr)
        );
        if (sym_name && *sym_name != '\0') {
            symbols.emplace_back(sym_name);
        }
    }

    UnMapAndLoad(&loaded_img);
    return symbols;
}
#endif

#ifndef _WIN32
// Linux/macOS：获取动态库的所有导出符号名（回调函数）
static int elf_symbol_callback(struct dl_phdr_info* info, size_t size, void* data) {
    std::vector<std::string>* symbols = reinterpret_cast<std::vector<std::string>*>(data);
    if (!symbols) return 1;  // 终止遍历

    // 打开当前模块的符号表（RTLD_NOLOAD：不重复加载）
    void* handle = dlopen(info->dlpi_name, RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) return 0;  // 跳过非目标模块

    // 遍历 ELF 段（寻找 .dynsym 动态符号表）
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        const Elf64_Phdr* phdr = &info->dlpi_phdr[i];
        // 仅处理动态符号表段（PT_DYNAMIC）
        if (phdr->p_type != PT_DYNAMIC) continue;

        // 解析动态符号表（Elf64_Dyn：动态段条目）
        Elf64_Dyn* dyn = reinterpret_cast<Elf64_Dyn*>(info->dlpi_addr + phdr->p_vaddr);
        Elf64_Sym* symtab = nullptr;  // 符号表基地址
        const char* strtab = nullptr; // 字符串表基地址
        Elf64_Word symcount = 0;      // 符号数量

        // 遍历动态段，提取符号表/字符串表信息
        for (; dyn->d_tag != DT_NULL; ++dyn) {
            switch (dyn->d_tag) {
                case DT_SYMTAB: symtab = reinterpret_cast<Elf64_Sym*>(dyn->d_un.d_ptr); break;
                case DT_STRTAB: strtab = reinterpret_cast<const char*>(dyn->d_un.d_ptr); break;
                case DT_SYMENT: symcount = phdr->p_memsz / sizeof(Elf64_Sym); break; // 符号总数
            }
        }

        if (!symtab || !strtab || symcount == 0) continue;

        // 遍历符号表，筛选导出符号（STB_GLOBAL：全局符号，STT_OBJECT/STT_FUNC：变量/函数）
        for (Elf64_Word i = 0; i < symcount; ++i) {
            const Elf64_Sym* sym = &symtab[i];
            // 条件：全局符号 + 有名字 + 非调试符号
            if (ELF64_ST_BIND(sym->st_info) != STB_GLOBAL || 
                ELF64_ST_TYPE(sym->st_info) == STT_DEBUG || 
                sym->st_name == 0) {
                continue;
            }
            const char* sym_name = strtab + sym->st_name;
            // 排除动态库内部符号（如 _init、_fini）
            if (strstr(sym_name, "_init") || strstr(sym_name, "_fini")) continue;
            symbols->emplace_back(sym_name);
        }
    }

    dlclose(handle);
    return 0;  // 继续遍历其他模块
}

// Linux/macOS：触发符号表遍历，返回所有导出符号名
std::vector<std::string> get_elf_dylib_symbols(const std::string& so_path) {
    std::vector<std::string> symbols;
    // 调用 dl_iterate_phdr，通过回调收集符号
    dl_iterate_phdr(elf_symbol_callback, &symbols);
    return symbols;
}
#endif


inline std::unordered_map<std::string, Value> load_cppmodule(const std::string& path) {
    std::unordered_map<std::string, Value> result;
    DYLIB_HANDLE handle = nullptr;
    std::vector<std::string> all_symbols;

    // 加载动态库
    handle = DYLIB_LOAD(path);
    if (!handle) {
        std::cerr << "Load module failed: " << DYLIB_ERROR() << std::endl;
        return result;
    }

    // 获取动态库的所有导出符号名
#ifdef _WIN32
    all_symbols = get_win_dylib_symbols(handle);
#else
    all_symbols = get_elf_dylib_symbols(path);
#endif
    if (all_symbols.empty()) {
        std::cerr << "No exported symbols found in module" << std::endl;
        DYLIB_UNLOAD(handle);
        return result;
    }

    for (const auto& sym_name : all_symbols) {
        if (sym_name.starts_with("lamina_protect_")) continue;
        // 尝试解析为 Value 类型变量
        void* sym_addr = DYLIB_GETSYM(handle, sym_name.c_str());
        if (!sym_addr) continue;

        const auto* var_ptr = static_cast<Value*>(sym_addr);
        // Value 的 type 必须是合法值
        bool is_valid_value_var = (!var_ptr->is_lmCppFunction());
        if (is_valid_value_var) {
            result[sym_name] = *var_ptr;
            std::cout << "Found Value var: " << sym_name << std::endl;
            continue;
        }

        // 尝试解析为 "ModuleFunc 类型函数"（符号地址是函数指针）
        auto func_ptr = reinterpret_cast<ModuleFunc>(sym_addr);
        if (func_ptr != nullptr) {
            result[sym_name] = Value(std::make_shared<LmCppFunction>(func_ptr));
            std::cout << "Found ModuleFunc: " << sym_name << std::endl;
            continue;
        }


        std::cerr << "Skip unknown type symbol: " << sym_name << std::endl;
    }

    // 注意：动态库句柄需保留（后续调用函数依赖），建议在 VM 生命周期结束时统一卸载
    // DYLIB_UNLOAD(handle); 

    return result;
}

