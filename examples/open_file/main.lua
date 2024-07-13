local core = require('core').init()

function core.onStart()
    selene.fs.File.__gc = function(f)
        f:close()
    end
    file = core.projectFs:open('file.txt')
    data = file:read()

    str = data:getString()
    print(str)
end

function core.onRender(r)
    r:clear()
    r:print(str, 8, 8)
end

return core.default()
