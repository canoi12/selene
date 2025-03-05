newoption {
    trigger = 'project-dir',
    value = 'PATH',
    description = 'Set project directory to convert to emscripten .js'
}

if not _OPTIONS['project-dir'] then
    error("A project directory path must be given")
end

if not os.isdir(_OPTIONS['project-dir']) then
    error("Invalid project directory")
end

newoption {
    trigger = 'out-dir',
    value = 'PATH',
    description = 'Set output directory of the JS file',
    default = '.'
}

if not os.isdir(_OPTIONS['out-dir']) then
    error("Invalid output directory")
end

newaction {
    trigger = 'build',
    description = "build the project",
    execute     = function ()
        if not os.getenv('EMSDK') then
            error("please setup the EMSDK variable")
        end
        os.execute("$EMSDK/upstream/emscripten/tools/file_packager game --embed " .. _OPTIONS['project-dir'] .. "@./ >> " .. _OPTIONS['out-dir'] .. "/game.js")
    end
}