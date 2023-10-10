local class = require 'core.class'
local graphics = require 'core.graphics'
local ui = class:extend('ui')

function ui:constructor()
  self.frames = {}
  self.buttons = {}

  self.colors = {
    frame = { 75, 125, 125, 255 }
  }
end

function ui:begin()
  self.frames = {}
end

function ui:finish()
  for i,frame in ipairs(self.frames) do
    graphics.set_color(table.unpack(self.colors.frame))
    graphics.fill_rectangle(frame.x, frame.y, frame.w, frame.h)
  end
end

function ui:button(text)
end

function ui:draw_frame(x, y, w, h)
  table.insert(self.frames, {x=x, y=y, w=w, h=h})
end

return ui
