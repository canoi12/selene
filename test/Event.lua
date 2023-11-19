local sdl = selene.sdl2

local types = {
    [sdl.QUIT] = "quit",
    [sdl.WINDOWEVENT] = "window event",
    [sdl.MOUSEMOTION] = "mouse motion",
    [sdl.MOUSEBUTTONDOWN] = "mouse button down",
    [sdl.MOUSEBUTTONUP] = "mouse button up",
}

--- @class Event
--- @field handle sdl.Event
local Event = {}

local ev_mt = {}
ev_mt.__index = Event

--- Create a new event
--- @return Event
function Event.create()
    local e = {}
    e.handle = sdl.Event.create()
    return setmetatable(e, ev_mt)
end

--- Get the Type of the current event
--- @return string
function Event:getType()
    return types[self.handle:getType()]
end

--- Poll event
--- @return boolean
function Event:poll()
    return self.handle:poll()
end

return Event