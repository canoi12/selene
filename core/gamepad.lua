local sdl = selene.sdl2
local gamepad = {}

function gamepad.open(index)
  return sdl.Gamepad.Open(index)
end

function gamepad.is_valid(index)
  return sdl.Gamepad.IsValid(index)
end

return gamepad
