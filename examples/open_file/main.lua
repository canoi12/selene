local graphics = require 'core.graphics'
local filesystem = require 'core.filesystem'

function selene.load()
    selene.fs.File.__gc = function(f)
        f:Close()
    end
    file = filesystem.open('file.txt')
    data = file:Read()

    str = data:GetString()
    print(str)
end

function selene.draw()
    graphics.clear()
    graphics.print(str, 8, 8)
end
