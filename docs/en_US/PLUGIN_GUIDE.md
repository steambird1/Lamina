# Lamina Module Development Guide

<div align="right">
  <a href="../zh_TW/PLUGIN_GUIDE.md">🇹🇼 繁體中文</a> | <a href="../zh_CN/PLUGIN_GUIDE.md">🇨🇳 简体中文</a> | <strong>🇺🇸 English</strong>
</div>

## Overview
Lamina supports extending functionality via dynamic library modules (DLL/SO). The module system adopts a modern design, providing a type-safe cross-platform API, namespace management, and function registration mechanisms.

## Project Structure
```
Lamina/
├── interpreter/          # Core interpreter
│   ├── module_api.hpp    # Module API definitions
│   ├── module_loader.hpp # Module loader
│   └── ...               # Other core files
├── extensions/           # Extension module source code
│   ├── minimal/          # Example module
│   │   ├── ultra_minimal.cpp  # Module implementation
│   │   └── CMakeLists.txt     # Build configuration
│   └── standard/         # Standard library modules
│       ├── array.cpp     # Array operations
│       ├── math.cpp      # Math functions
│       ├── stdio.cpp     # Input/Output
│       └── ...           # Other standard libraries
├── modules/              # Compiled modules (.dll/.so)
└── test/                 # Test files
```

## Module API Overview

### Core Features
- **Namespace Support**: Each module has its own namespace, avoiding function name collisions
- **Type Safety**: Unified `LaminaValue` type system, supporting basic data types
- **Cross-Platform Compatibility**: Uses `LAMINA_EXPORT` and `LAMINA_CALL` macros to ensure compatibility across platforms
- **Dynamic Loading**: Modules are loaded at runtime, no need to recompile the interpreter

### Data Type System
```cpp
// Supported value types
typedef enum {
    LAMINA_TYPE_NULL = 0,
    LAMINA_TYPE_BOOL,
    LAMINA_TYPE_INT,      // Integer type
    LAMINA_TYPE_NUMBER,   // Floating-point type
    LAMINA_TYPE_STRING    // String type
} LaminaValueType;

// Unified value structure
typedef struct {
    LaminaValueType type;
    union {
        bool bool_val;
        int int_val;
        double number_val;
        const char* string_val;
    } data;
} LaminaValue;
```

### Module Interface Structure
```cpp
// Module function signature
typedef LaminaValue (LAMINA_CALL *LaminaFunction)(const LaminaValue* args, int argc);

// Function registration entry
typedef struct {
    const char* name;        // Function name
    LaminaFunction func;     // Function pointer
    const char* description; // Function description
} LaminaFunctionEntry;

// Module information
typedef struct {
    const char* namespace_name; // Namespace name
    const char* version;        // Version
    const char* description;    // Module description
} LaminaModuleInfo;

// Module exports structure
typedef struct {
    LaminaModuleInfo info;           // Basic module info
    LaminaFunctionEntry* functions;  // Exported functions array
    int function_count;              // Number of functions
} LaminaModuleExports;
```

## Creating Your First Module

### 1. Minimal Module Example
Below is a complete minimal module implementation (see `extensions/minimal/ultra_minimal.cpp`):

```cpp
#include "../../interpreter/module_api.hpp"
#include <cstdio>

// Simple test function
LAMINA_EXPORT LaminaValue LAMINA_CALL test_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};  // Zero-initialized
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = 42;
    
    printf("test_function called successfully!\n");
    return result;
}

// More complex function example
LAMINA_EXPORT LaminaValue LAMINA_CALL hello_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = 100;
    
    printf("Hello from minimal module function!\n");
    return result;
}

// Exported functions table
static LaminaFunctionEntry functions[] = {
    {"test", test_function, "Simple test function returning 42"},
    {"hello", hello_function, "Hello function returning 100"}
};

// Module exports structure
static LaminaModuleExports exports = {
    {
        "minimal",                    // Namespace name
        "1.0.0",                     // Version
        "Ultra minimal test module"   // Description
    },
    functions,                       // Functions array
    2                               // Number of functions
};

// Required export function
LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    return &exports;
}
```

### 2. CMakeLists.txt Configuration
```cmake
# Minimal module build configuration
cmake_minimum_required(VERSION 3.10)

# Set module name
set(MODULE_NAME minimal)

# Create shared library
add_library(${MODULE_NAME} SHARED
    ultra_minimal.cpp
)

# Set output directory
set_target_properties(${MODULE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
)

# Cross-platform build settings
if(WIN32)
    # Windows-specific settings
    set_target_properties(${MODULE_NAME} PROPERTIES
        SUFFIX ".dll"
        PREFIX ""
    )
else()
    # Linux/macOS settings
    set_target_properties(${MODULE_NAME} PROPERTIES
        SUFFIX ".so"
        PREFIX "lib"
    )
endif()

# C++ standard
set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
```
## Building and Using Modules

