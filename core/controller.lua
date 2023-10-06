local sdl = selene.sdl2
local controller = {}

function controller.open(index)
  return sdl.GameControllerOpen(index)
end

function controller.is_valid(index)
  return sdl.IsGameController(index)
end

return controller
