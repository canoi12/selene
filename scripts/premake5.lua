newoption {
    trigger = 'plugins-dir',
    description = 'Set the path of the plugins directory',
    default = 'plugins/'
}

if not path.isabsolute(_OPTIONS['plugins-dir']) then
    _PLUGINS_DIR = _WORKING_DIR .. '/' .. _OPTIONS['plugins-dir']
else
    _PLUGINS_DIR = _OPTIONS['plugins-dir']
end

if _OPTIONS['shared-lib'] then
    print(_OPTIONS['shared-lib'])
    _OPTIONS['lua-shared'] = ''
end

workspace "selene"
    language "C"
    configurations {"Debug", "Release"}

    filter {"action:vs*"}
        platforms {"win32", "win64"}
        system "windows"
    filter {"action:gmake*"}
        platforms {"linux", "win32", "win64"}
    filter {"action:androidmk"}
        platforms {"android"}
        system "android"
        defines {"__ANDROID__"}
    filter {}
        
    location(_OUT_DIR)
    includedirs {"include", _LUA_DIR .. "/src", "src", "src/third"}
    includedirs {'src/third/glad/include'}

    if COMPILE_WEB then
        platforms {"emscripten"}
    end

    build_sdl()

    filter {"action:gmake*", "platforms:win32 or win64"}
        system "windows"

    filter {'configurations:Debug'}
        symbols "On"
        defines {'DEBUG'}
        
    filter {'configurations:Release'}
        defines {'NDEBUG'}
        optimize "On"

    filter {"options:plugins=*"}
        defines {'SELENE_PLUGINS_PRELOAD'}
    
    filter {"system:android"}
    if _ACTION == 'androidmk' then
        ndkabi "x86 x86_64 arm64-v8a"
        ndkplatform "android-26"
    end

    filter {"platforms:linux"}
        system "linux"

if (COMPILE_WEB) then
    filter {"platforms:emscripten"}
        system "emscripten"
        buildoptions {"-sUSE_SDL=2"}
        linkoptions {"-s WASM=1"}
        defines {"__EMSCRIPTEN__"}
end
    filter {}

-- require('src.modules')
project("selene")
    filter {"system:android"}
        files {"src/platforms/android.c"}
    filter {}
    if _OPTIONS['static-lib'] then
        kind "StaticLib"
    elseif _OPTIONS['shared-lib'] then
        kind "SharedLib"
    else
        kind "ConsoleApp"
    end

    language "C"
    targetdir(_BIN_DIR)
    includedirs {"include/", "./", "plugins/", "src/"}
    -- links {"modules", "lua"}
    links {"lua"}
    files {"src/selene.c", "src/Data.c", "src/modules/*.c", "include/*.h", 'src/third/glad/src/glad.c'}

    if _OPTIONS['plugins'] then
        links {"selene-plugins"}
    end

    filter {"kind:ConsoleApp"}
        files {"src/main.c"}
    filter {}

    link_sdl()

    filter "system:android"
        links { "android", "GLESv1_CM", "GLESv2", "log" }
    filter "system:linux"
        links {"m", "dl", "GL"}
    filter "system:windows"
        links {"opengl32", "glu32"}
    filter {"system:emscripten"}
        pic "On"
        -- files {"index.html"}
        targetextension ".html"
        links {"m"}
        linkoptions {"-sALLOW_MEMORY_GROWTH", "-sFORCE_FILESYSTEM", "--shell-file %{_WORKING_DIR}/cross/emscripten/index.html"}
    filter {"system:emscripten"}
        linkoptions {"-sMAIN_MODULE"}
    filter {}


require('lua_proj')

if _OPTIONS['plugins'] then
    include(_PLUGINS_DIR .. '/project.lua')
end

-- if _OPTIONS['plugins'] then
--     local path = _WORKING_DIR .. '/' .. _OPTIONS['plugins-dir']
--     package.path = path .. '/?.lua;' .. path .. '/?/init.lua;' .. package.path
--     require('options')
--     require('plugins_loader')(proj, path)

--     project "selene"
--         files {path .. '/plugins.c', path .. 'plugins.h'}
-- end
