local sdl = selene.sdl2
local keyboard = {}

function keyboard.is_down(key)
  local scancode = sdl.GetScancodeFromName(key)
  return sdl.CheckKeyState(scancode)
end

function keyboard.is_up(key)
  return not keyboard.is_down(key)
end

return keyboard
