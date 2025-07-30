/*
     @Dev Ange1PlsGreet
*/
#include "module.hpp"

ModuleLoader::ModuleLoader(const std::string& soPath) {
#ifdef  __linux__
    m_handle = dlopen(soPath.c_str(), RTLD_LAZY);
    if (!m_handle) {
        std::cerr << "Failed to load library: " << dlerror() << std::endl;
    }
#endif
}

ModuleLoader::~ModuleLoader() {
#ifdef  __linux__
    if (m_handle) {
        dlclose(m_handle);
        m_handle = nullptr;
    }
#endif
}

void* ModuleLoader::findSymbol(const std::string& symbolName) {
    void* symbol = nullptr;
#ifdef __linux__
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
#ifdef __linux__
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
            if (sym->st_name && strtab + sym->st_name) {
                std::string symbolName = strtab + sym->st_name;
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
#endif
    return entryFunctions;
}