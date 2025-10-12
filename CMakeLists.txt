cmake_minimum_required(VERSION 3.12.1)
project(Lamina VERSION 1.2.0 LANGUAGES CXX)# Lamina version here

include(GNUInstallDirs)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}/lamina)

add_subdirectory(interpreter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MD /Gz /FS /utf-8")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MD /Gz /FS")
        
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        
        add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        
        message(STATUS "Using MSVC compiler (${CMAKE_CXX_COMPILER_VERSION})")
    else()
        message(STATUS "Using MinGW/GCC compiler. Note: Some features may require additional configuration.")
        if(MINGW)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
        endif()
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter
    ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(interpreter)

add_library(lamina_core SHARED
    interpreter/lamina_api/ast.hpp
    interpreter/lamina_api/bigint.hpp
    interpreter/lamina_api/irrational.hpp
    interpreter/lamina_api/rational.hpp
    interpreter/lamina_api/value.hpp
    interpreter/lamina_api/lamina.hpp
    interpreter/lamina_api/symbolic.hpp
    interpreter/lamina_api/symbolic.cpp

    extensions/standard/math.cpp
    extensions/standard/basic.cpp
    extensions/standard/random.cpp
    extensions/standard/times.cpp
    extensions/standard/array.cpp
    extensions/standard/string.cpp
    extensions/standard/cas.hpp
    extensions/standard/cas.cpp
    extensions/standard/lmStruct.hpp
    extensions/standard/lmStruct.cpp
    extensions/standard/standard.hpp
    extensions/standard/io.cpp

    interpreter/eval.cpp
    interpreter/interpreter.cpp
    interpreter/interpreter.hpp
    interpreter/lexer.cpp
    interpreter/lexer.hpp
    interpreter/parser.cpp
    interpreter/parser.hpp
    interpreter/parse_expr.cpp
    interpreter/parse_factor.cpp
    interpreter/parse_stmt.cpp
    interpreter/cpp_module_loader.hpp

    interpreter/utils/src_manger.cpp
    interpreter/cpp_module_loader.hpp
    interpreter/utils/properties_parser.hpp
    interpreter/utils/src_manger.hpp
)

target_sources(lamina_core PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)

set_target_properties(lamina_core PROPERTIES 
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    ENABLE_EXPORTS ON
    PREFIX ""
)

target_compile_definitions(lamina_core PRIVATE LAMINA_CORE_EXPORTS)

if(WIN32)
    target_link_libraries(lamina_core PRIVATE imagehlp)
endif()

if(MSVC)
    target_compile_options(lamina_core PRIVATE
        /bigobj
        /W4
        /FS
        /wd4251
        /wd4267
        /wd4996
        /wd4702
        /wd4100
        /wd4189
        /wd4459
        /wd4244
        /wd4005
    )
else()
    target_compile_options(lamina_core PRIVATE 
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
    )
endif()

add_executable(Lamina 
    interpreter/main.cpp 
    interpreter/utils/repl_input.hpp
    interpreter/utils/repl_input.cpp
    interpreter/console_ui.hpp
    interpreter/console_ui.cpp
    interpreter/utils/color_style.hpp
    interpreter/utils/color_style.cpp
    interpreter/eval.cpp
)

target_sources(Lamina PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)

target_link_libraries(Lamina PRIVATE lamina_core)

if(UNIX)
    target_link_libraries(Lamina PRIVATE dl)
endif()

if(WIN32)
    add_custom_command(TARGET Lamina POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:lamina_core>
            $<TARGET_FILE_DIR:Lamina>
        COMMENT "Copying lamina_core.dll to output directory"
    )
endif()

message(STATUS "Building extensions for ${CMAKE_SYSTEM_NAME}")

function(build_extension EXTENSION_PATH)
    get_filename_component(EXTENSION_NAME ${EXTENSION_PATH} NAME)
    get_filename_component(EXTENSION_DIR ${EXTENSION_PATH} DIRECTORY)
    
    set(EXTENSION_SOURCE "${EXTENSION_PATH}/${EXTENSION_NAME}.cpp")
    if(EXISTS ${EXTENSION_SOURCE})
        message(STATUS "Building extension: ${EXTENSION_NAME}")
        
        add_library(${EXTENSION_NAME} SHARED ${EXTENSION_SOURCE})
        
        target_include_directories(${EXTENSION_NAME} PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/interpreter
            ${CMAKE_CURRENT_SOURCE_DIR}/extensions
            ${EXTENSION_PATH}
        )
        
        set_target_properties(${EXTENSION_NAME} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
            LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
            POSITION_INDEPENDENT_CODE ON
            PREFIX ""
        )
        
        if(MSVC)
            target_compile_options(${EXTENSION_NAME} PRIVATE 
                /W4
                /wd4251
                /wd4267
                /wd4996
                /wd4702
                /wd4100
                /wd4189
                /wd4459
                /wd4244
                /wd4005
            )
        else()
            target_compile_options(${EXTENSION_NAME} PRIVATE 
                -Wall -Wextra -Wpedantic
                -Wno-unused-parameter
                -Wno-unused-variable
            )
        endif()
        
        if(WIN32)
            if(MSVC)
                add_custom_command(TARGET ${EXTENSION_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_FILE:${EXTENSION_NAME}>
                        $<TARGET_FILE_DIR:${EXTENSION_NAME}>
                    COMMENT "Copying ${EXTENSION_NAME}.dll to output directory"
                )
                if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                    add_custom_command(TARGET ${EXTENSION_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            $<TARGET_PDB_FILE:${EXTENSION_NAME}>
                            $<TARGET_FILE_DIR:${EXTENSION_NAME}>
                        COMMENT "Copying ${EXTENSION_NAME}.pdb to output directory"
                    )
                endif()
            else()
                add_custom_command(TARGET ${EXTENSION_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_FILE:${EXTENSION_NAME}>
                        $<TARGET_FILE_DIR:${EXTENSION_NAME}>
                    COMMENT "Copying ${EXTENSION_NAME}.dll to output directory"
                )
            endif()
        endif()
    else()
        message(STATUS "Skipping extension ${EXTENSION_NAME}: source file not found (${EXTENSION_SOURCE})")
    endif()
endfunction()

file(GLOB EXTENSION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/extensions/*")
set(FOUND_EXTENSIONS 0)
foreach(EXTENSION_DIR ${EXTENSION_DIRS})
    if(IS_DIRECTORY ${EXTENSION_DIR})
        get_filename_component(DIR_NAME ${EXTENSION_DIR} NAME)
        if(NOT DIR_NAME STREQUAL "standard")
            get_filename_component(EXTENSION_NAME ${EXTENSION_DIR} NAME)
            set(EXTENSION_SOURCE "${EXTENSION_DIR}/${EXTENSION_NAME}.cpp")
            if(EXISTS ${EXTENSION_SOURCE})
                build_extension(${EXTENSION_DIR})
                math(EXPR FOUND_EXTENSIONS "${FOUND_EXTENSIONS} + 1")
            endif()
        endif()
    endif()
endforeach()

if(FOUND_EXTENSIONS EQUAL 0)
    message(STATUS "No additional extensions found to build")
else()
    message(STATUS "Found and configured ${FOUND_EXTENSIONS} extension(s)")
endif()

set_target_properties(Lamina PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    BUILD_WITH_INSTALL_RPATH True
)

install(TARGETS Lamina lamina_core
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY interpreter/
    DESTINATION include/lamina
    FILES_MATCHING PATTERN "*.hpp"
)
