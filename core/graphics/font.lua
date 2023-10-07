local class = require 'core.class'
local Image = require 'core.graphics.image'
local gl = selene.gl
local filesystem = require 'core.filesystem'
local Font = class:extend("Font")

function Font:constructor(path, size)
  local data, width, height, rects = selene.utils.LoadTTF(filesystem.resolve(path), size)
  
  self.image = Image:new(width, height, data)
  self.rects = rects
  self.size = size
end

return Font
