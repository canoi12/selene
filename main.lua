local graphics = require 'core.graphics'
local ui = require('core.ui'):new()
local Canvas = require 'core.graphics.Canvas'
local keyboard = require 'core.keyboard'
local Image = require 'core.graphics.Image'

local highscore = 0

function selene.load()
  canvas = Canvas(160, 95)
  image = Image("sprites.png")
  teste = selene.audio.LoadWav("som.wav")
  print(teste:GetFrequency())
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
  graphics.clear(75, 125, 125)

  graphics.set_canvas(canvas)
  graphics.draw(image)
  graphics.set_canvas()

  graphics.draw(canvas, nil, 0, 32)

  graphics.draw_rectangle(x, y + 95, 32, 16)
  graphics.print('olar')

  -- graphics.set_color(75, 75, 75)
  -- graphics.fill_rectangle(128, 128, 320, 128)
  -- graphics.set_color(0, 0, 0)
  -- graphics.draw_rectangle(129, 128, 319, 13)
  -- graphics.draw_rectangle(129, 128+14, 319, 128-13)
  -- graphics.print('Window', 131, 127)
  -- graphics.set_color()
  -- graphics.draw_line(129, 128, 129+319, 128)
  -- graphics.draw_line(129+319, 128, 129+319, 128+13)
  -- graphics.draw_line(129, 128+14, 129+319, 128+14)
  -- graphics.draw_line(129+319, 128+14, 129+319, 128+128+1)
  -- graphics.print('Window', 132, 126)
end
