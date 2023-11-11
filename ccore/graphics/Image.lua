local gl = selene.gl
local filesystem = require 'core.filesystem'
local Drawable = require 'core.graphics.Drawable'
local Image = Drawable:extend('Image')

local formats = {
  red = gl.RED,
  rg = gl.RG,
  rgb = gl.RGB,
  rgba = gl.RGBA
}

local Image_mt = {
  __index = Image,
  __gc = function(s)
    s.texture:destroy()
  end
}

function Image.create(width, height, format, data)
  assert(width > 0, "Image width must be greater than zero")
  assert(height > 0, "Image height must be greater than zero")
  format = format or "rgba"
  assert(formats[format], "Invalid image format")
  local i = {}
  i.texture = gl.Texture.create()
  local f = formats[format]
  gl.Texture.bind(gl.TEXTURE_2D, i.texture)
  gl.Texture.Image2D(gl.TEXTURE_2D, f, width, height, f, gl.UNSIGNED_BYTE, data)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
  gl.Texture.bind(gl.TEXTURE_2D)
  return setmetatable(i, Image_mt)
end

function Image.load(path)
  assert(path, "Invalid image path")
  local data, width, height, c = selene.image.loadData(path)
  local f = gl.RGBA
  if c == 3 then
    f = gl.RGB
  end

  local i = {}
  i.texture = gl.Texture.create()
  local f = formats[format]
  gl.Texture.bind(gl.TEXTURE_2D, i.texture)
  gl.Texture.Image2D(gl.TEXTURE_2D, f, width, height, f, gl.UNSIGNED_BYTE, data)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
  gl.Texture.bind(gl.TEXTURE_2D)
  data:free()
  return setmetatable(i, Image_mt)
end

function Image:constructor(...)
  local args = { ... }
  local image_data = nil
  self.target = gl.TEXTURE_2D
  local texture = nil
  local width = 0
  local height = 0
  if type(args[1]) == "number" then
    texture = gl.Texture.New()
    width = args[1]
    height = args[2]

    gl.Texture.Bind(self.target, texture)
    gl.Texture.Image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, args[3])
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.Bind(self.target)
    self.width = width
    self.height = height
    self.comps = 4
    self.data = selene.Data.New(self.width * self.height * 4)
  elseif type(args[1]) == "string" then
    local data, w, h, c = selene.image.LoadData(filesystem.resolve(args[1]))
    texture = gl.Texture.New()
    width = w
    height = h
    self.comps = c
    self.data = data
    gl.Texture.Bind(self.target, texture)
    gl.Texture.Image2D(self.target, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, self.data)
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.Bind(self.target)
  end
  if not texture then error("Null texture") end
  if width <= 0 or height <= 0 then error("Invalid texture size") end
  Drawable.constructor(self, texture, width, height)
end

function Image:update(x, y, r, g, b, a)
  self.data:update(x, y, r, g, b, a)
  gl.Texture.Bind(self.target, self.texture)
  gl.Texture.Bind(self.target)
end

function Image:__gc()
  self.texture:Delete()
  self.data:Free()
end

return Image
