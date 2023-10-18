local sdl = selene.sdl2
local joystick = {}

function joystick.open(index)
  return sdl.Joystick.Open(index)
end

function joystick.count()
  return sdl.Joystick.Count()
end

return joystick
