local gl = poti.gl
local render = require 'core.render'
local Drawable = require 'core.graphics.drawable'
local SpriteBatch = Drawable:extend('SpriteBatch')

function SpriteBatch:constructor(drawable, size)
  self.drawable = drawable
  self.size = size
end

function SpriteBatch:push()
end

return SpriteBatch
