local class = require 'core.class'
local Drawable = class:extend('Drawable')

function Drawable:constructor(texture, width, height)
  self.texture = texture
  self.width = width
  self.height = height
end

function Drawable:get_uv(rect)
  local width = self.width
  local height = self.height
  rect = rect or { x = 0, y = 0, w = width, h = height }

  local uv = {}
  uv[1] = rect.x / width
  uv[2] = rect.y / height
  uv[3] = uv[1] + rect.w / width
  uv[4] = uv[2] + rect.h / height
  return uv
end

return Drawable
