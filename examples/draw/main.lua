local graphics = require 'core.graphics'
local Image = require 'core.graphics.Image'

function selene.load()
  image = Image('sprites.png')
  local data = selene.stdc.malloc(10)
  print(data, tonumber(data), tolightuserdata(0x90))
  selene.stdc.free(data)
end

function selene.draw()
  graphics.clear()
  graphics.draw(image)
end
