project ("lua")
    language "C"
    files {_LUA_DIR .. "/src/*.c", _LUA_DIR .. "/src/*.h"}
    removefiles {_LUA_DIR .. "/src/lua.*", _LUA_DIR .. "/src/luac.*"}
    kind "StaticLib"
    filter {"options:lua-shared"}
        kind "SharedLib"

    filter {"system:emscripten"}
        pic "On"

    filter "system:linux"
        defines {"LUA_USE_LINUX"}
    filter {"kind:SharedLib", "system:linux or android"}
        pic "On"
    filter {"kind:SharedLib", "system:windows"}
        defines {"LUA_BUILD_AS_DLL"}
    filter {"kind:SharedLib", "system:emscripten"}
        linkoptions {"-sSIDE_MODULE"}
    filter {}