### Building Modules
```bash
# Configure build
cmake -B build -DCMAKE_BUILD_TYPE=Debug .

# Build project (including all modules)
cmake --build build --config Debug --parallel

# Modules will be output to: build/Debug/minimal.dll (Windows) or build/Debug/libminimal.so (Linux)
```

### Using Modules in Lamina Scripts
```lamina
// Load module
include "minimal.dll";

// Call module functions
let result = minimal.test();
print("Test result: " + result);  // Output: Test result: 42

let hello_result = minimal.hello();
print("Hello result: " + hello_result);  // Output: Hello result: 100
```

### Running Example
```bash
# Make sure module file is accessible
cd build/Debug

# Run Lamina interpreter
./Lamina.exe ../../test/test_minimal.lamina
```

## Advanced Module Development

### Argument Processing and Type Checking
```cpp
LAMINA_EXPORT LaminaValue LAMINA_CALL advanced_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};
    
    // Argument count check
    if (argc < 2) {
        result.type = LAMINA_TYPE_STRING;
        result.data.string_val = "Error: Need at least 2 arguments";
        return result;
    }
    
    // Type check
    if (args[0].type != LAMINA_TYPE_INT || args[1].type != LAMINA_TYPE_INT) {
        result.type = LAMINA_TYPE_STRING;
        result.data.string_val = "Error: Arguments must be integers";
        return result;
    }
    
    // Perform computation
    int a = args[0].data.int_val;
    int b = args[1].data.int_val;
    
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = a * b;
    return result;
}
```

### String Processing
```cpp
LAMINA_EXPORT LaminaValue LAMINA_CALL string_processor(const LaminaValue* args, int argc) {
    static char buffer[512];  // Static buffer ensures lifetime
    LaminaValue result = {0};
    
    if (argc < 1 || args[0].type != LAMINA_TYPE_STRING) {
        result.type = LAMINA_TYPE_STRING;
        result.data.string_val = "Error: Expected string argument";
        return result;
    }
    
    // Safe string processing
    snprintf(buffer, sizeof(buffer), "Processed: %s", args[0].data.string_val);
    
    result.type = LAMINA_TYPE_STRING;
    result.data.string_val = buffer;
    return result;
}
```

### Error Handling Patterns
```cpp
// Unified error return pattern
static LaminaValue make_error(const char* message) {
    LaminaValue error = {0};
    error.type = LAMINA_TYPE_STRING;
    error.data.string_val = message;
    return error;
}

// Unified success return pattern
static LaminaValue make_success_int(int value) {
    LaminaValue result = {0};
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = value;
    return result;
}
```

## Best Practices

### 1. Module Design Principles
- **Single Responsibility**: Each module focuses on a specific functional area
- **Clear Namespace**: Use concise and descriptive namespace names
- **Function Naming**: Use verbs to describe function behavior, keep naming consistent
- **Versioning**: Use semantic versioning (major.minor.patch)

### 2. Code Convention
```cpp
// Good function naming examples
{"calculate", math_calculate, "Calculate mathematical expression"},
{"format", string_format, "Format string with parameters"},
{"validate", data_validate, "Validate input data"}

// Naming to avoid
{"func1", some_function, "Does something"},
{"process", unclear_function, "Process data"}
```

### 3. Error Handling Strategy
- Always check argument count and types
- Return meaningful error messages
- Use consistent error format
- Avoid program crashes, handle exceptions gracefully

### 4. Memory Safety
- Use static buffers or global variables for returned strings
- Avoid returning pointers to local variables
- Use safe string functions (`snprintf`, `strncpy`)
- Ensure buffer sizes are sufficient to avoid overflows

### 5. Cross-Platform Compatibility
- Use the provided `LAMINA_EXPORT` and `LAMINA_CALL` macros
- Avoid platform-specific data types or functions
- Test on multiple platforms and compilers

## Troubleshooting

### Common Issues & Solutions

#### 1. Module Fails to Load
**Issue**: `ERROR: Module not loaded`
```cpp
// Checklist:
// - Is the DLL file in the correct location?
// - Does the filename match the include statement?
// - Are required dependencies present?
```

