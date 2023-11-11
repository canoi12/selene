local _tl_compat; if (tonumber((_VERSION or ''):match('[%d.]*$')) or 0) < 5.3 then local p, m = pcall(require, 'compat53.module'); if p then _tl_compat = m end end; local assert = _tl_compat and _tl_compat.assert or assert; local gl = selene.gl
local Rect = require('core.Rect')
ImageFilter = {}




local Image = {}




















function Image.create(width, height, channels, data)
   local img = {}
   local texture = gl.Texture.create()
   channels = channels or 4
   assert(channels > 0 and channels < 5)
   img.handle = texture
   img.width = width
   img.height = height
   img.channels = channels

   gl.Texture.bind(gl.TEXTURE_2D, texture)
   gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, data)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
   gl.Texture.bind(gl.TEXTURE_2D)

   return setmetatable(img, { __index = Image, __gc = function(i) i.handle:destroy() end })
end

function Image.load(path)
   local img = {}
   local data, w, h, c = selene.image.loadData(path)
   local texture = gl.Texture.create()
   img.handle = texture
   img.width = w
   img.height = h
   img.channels = c
   gl.Texture.bind(gl.TEXTURE_2D, texture)
   gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, w, h, gl.RGBA, gl.UNSIGNED_BYTE, data)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
   gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
   gl.Texture.bind(gl.TEXTURE_2D)
   data:free()
   return setmetatable(img, { __index = Image, __gc = function(i) i.handle:destroy() end })
end

function Image:getUV(rect)
   local width = self.width
   local height = self.height

   rect = (rect or { x = 0, y = 0, w = width, h = height })

   local uv = {}
   local uv_y = rect.y / height
   uv[1] = rect.x / width
   uv[2] = uv_y
   uv[3] = uv[1] + rect.w / width
   uv[4] = uv_y + rect.h / height
   return uv
end

return Image
