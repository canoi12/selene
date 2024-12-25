newoption {
    trigger = 'lib',
    value = 'NAME',
    description = 'generate files of the selected lib',
    allowed = {
        {'sdl2', 'SDL2'},
        {'gl', 'OpenGL'}
    }
}

newoption {
    trigger = 'single',
    description = 'generate single file version of the library',
}


if not _OPTIONS['lib'] then
    error("You must choose a lib to generate the files")
end


local lib = require('generator.' .. _OPTIONS['lib'] .. '.lib')
function generate_files()
    if _OPTIONS['single'] then
        io.writefile(_MAIN_SCRIPT_DIR .. '/' .. _OPTIONS['lib'] .. '.c', generate_single(lib))
        io.writefile(_MAIN_SCRIPT_DIR .. '/' .. _OPTIONS['lib'] .. '.lua', generate_lua_defs(lib))
    else
        generate_lib(_OPTIONS['lib'], lib)
    end
end

local exec = generate_files

newaction {
    trigger = 'generate',
    description = 'generate C and Lua files',
    execute = generate_files
}
