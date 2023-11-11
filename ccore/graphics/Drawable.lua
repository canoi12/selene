local class = require 'core.class'
local gl = selene.gl
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

function Drawable:get_size()
  return self.width, self.height
end

function Drawable:get_filter()
  return self.filter
end

local filters = {
  nearest = gl.NEAREST,
  linear = gl.LINEAR
}

function Drawable:set_filter(filter)
  local f = filters[filter]
  if not f then
    error('Invalid filter')
  end
  self.filter = filter
  gl.Texture.Bind(gl.TEXTURE_2D, obj.texture)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, f)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, f)
  gl.Texture.Bind(gl.TEXTURE_2D)
end

local wraps = {
  ['repeat'] = gl.REPEAT,
  ['clamp'] = gl.CLAMP_TO_BORDER
}

function Drawable:set_wrap(wrap)
  local w = wraps[wrap]
  if not w then
    error('Invalid wrap')
  end
  gl.Texture.Bind(gl.TEXTURE_2D, obj.texture)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, w)
  gl.Texture.Parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, w)
  gl.Texture.Bind(gl.TEXTURE_2D)
end

function Drawable:__gc()
  self.texture:Delete()
end

return Drawable
