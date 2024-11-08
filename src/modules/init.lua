local modules = os.matchdirs("*")
project "modules"
    kind "StaticLib"
    includedirs(modules)
    includedirs {"./"}
    files {"**.c", "**.h"}
    filter {"system:emscripten"}
        pic "On"
    filter {}
-- if _OPTIONS['sdl-dir'] then
--     local sdl_dir = _OPTIONS['sdl-dir']
--     if not path.isabsolute(sdl_dir) then
--         sdl_dir = _WORKING_DIR .. '/' .. sdl_dir
--     end
--     includedirs {sdl_dir .. '/include'}
--     if _ACTION == 'androidmk' then
--         filter {"system:android"}
--             amk_includes {sdl_dir .. '/Android.mk'}
--     end
-- else
--     filter {"system:not emscripten", "action:gmake2"}
--         buildoptions {"`sdl2-config --cflags`"}
-- end

    -- filter {"not options:no-sdl", "system:emscripten"}
    --     linkoptions {"-sUSE_SDL=2"}
    -- filter {}


return modules