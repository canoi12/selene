local sdl = selene.sdl2
local mouse = {}

function mouse.get_position()
  return sdl.GetMousePosition()
end

function mouse.get_relative_position()
  return sdl.GetMouseRelativePosition()
end

function mouse.is_down(btn)
  return sdl.IsMouseDown(btn)
end

function mouse.is_up(btn)
  return not mouse.is_down()
end

return mouse
