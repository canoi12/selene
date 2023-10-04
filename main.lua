local graphics = require 'core.graphics'
local Canvas = require 'core.graphics.canvas'
local keyboard = require 'core.keyboard'
local Image = require 'core.graphics.image'

function selene.load()
  canvas = Canvas:new(160, 95)
  image = Image:new("sprites.png")
end

x = 0
y = 0

function selene.update(dt)
  if keyboard.is_down('a') then
    x = x - (80 * dt)
  elseif keyboard.is_down('d') then
    x = x + (80 * dt)
  end
end

function selene.draw()
  graphics.clear()

  graphics.set_canvas(canvas)
  graphics.draw(image)
  graphics.set_canvas()

  graphics.draw(canvas)

  graphics.draw_rectangle(x, y + 95, 32, 16)
end

function selene.mouse_button_callback(pressed, which, btn)
  print(which, btn)
end
