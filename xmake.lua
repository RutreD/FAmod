option("launch_game")
    set_default(false)
    set_showmenu(true)
    set_description("Launch the game executable after build")
option_end()

option("game_path")
    set_default("C:/ProgramData/FAForever/bin")
    set_showmenu(true)
    set_description("Path to the game directory containing ForgedAlliance.exe")
option_end()

set_languages("cxx23")
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

add_requires("xbyak v7.40")

target("replay_desync")
    set_kind("shared")
    set_filename("dsound.dll")

    add_files("src/*.cpp")
    add_files("src/dsound_proxy/dsound_proxy.cpp", "src/dsound_proxy/dsound_proxy.def")

    add_packages("xbyak")

    if get_config("toolchain") == "mingw" then
        add_cxflags("-mincoming-stack-boundary=2", "-mstackrealign")
        add_shflags("src/dsound_proxy/dsound_proxy.def", {force = true})
        add_shflags("-Wl,--kill-at")
        add_syslinks("user32", "gdi32")
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
