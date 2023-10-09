local gl = selene.gl
local filesystem = require 'core.filesystem'
local Drawable = require 'core.graphics.Drawable'
local Image = Drawable:extend('Image')

function Image:constructor(...)
  local args = { ... }
  local image_data = nil
  self.target = gl.TEXTURE_2D
  local texture = nil
  local width = 0
  local height = 0
  if type(args[1]) == "number" then
    texture = gl.NewTexture()
    width = args[1]
    height = args[2]

    gl.BindTexture(self.target, texture)
    gl.TexImage2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, args[3])
    gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.BindTexture(self.target)
    self.width = width
    self.height = height
    self.comps = 4
    self.data = selene.utils.NewData(self.width * self.height * 4)
  elseif type(args[1]) == "string" then
    local data, w, h, c = selene.utils.LoadImageData(filesystem.resolve(args[1]))
    texture = gl.NewTexture()
    width = w
    height = h
    self.comps = c
    self.data = data
    gl.BindTexture(self.target, texture)
    gl.TexImage2D(self.target, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, self.data)
    gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.BindTexture(self.target)
  end
  if not texture then error("Null texture") end
  if width <= 0 or height <= 0 then error("Invalid texture size") end
  Drawable.constructor(self, texture, width, height)
end

function Image:update(x, y, r, g, b, a)
  self.data:update(x, y, r, g, b, a)
  gl.BindTexture(self.target, self.texture)
  gl.BindTexture(self.target)
end

return Image
