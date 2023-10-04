local class = require 'core.class'
local ui = class:extend('ui')

function ui:constructor()
  self.frames = {}
  self.buttons = {}
end

function ui:begin()
  self.frames = {}
end

function ui:finish()
end

function ui:button(text)
end

return ui
