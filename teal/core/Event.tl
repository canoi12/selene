local sdl = selene.sdl2
local enum EventType
    "quit"
    "window event"
    "mouse motion"
    "mouse button down"
    "mouse button up"
end

local types : {integer:EventType} = {
    [sdl.QUIT] = "quit",
    [sdl.WINDOWEVENT] = "window event",
    [sdl.MOUSEMOTION] = "mouse motion",
    [sdl.MOUSEBUTTONDOWN] = "mouse button down",
    [sdl.MOUSEBUTTONUP] = "mouse button up",

}

local record Event
    handle: sdl.Event

    create: function() : Event
    poll: function(Event) : boolean
    getType: function(Event) : EventType | nil
end

function Event.create() : Event
    local e : Event = {}
    e.handle = sdl.Event.create()
    return setmetatable(e, {
        __index = Event
    })
end

function Event:getType() : EventType | nil
    return types[self.handle:getType()]
end

function Event:poll() : boolean
    return self.handle:poll()
end

return Event