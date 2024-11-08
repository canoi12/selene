function build_sdl()
    if _OPTIONS['sdl-dir'] and not _OPTIONS['no-sdl'] then
        local sdl_dir = _OPTIONS['sdl-dir']
        if not path.isabsolute(sdl_dir) then
            sdl_dir = _WORKING_DIR .. '/' .. sdl_dir
        end
        includedirs {sdl_dir .. '/include'}
    elseif not _OPTIONS['no-sdl'] and not _OPTIONS['emscripten'] then
        buildoptions {"`sdl2-config --cflags`"}
    end

    filter {"options:no-sdl"}
        defines {"SELENE_NO_SDL"}
end

function link_sdl()
    if _OPTIONS['sdl-dir'] then
        local sdl_dir = _OPTIONS['sdl-dir']
        if not path.isabsolute(sdl_dir) then
            sdl_dir = _MAIN_SCRIPT_DIR .. '/' .. sdl_dir
        end
        filter {"platforms:win32"}
            libdirs {sdl_dir .. "/lib/x86"}
        filter {"platforms:win64"}
            libdirs {sdl_dir .. "/lib/x64"}
    
        if _ACTION == 'androidmk' then
            filter {"system:android"}
                amk_includes {_OPTIONS['sdl-dir'] .. '/Android.mk'}
                amk_sharedlinks {"SDL2"}
        end
    else
        filter {"system:not emscripten", "action:gmake2"}
            -- buildoptions {"`sdl2-config --cflags`"}
            linkoptions {"`sdl2-config --libs`"}
    end
    
    filter {"not options:no-sdl", "system:emscripten"}
        linkoptions {"-sUSE_SDL=2"}
    filter {"not options:no-sdl", "system:not emscripten", "system:not android"}
        links {"SDL2"}
    filter {}
end

newoption {
    trigger = 'out-dir',
    value = 'PATH',
    description = 'Set workspace location directory',
    default = 'build/'
}

newoption {
    trigger = 'lua-version',
    value = 'VERSION',
    description = 'Set used Lua version',
    default = '5.4',
    allowed = {
        {'5.2', 'Lua 5.2'},
        {'5.3', 'Lua 5.3'},
        {'5.4', 'Lua 5.4'},
        {'jit', 'LuaJIT'},
    }
}

newoption {
    trigger = 'lua-dir',
    value = 'PATH',
    description = 'Set the path to the Lua directory to use in compilation',
    default = 'lua/lua' .. _OPTIONS['lua-version']
}

newoption {
    trigger = 'lua-shared',
    description = 'Set if you want to compile Lua as a shared library, otherwise it will compile as a static library',
}

newoption {
    trigger = 'no-sdl',
    description = 'Disable the SDL2 compilation from the code'
}

newoption {
    trigger = 'plugins',
    value = 'LIBTYPE',
    description = 'If set, the plugins project will be added',
    allowed = {
        {'static', 'Static Library'},
        {'shared', 'Shared Library'}
    }
}

newoption {
    trigger = 'emscripten',
    description = 'Setup project to compile for emscripten (needs emscripten module for premake)'
}

newoption {
    trigger = 'android',
    description = 'Setup project to compile for android (needs my fork of androidmk premake module: https://github.com/canoi12/premake-androidmk)'
}

newoption {
    trigger = 'sdl-dir',
    value = 'PATH',
    description = 'Set the SDL2 directory path'
}

newoption {
    trigger = 'shared-lib',
    description = 'Compile the project as a shared library'
}

newoption {
    trigger = 'static-lib',
    description = 'Compile the project as a static library'
}

newoption {
    trigger = 'ecc',
    description = 'Generate a compile_commands.json'
}

newaction {
    trigger = 'clean',
    description = "clean the build folder",
    execute     = function ()
        print("clean the build...")
        os.rmdir(_OPTIONS['out-dir'])
        print("done.")
    end
}

if not os.isdir(_OPTIONS['lua-dir']) then
    error("invalid Lua dir path")
end

if not path.isabsolute(_OPTIONS['lua-dir']) then
    _LUA_DIR = _WORKING_DIR .. '/' .. _OPTIONS['lua-dir']
else
    _LUA_DIR = _OPTIONS['lua-dir']
end

if not path.isabsolute(_OPTIONS['out-dir']) then
    _OUT_DIR = _WORKING_DIR .. '/' .. _OPTIONS['out-dir']
else
    _OUT_DIR = _OPTIONS['out-dir']
end

if _OPTIONS['android'] then
    _OUT_DIR = _OUT_DIR .. '/app/jni'
    require 'cross.android'
    _ACTION = 'androidmk'

    if not os.isdir(_OUT_DIR) then
        os.mkdir(_OUT_DIR)
    end

    local status, err = os.copyfile(_WORKING_DIR .. '/cross/android/Application.mk', _WORKING_DIR .. '/build/app/jni/Application.mk')
    local function copy_dir(dir)

    end
    if not status then
        error(err)
    end
end

if _OPTIONS['emscripten'] then
    require 'cross.emscripten'
    _ACTION = 'gmake2'
end

if _OPTIONS['sdl-dir'] then
    if not os.isdir(_OPTIONS['sdl-dir']) then
        error("Invalid SDL directory path")
    end
end

if _OPTIONS['ecc'] then
    require('ecc')
    _ACTION = 'ecc'
end

_BIN_DIR = '%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}'
_PLUGINS_DIR = _MAIN_SCRIPT_DIR
_PLUGINS_BLOCKLIST = {'core'}

require('generator')
