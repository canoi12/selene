local modules = os.matchdirs("*")
project "modules"
    kind "StaticLib"
    includedirs(modules)
    includedirs {"./"}
    files {"**.c", "**.h"}
    filter {"system:emscripten"}
        pic "On"
    filter {}

return modules