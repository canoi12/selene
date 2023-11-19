local Keyboard = {}

function Keyboard.isDown(key)
  local scancode = selene.sdl2.getScancodeFromName(key)
  return selene.sdl2.checkKeyState(scancode)
end

function Keyboard.isUp(key)
  return not Keyboard.isDown(key)
end

return Keyboard
