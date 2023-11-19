--- @class Mouse
local Mouse = {}

function Mouse.getPosition()
  return selene.sdl2.getMousePosition()
end

function Mouse.getRelativePosition()
  return selene.sdl2.getMouseRelativePosition()
end

function Mouse.isDown(btn)
  return selene.sdl2.isMouseDown(btn)
end

function Mouse.isUp(btn)
  return not Mouse.isDown(btn)
end

return Mouse
