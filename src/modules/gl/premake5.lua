local lib = require('gen_defs.lib')
function generate_files()
    generate_lib('lgl', lib)
end

newaction {
    trigger = 'generate',
    description = 'generate C and Lua files',
    execute = generate_files
}
