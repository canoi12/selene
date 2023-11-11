local sdl = selene.sdl2
local Event = {}

local eventType = {
    [sdl.QUIT] = "quit",
    [sdl.WINDOWEVENT] = "window event",
    [sdl.MOUSEMOTION] = "mouse motion",
    [sdl.MOUSEBUTTONDOWN] = "mouse button down",
    [sdl.MOUSEBUTTONUP] = "mouse button up",
}

function Event.create()
    local e = {}
    e.handle = sdl.Event.create()
    return setmetatable(e, {
        __index = Event
    })
end

function Event:poll()
    return self.event:poll()
end

function Event:getType()
    return eventType[self.handle:getType()]
end


return Event