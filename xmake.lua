add_rules("mode.debug", "mode.release")
set_policy("package.include_external_headers", false)
add_requires("libuv 056e180e11c3a2ff7120f484da9d0f15a5776fd4")
add_requires("exprtk cc1b800c2bd1ac3ac260478c915d2aec6f4eb41c")
local function get_ver(os)
    local res={
            major=0,
            minor=0,
            patch=0,
            version="0.0.0-Unknown"
    }
    local out,err=os.iorun("git describe --tags --abbrev=0")
    if not err then
            return res
    end
    res["version"]=string.trim(out)
    res["major"],res["minor"],res["patch"]=string.match(res["version"],"v(%d+).(%d+).(%d+)")
    res["version"]=res["version"]:sub(2)
    return res
end
target("lamina_core")
    set_kind("shared")
    set_languages("c++20")
    add_files(
        "interpreter/**.cpp|main.cpp|repl_input.cpp|module.cpp|console_ui.cpp",
        "extensions/**.cpp"
    )
    add_includedirs("interpreter")
    add_headerfiles("interpreter/*.hpp")
    add_packages("libuv")
    add_packages("exprtk")
    add_defines(
        "LAMINA_CORE_EXPORTS",
        "USE_LIBUV"
    )
    if is_plat("windows") then
        add_links("imagehlp")
    end
    add_rules("utils.symbols.export_all")
    set_configdir("interpreter")
    add_configfiles("interpreter/*.in",{pattern="@(.-)@"})
    on_load(function (target)
        local ver=get_ver(os)
        target:set("configvar","PROJECT_VERSION_MAJOR",ver["major"])
        target:set("configvar","PROJECT_VERSION_MINOR",ver["minor"])
        target:set("configvar","PROJECT_VERSION_PATCH",ver["patch"])
        target:set("configvar","PROJECT_VERSION",ver["version"])
        target:set("configvar","HELP_TEXT",io.readfile("interpreter/resources/help.txt"))
    end)

target("lamina")
    set_kind("binary")
    set_languages("c++20")
    add_deps("lamina_core")
    add_files(
        "interpreter/main.cpp",
        "interpreter/repl_input.cpp",
        "interpreter/console_ui.cpp"
    )
    add_includedirs("interpreter")
