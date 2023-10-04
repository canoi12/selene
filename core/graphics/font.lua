local class = require 'core.class'
local filesystem = require 'core.filesystem'
local Font = class:extend("Font")

function Font:constructor(path)
  local file = filesystem.open(path)
  local data = file:to_data()

  self.path = args[1]
  self.image = nil
  self.rects = nil
  self.size = 16
end

return Font
