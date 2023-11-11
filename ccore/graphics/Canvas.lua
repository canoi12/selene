local gl = selene.gl
local Drawable = require 'core.graphics.Drawable'
local Canvas = Drawable:extend('Canvas')

local Canvas = {}
local Canvas_mt = {
  __index = Canvas,
  __gc = function(c)
    c.texture:destroy()
    c.framebuffer:destroy()
  end
}

function Canvas.create(width, height)
  assert(width > 0, "Canvas width should be greater than zero")
  assert(height > 0, "Canvas height should be greater than zero")
  local c = {}
  local texture = gl.Texture.create()
  c.texture = texture
  gl.Texture.bind(gl.TEXTURE_2D, texture)
  gl.Texture.Image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
  gl.Texture.bind(gl.TEXTURE_2D)

  local framebuffer = gl.Framebuffer.create()
  gl.Framebuffer.bind(gl.FRAMEBUFFER, framebuffer)
  gl.Framebuffer.AttachTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture)
  gl.Framebuffer.bind(gl.FRAMEBUFFER)

  c.framebuffer = framebuffer
  c.width = width
  c.height = height

  return setmetatable(c, Canvas_mt)
end

function Canvas:getUV(rect)
  local width = self.width
  local height = self.height
  rect = rect or { x = 0, y = 0, w = width, h = height }

  local uv = {}
  local uv_y = rect.y / height
  uv[1] = rect.x / width
  uv[2] = 1 - uv_y
  uv[3] = uv[1] + rect.w / width
  uv[4] = 1 - (uv_y + rect.h / height)
  return uv
end

function Canvas:constructor(width, height)
  if width <= 0 or height <= 0 then error("Invalid canvas size") end
  Drawable.constructor(self, gl.Texture.New(), width, height)
  gl.Texture.Bind(gl.TEXTURE_2D, self.texture)
  gl.Texture.Image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
  gl.Texture.Bind(gl.TEXTURE_2D)

  self.target = gl.FRAMEBUFFER
  self.framebuffer = gl.Framebuffer.New()
  gl.Framebuffer.Bind(gl.FRAMEBUFFER, self.framebuffer)
  gl.Framebuffer.AttachTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, self.texture)
  gl.Framebuffer.Bind(gl.FRAMEBUFFER)
end

function Canvas:get_uv(rect)
  local width = self.width
  local height = self.height
  rect = rect or { x = 0, y = 0, w = width, h = height }

  local uv = {}
  local uv_y = rect.y / height
  uv[1] = rect.x / width
  uv[2] = 1 - uv_y
  uv[3] = uv[1] + rect.w / width
  uv[4] = 1 - (uv_y + rect.h / height)
  return uv
end

function Canvas:__gc()
  self.texture:Delete()
  self.framebuffer:Delete()
end

return Canvas
