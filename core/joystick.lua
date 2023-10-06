local sdl = selene.sdl2
local joystick = {}

function joystick.open(index)
  return sdl.JoystickOpen(index)
end

function joystick.count()
  return sdl.NumJoysticks()
end

return joystick