**Solutions**:
- Make sure the module file is in the `build/Debug/` directory
- Check file extension (Windows: `.dll`, Linux: `.so`)
- Use dependency tools to check for missing dynamic libraries

#### 2. Function Not Found
**Issue**: `ERROR: Function 'funcname' not found in module`
```cpp
// Checklist:
// - Is the function registered in the functions array?
// - Is the function name spelled correctly?
// - Does the namespace match?
```

**Solutions**:
- Verify function names in the `LaminaFunctionEntry` array
- Make sure correct namespace prefix is used when calling
- Check that `function_count` is correct

#### 3. Namespace Mismatch
**Issue**: `ERROR: Namespace mismatch`
```cpp
// In script: include "minimal.dll"; minimal.test();
// In module: namespace_name = "different_name"
```

**Solutions**:
- Make sure the module's `namespace_name` matches the prefix used when calling
- It's recommended to keep namespace name and filename consistent

#### 4. Build Errors
**Issue**: Linker errors or undefined symbols during compilation
```cpp
// Common causes:
// - Missing LAMINA_EXPORT macro
// - Calling convention mismatch
// - Incorrect header file path
```

**Solutions**:
- Ensure all exported functions use `LAMINA_EXPORT` and `LAMINA_CALL`
- Check the `module_api.hpp` include path
- Verify CMake configuration is correct

### Debugging Tips

#### 1. Add Debug Output
```cpp
LAMINA_EXPORT LaminaValue LAMINA_CALL debug_function(const LaminaValue* args, int argc) {
    #ifdef _DEBUG
    printf("DEBUG: Function called with %d arguments\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("DEBUG: Arg[%d] type=%d\n", i, args[i].type);
    }
    #endif
    
    // Actual function logic...
}
```

#### 2. Module Load Verification
```cpp
LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    printf("Module initializing: %s v%s\n", 
           exports.info.namespace_name, 
           exports.info.version);
    return &exports;
}
```

#### 3. Use System Tools
- **Windows**: Use `dumpbin /exports module.dll` to inspect exported symbols
- **Linux**: Use `nm -D module.so` or `objdump -T module.so`
- **Debugger**: Use GDB (Linux) or Visual Studio Debugger (Windows)

## Examples & References

### Complete Example Modules
See example modules in the project for full implementations:

#### minimal module (`extensions/minimal/`)
- **File**: `ultra_minimal.cpp`
- **Features**: Basic module example
- **Functions**: `test()`, `hello()`
- **Purpose**: Learn module development basics

#### standard module (`extensions/standard/`)
- **array.cpp**: Array operation functions
- **math.cpp**: Mathematical computation functions
- **stdio.cpp**: Input/output functions
- **times.cpp**: Time handling functions
- **random.cpp**: Random number generation
- **sockets.cpp**: Networking functionality

### Quickstart Template
Full workflow for creating a new module:

1. **Create Directory Structure**
```bash
mkdir extensions/mymodule
cd extensions/mymodule
```

2. **Create Source File (mymodule.cpp)**
```cpp
#include "../../interpreter/module_api.hpp"

LAMINA_EXPORT LaminaValue LAMINA_CALL my_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};
    result.type = LAMINA_TYPE_STRING;
    result.data.string_val = "Hello from my module!";
    return result;
}

static LaminaFunctionEntry functions[] = {
    {"greet", my_function, "Greet the user"}
};

static LaminaModuleExports exports = {
    {"mymodule", "1.0.0", "My custom module"},
    functions,
    1
};

LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    return &exports;
}
```

3. **Create Build File (CMakeLists.txt)**
```cmake
add_library(mymodule SHARED mymodule.cpp)
set_target_properties(mymodule PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    CXX_STANDARD 17
)
```

4. **Add to Main Build System**
Add to main `CMakeLists.txt`:
```cmake
add_subdirectory(extensions/mymodule)
```

5. **Build and Test**
```bash
cmake --build build --config Debug
cd build/Debug
./Lamina.exe
```

### API Quick Reference

#### Type Definitions
```cpp
LAMINA_TYPE_NULL     // Null value
LAMINA_TYPE_BOOL     // Boolean
LAMINA_TYPE_INT      // Integer
LAMINA_TYPE_NUMBER   // Float
LAMINA_TYPE_STRING   // String
```

#### Macro Definitions
```cpp
LAMINA_EXPORT        // Exported function declaration
LAMINA_CALL          // Calling convention
```

#### Required Function
```cpp
lamina_module_init() // Module initialization entry point
```

---
