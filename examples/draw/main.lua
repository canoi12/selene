local graphics = require 'core.graphics'
local Image = require 'core.graphics.Image'

function selene.load()
  image = Image('sprites.png')
end

function selene.draw()
  graphics.clear()
  graphics.draw(image)
end
