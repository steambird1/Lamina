# Generate version header automatically
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
    ${CMAKE_BINARY_DIR}/version.hpp
    @ONLY
)

# Read help file and parse it to C++ string
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/resources/help.txt HELP_TEXT)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/help_text.hpp.in
    ${CMAKE_BINARY_DIR}/help_text.hpp
    @ONLY
)
