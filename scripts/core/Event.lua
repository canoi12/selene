local sdl = selene.sdl2








local types = {
   [sdl.QUIT] = "quit",
   [sdl.WINDOWEVENT] = "window event",
   [sdl.MOUSEMOTION] = "mouse motion",
   [sdl.MOUSEBUTTONDOWN] = "mouse button down",
   [sdl.MOUSEBUTTONUP] = "mouse button up",

}

local Event = {}







function Event.create()
   local e = {}
   e.handle = sdl.Event.create()
   return setmetatable(e, {
      __index = Event,
   })
end

function Event:getType()
   return types[self.handle:getType()]
end

function Event:poll()
   return self.handle:poll()
end

return Event
