local sdl = selene.sdl2

--- @alias event_types
---| 'quit'
---| 'window event'
---| 'mouse motion'
---| 'mouse button down'
---| 'mouse button up'
---| 'audio device added'
---| 'audio device removed'

--- @type {integer:event_types}
local types = {
    [sdl.QUIT] = "quit",
    [sdl.WINDOWEVENT] = "window event",
    [sdl.MOUSEMOTION] = "mouse motion",
    [sdl.MOUSEBUTTONDOWN] = "mouse button down",
    [sdl.MOUSEBUTTONUP] = "mouse button up",
    [sdl.AUDIODEVICEADDED] = "audio device added",
    [sdl.AUDIODEVICEREMOVED] = "audio device removed"
}

--- @alias window_ev_types
---| 'close'
---| 'enter'
---| 'leave'
---| 'focus gained'
---| 'focus lost'
---| 'resized'
---| 'hidden'
---| 'maximized'
---| 'minimized'
---| 'restored'
---| 'moved'

--- @type {integer:window_ev_types}
local window_ev_table = {
    [sdl.WINDOWEVENT_CLOSE] = "close",
    [sdl.WINDOWEVENT_ENTER] = "enter",
    [sdl.WINDOWEVENT_LEAVE] = "leave",
    [sdl.WINDOWEVENT_FOCUS_GAINED] = "focus gained",
    [sdl.WINDOWEVENT_FOCUS_LOST] = "focus lost",
    [sdl.WINDOWEVENT_RESIZED] = "resized",
    [sdl.WINDOWEVENT_HIDDEN] = "hidden",
    [sdl.WINDOWEVENT_MAXIMIZED] = "maximized",
    [sdl.WINDOWEVENT_MINIMIZED] = "minimized",
    [sdl.WINDOWEVENT_RESTORED] = "restored",
    [sdl.WINDOWEVENT_MOVED] = "moved",
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
--- @return event_types
function Event:getType()
    return types[self.handle:getType()]
end

--- @return window_ev_types, integer, integer, integer
function Event:getWindowEvent()
    local wev, wid, d1, d2 = self.handle:windowEvent()
    return window_ev_table[wev], wid, d1, d2
end

--- Poll event
--- @return boolean
function Event:poll()
    return self.handle:poll()
end

return Event