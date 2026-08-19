option("launch_game")
    set_default(false)
    set_showmenu(true)
    set_description("Launch the game executable after build")
option_end()

option("game_path")
    set_default("D:/FAForever/bin")
    set_showmenu(true)
    set_description("Path to the game directory containing ForgedAlliance.exe")
option_end()

set_languages("cxx23")
set_policy("build.c++.modules", true)
add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    add_defines("DEBUG", "_DEBUG")
end
add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")

set_policy("build.warning", true)
set_warnings("all", "error")

set_allowedplats("windows")
set_allowedarchs("x86")

set_defaultplat("windows")
set_defaultarchs("x86")

package("rcmp")
    set_homepage("https://github.com/Smertig/rcmp")
    set_description("C++17, multi-architecture cross-platform hooking library with clean API.")
    set_license("MIT")

    add_urls("https://github.com/Smertig/rcmp/archive/refs/tags/$(version).tar.gz",
             "https://github.com/Smertig/rcmp.git")

    add_versions("v0.2.2", "accbf1d2c72b169857900ce816ca3c1718c63c9f67ded413613c236455a331d5")

    on_install(function (package)
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")
            target("rcmp")
                set_kind("static")
                set_languages("cxx17")
                add_defines("RCMP_VERSION_MAJOR=0", "RCMP_VERSION_MINOR=2", "RCMP_VERSION_PATCH=2")
                add_includedirs("include", "external/nmd/include")
                add_headerfiles("include/(**.hpp)", "include/(**.h)", "external/nmd/include/(**.h)")
                add_files(
                    "source/codegen.cpp",
                    "source/memory.cpp",
                    "source/detail/arch/impl.cpp",
                    "source/detail/platform/impl.cpp"
                )
        ]])
        package:add("defines", "RCMP_VERSION_MAJOR=0", "RCMP_VERSION_MINOR=2", "RCMP_VERSION_PATCH=2")
        import("package.tools.xmake").install(package)
    end)

    on_test(function (package)
        assert(package:has_cxxincludes("rcmp.hpp", {configs = {languages = "c++17"}}))
    end)
package_end()

add_requires("rcmp")
add_requires("xbyak v7.40", "nlohmann_json develop")
add_requires("imgui", {configs = {win32 = true, dx9 = true, no_obsolete_functions = true}})

target("famod")
    set_kind("shared")
    set_filename("dsound.dll")

    add_files("src/*.cpp")
    add_files("src/**.cppm")
    add_files("src/thirdparty/dsound_proxy/dsound_proxy.cpp", "src/thirdparty/dsound_proxy/dsound_proxy.def")

    add_includedirs("include")
    add_packages("xbyak", "nlohmann_json", "imgui", "rcmp")

    if get_config("toolchain") == "mingw" then
        add_shflags("src/thirdparty/dsound_proxy/dsound_proxy.def", {force = true})
        add_shflags("-Wl,--kill-at")
        add_syslinks("user32", "gdi32", "dwmapi", "d3d9", "imm32", "stdc++exp")
    end

    if has_config("launch_game") then
        after_build(function (target)
            local game_path = get_config("game_path")
            if game_path and os.isdir(game_path) then
                os.cp(target:targetfile(), game_path)
                os.cd(game_path)
                os.execv("ForgedAlliance.exe", {"/init", "init.lua", "/nomovie"})
            end
        end)
    end
