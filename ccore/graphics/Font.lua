local Image = require 'core.graphics.Image'
local gl = selene.gl
local Font = {}

local Font_mt = {
  __index = Font,
  __gc = function(f)
    f.texture:destroy()
  end
}

function Font.loadTTF(path, size)
  local f = {}
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
  f.channels = 4
  
  return setmetatable(f, Font_mt)
end

function Font.default()
  local f = {}
  local data, width, height, rects = selene.font.getDefault()

  f.texture = gl.Texture.create()
  gl.Texture.bind(gl.TEXTURE_2D, f.texture)
  gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, data)
  gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
  gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
  gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
  gl.Texture.bind(gl.TEXTURE_2D)

  f.size = 8
  f.rects = rects
  f.width = width
  f.height = height
  f.channels = 4

  return setmetatable(f, Font_mt)
end

return Font
