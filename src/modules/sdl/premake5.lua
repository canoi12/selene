local lib = require('gen_defs.lib')
function generate_files()
    generate_lib('selene_sdl', lib)
end

newaction {
    trigger = 'generate',
    description = 'generate C and Lua files',
    execute = generate_files
}
