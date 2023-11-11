local gl = selene.gl
local Rect = require 'core.Rect'
local record Canvas
    handle: gl.Framebuffer | nil
    texture: gl.Texture | nil
    width: integer
    height: integer
    channels: integer

    create: function(integer, integer) : Canvas
    default: function() : Canvas
    
    metamethod __index: Canvas
    metamethod __gc: function(Canvas)

    getTexture: function(Canvas) : gl.Texture
    getUV: function(Canvas, Rect | nil) : {number}
end

function Canvas.create(width: integer, height: integer) : Canvas
    local canvas : Canvas = {}

    canvas.width = width
    canvas.height = height
    canvas.channels = 4
    canvas.texture = gl.Texture.create()
    gl.Texture.bind(gl.TEXTURE_2D, canvas.texture)
    gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.bind(gl.TEXTURE_2D)

    canvas.handle = gl.Framebuffer.create()
    gl.Framebuffer.bind(gl.FRAMEBUFFER, canvas.handle)
    gl.Framebuffer.attachTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, canvas.texture)
    gl.Framebuffer.bind(gl.FRAMEBUFFER)

    return setmetatable(canvas, {
        __index = Canvas,
        __gc = function(c: Canvas)
            local fbo = c.handle as gl.Framebuffer
            local tex = c.texture as gl.Texture
            fbo:destroy()
            tex:destroy()
        end
    })
end

function Canvas.default() : Canvas

end

function Canvas:getTexture() : gl.Texture
    return self.texture
end

function Canvas:getUV(rect: Rect | nil) : {number}
    local width = self.width
    local height = self.height

    rect = (rect or { x = 0, y = 0, w = width, h = height }) as Rect

    local uv = {}
    local uv_y = rect.y / height
    uv[1] = rect.x / width
    uv[2] = 1 - uv_y
    uv[3] = uv[1] + rect.w / width
    uv[4] = 1 - (uv_y + rect.h / height)
    return uv
end

return Canvas