local gl = selene.gl
local record Font
    texture: gl.Texture
    width: integer
    height: integer
    size: integer
    rects: {selene.font.Glyph}

    load: function(string, integer) : Font

    default: function() : Font
end

function Font.load(path: string, size: integer) : Font
    local f : Font = {}

    local data, width, height, rects = selene.font.loadTTF(path, size)

    f.texture = gl.Texture.create()
    gl.Texture.bind(gl.TEXTURE_2D, f.texture)
    gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, data)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.bind(gl.TEXTURE_2D)
    data:free()

    f.size = size
    f.rects = rects
    f.width = width
    f.height = height

    return setmetatable(f, {
        __index = Font,
        __gc = function(o: Font)
            o.texture:destroy()
        end
    })
end


function Font.default() : Font
    local f : Font = {}
    local data, width, height, rects = selene.font.getDefault()
    f.texture = gl.Texture.create()
    gl.Texture.bind(gl.TEXTURE_2D, f.texture)
    gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, data)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.bind(gl.TEXTURE_2D)
    data:free()

    f.size = 8
    f.rects = rects
    f.width = width
    f.height = height

    return setmetatable(f, {
        __index = Font,
        __gc = function(o: Font)
            o.texture:destroy()
        end
    })
end

return Font