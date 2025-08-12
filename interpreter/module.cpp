/*
     @Dev Ange1PlsGreet
*/

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#ifdef __linux__
#include <elf.h>
#include <link.h>
#endif
#endif
#include "module.hpp"

ModuleLoader::ModuleLoader(const std::string& soPath) {
#ifdef _WIN32
    m_handle = (void*) LoadLibraryA(soPath.c_str());
    if (!m_handle) {
        std::cerr << "Failed to load library: " << soPath << std::endl;
    }
#elif __linux__
    m_handle = dlopen(soPath.c_str(), RTLD_LAZY);
    if (!m_handle) {
        std::cerr << "Failed to load library: " << dlerror() << std::endl;
    }
#else
    // 对于其他 Unix-like 系统（如 macOS）
    m_handle = dlopen(soPath.c_str(), RTLD_LAZY);
    if (!m_handle) {
        std::cerr << "Failed to load library: " << dlerror() << std::endl;
    }
#endif
}

ModuleLoader::~ModuleLoader() {
#ifdef _WIN32
    if (m_handle) {
        FreeLibrary((HMODULE) m_handle);
        m_handle = nullptr;
    }
#elif __linux__
    if (m_handle) {
        dlclose(m_handle);
        m_handle = nullptr;
    }
#else
    // 对于其他 Unix-like 系统（如 macOS）
    if (m_handle) {
        dlclose(m_handle);
        m_handle = nullptr;
    }
#endif
}

void* ModuleLoader::findSymbol(const std::string& symbolName) {
    void* symbol = nullptr;
#ifdef _WIN32
    if (!m_handle) {
        return nullptr;
    }
    symbol = (void*) GetProcAddress((HMODULE) m_handle, symbolName.c_str());
    if (!symbol) {
        std::cerr << "Error looking up symbol '" << symbolName << "' in DLL." << std::endl;
        return nullptr;
    }
#elif __linux__
    if (!m_handle) {
        return nullptr;
    }
    dlerror();
    symbol = dlsym(m_handle, symbolName.c_str());
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Error looking up symbol '" << symbolName << "': " << dlsym_error << std::endl;
        return nullptr;
    }
#else
    // 对于其他 Unix-like 系统（如 macOS）
    if (!m_handle) {
        return nullptr;
    }
    dlerror();
    symbol = dlsym(m_handle, symbolName.c_str());
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Error looking up symbol '" << symbolName << "': " << dlsym_error << std::endl;
        return nullptr;
    }
#endif
    return symbol;
}

bool ModuleLoader::isLoaded() const {
    return m_handle != nullptr;
}

std::vector<ModuleLoader::EntryFunction> ModuleLoader::findEntryFunctions() {
    std::vector<EntryFunction> entryFunctions;
    if (!m_handle) return entryFunctions;

#ifdef _WIN32
    // 只查找 _entry 符号
    void* sym = findSymbol("_entry");
    if (sym) {
        auto entryFunc = reinterpret_cast<void (*)(Interpreter&)>(sym);
        entryFunctions.push_back([entryFunc](Interpreter& interpreter) {
            entryFunc(interpreter);
        });
    }
#elif __ANDROID__
    // Android跳过
    void* sym = dlsym(m_handle, "_entry");
    if (sym) {
        auto entryFunc = reinterpret_cast<void (*)(Interpreter&)>(sym);
        entryFunctions.push_back([entryFunc](Interpreter& interpreter) {
            entryFunc(interpreter);
        });
    }
#elif __linux__
    if (!m_handle) {
        return entryFunctions;
    }

    struct link_map* lmap;
    dlinfo(m_handle, RTLD_DI_LINKMAP, &lmap);

    const ElfW(Sym)* symtab = nullptr;
    const char* strtab = nullptr;
    size_t symtabsz = 0;

    for (ElfW(Dyn)* d = lmap->l_ld; d->d_tag != DT_NULL; ++d) {
        if (d->d_tag == DT_SYMTAB) {
            symtab = reinterpret_cast<const ElfW(Sym)*>(d->d_un.d_ptr);
        } else if (d->d_tag == DT_STRTAB) {
            strtab = reinterpret_cast<const char*>(d->d_un.d_ptr);
        } else if (d->d_tag == DT_SYMENT) {
            symtabsz = d->d_un.d_val;
        }
    }

    if (symtab && strtab && symtabsz > 0) {
        size_t symcount = (reinterpret_cast<const char*>(strtab) - reinterpret_cast<const char*>(symtab)) / symtabsz;

        for (size_t j = 0; j < symcount; ++j) {
            const ElfW(Sym)* sym = &symtab[j];
            if (sym->st_name != 0) {
                const char* symbolNamePtr = strtab + sym->st_name;
                std::string symbolName = symbolNamePtr;
                if (symbolName.find("_entry") != std::string::npos) {
                    void* symbolAddr = findSymbol(symbolName.c_str());
                    if (symbolAddr) {
                        auto entryFunc = reinterpret_cast<void (*)(Interpreter&)>(symbolAddr);
                        entryFunctions.emplace_back([entryFunc](Interpreter& interpreter) {
                            entryFunc(interpreter);
                        });
                    }
                }
            }
        }
    }
    return entryFunctions;
#else
    // 对于其他 Unix-like 系统（如 macOS），使用简单的 dlsym 查找
    void* sym = dlsym(m_handle, "_entry");
    if (sym) {
        auto entryFunc = reinterpret_cast<void (*)(Interpreter&)>(sym);
        entryFunctions.push_back([entryFunc](Interpreter& interpreter) {
            entryFunc(interpreter);
        });
    }
#endif
    return entryFunctions;
}
