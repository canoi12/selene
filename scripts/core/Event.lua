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
    [sdl.KEYDOWN] = "key down",
    [sdl.KEYUP] = "key up",
    [sdl.TEXTEDITING] = "text editing",
    [sdl.TEXTINPUT] = "text input",
    [sdl.AUDIODEVICEADDED] = "audio device added",
    [sdl.AUDIODEVICEREMOVED] = "audio device removed",
    [sdl.CLIPBOARDUPDATE] = "clipboard update",
    [sdl.DROPFILE] = "drop file",
    [sdl.DROPTEXT] = "drop text",
    [sdl.DROPBEGIN] = "drop begin",
    [sdl.DROPCOMPLETE] = "drop complete",
    [sdl.AUDIODEVICEADDED] = "audio device added",
    [sdl.AUDIODEVICEREMOVED] = "audio device removed",
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

--- @alias EventName
---| "quit"
---| "mouse"

--- @alias MouseEvent {which:integer,x:number,y:number,button:{index:integer,pressed:boolean},motion:{x:number,y:number},scroll:{value:number[],flipped:boolean}}
--- @alias WindowEvent {id:integer,data:integer[]}
--- @alias KeyboardEvent {scancode:integer,keycode:integer,mod:integer,isrepeat:boolean,pressed:boolean}
--- @alias TextEvent {text:string,start:integer,length:integer}

--- @class Event
--- @field handle sdl.Event
--- @field name string
--- @field mouse MouseEvent
--- @field window WindowEvent
--- @field key KeyboardEvent
--- @field text TextEvent
--- @field callbacks {integer:function}
local Event = {}

local ev_mt = {}
ev_mt.__index = Event

--- Create a new event
--- @return Event
function Event.create()
    --- @type Event
    local e = {
        handle = sdl.Event.create(),
        name = 'none',
        audio = {
            which = 0,
            added = false
        },
        gamepad = {
            which = 0,
            added = false,
            button = {
                index = 0,
                state = false
            },
            axis = {
                axis = 0,
                value = 0
            }
        },
        mouse = {
            which = 0,
            x = 0, y = 0,
            button = { index = 0, pressed = false },
            motion = { x = 0, y = 0 },
            scroll = {value = {0, 0}, flipped = false}
        },
        window = {
            id = 0,
            data = {0, 0}
        },
        key = {
            scancode = 0,
            keycode = 0,
            mod = 0,
            isrepeat = false,
            pressed = false
        },
        text = {
            text = "",
            start = 0,
            lenght = 0
        }
    }
    e.callbacks = {
        [sdl.QUIT] = function() e.name = 'quit' end,
        [sdl.APP_TERMINATING] = function() e.name = 'app terminating' end,
        [sdl.APP_LOWMEMORY] = function() e.name = 'app low memory' end,
        [sdl.APP_WILLENTERBACKGROUND] = function() e.name = 'app will enter background' end,
        [sdl.APP_DIDENTERBACKGROUND] = function() e.name = 'app did enter background' end,
        [sdl.APP_WILLENTERFOREGROUND] = function() e.name = 'app will enter foreground' end,
        [sdl.APP_DIDENTERFOREGROUND] = function() e.name = 'app did enter foreground' end,
        [sdl.WINDOWEVENT] = function()
            local ev, wid, d1, d2 = e.handle:windowEvent()
            e.window.id = wid
            e.window.data[1] = d1
            e.window.data[2] = d2
            if window_ev_table[ev] then
                e.name = 'window ' .. window_ev_table[ev]
            else
                e.name = 'none'
            end
        end,
        [sdl.MOUSEBUTTONDOWN] = function()
            e.name = 'mouse button'
            local which,btn,x,y = e.handle:mouseButtonEvent()
            e.mouse.which = which
            e.mouse.x = x
            e.mouse.y = y
            e.mouse.button.index = btn
            e.mouse.button.pressed = true
        end,
        [sdl.MOUSEBUTTONUP] = function()
            e.name = 'mouse button'
            local which,btn,x,y = e.handle:mouseButtonEvent()
            e.mouse.which = which
            e.mouse.x = x
            e.mouse.y = y
            e.mouse.button.index = btn
            e.mouse.button.pressed = false
        end,
        [sdl.MOUSEMOTION] = function()
            e.name = 'mouse motion'
            local which, x, y, xrel, yrel = e.handle:mouseMotionEvent()
            e.mouse.which = which
            e.mouse.x = x
            e.mouse.y = y
            e.mouse.motion.x = xrel
            e.mouse.motion.y = yrel
        end,
        [sdl.MOUSEWHEEL] = function()
            e.name = 'mouse wheel'
            local which, x, y, flipped = e.handle:mouseWheelEvent()
            e.mouse.which = which
            e.mouse.scroll.value[1] = x
            e.mouse.scroll.value[2] = y
            e.mouse.scroll.flipped = flipped
        end,
        [sdl.KEYDOWN] = function()
            e.name = 'key down'
            local scan,pressed,rpt,key,mod = e.handle:keyboardEvent()
            e.key.scancode = scan
            e.key.keycode = key
            e.key.mod = mod
            e.key.pressed = pressed
            e.key.isrepeat = rpt
        end,
        [sdl.KEYUP] = function()
            e.name = 'key up'
            local scan,pressed,rpt,key,mod = e.handle:keyboardEvent()
            e.key.scancode = scan
            e.key.keycode = key
            e.key.mod = mod
            e.key.pressed = pressed
            e.key.isrepeat = rpt
        end,
        [sdl.TEXTEDITING] = function()
            e.name = 'text editing'
            local text, start, length = e.handle:textEditEvent()
            e.text.text = text
            e.text.start = start
            e.text.length = length
        end,
        [sdl.TEXTINPUT] = function()
            e.name = 'text input'
            local text = e.handle:textEvent()
            e.text.text = text
        end,
        [sdl.AUDIODEVICEADDED] = function()
            e.name = 'audio device'
            local which = e.handle:audioDeviceEvent()
            e.audio.added = true
            e.audio.which = which
        end,
        [sdl.AUDIODEVICEREMOVED] = function()
            e.name = 'audio device'
            local which = e.handle:audioDeviceEvent()
            e.audio.added = false
            e.audio.which = which
        end
    }
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
    local ret = self.handle:poll()
    local tp = self.handle:getType()
    local cb = self.callbacks[tp]
    if cb then cb() end
    return ret
end

return Event
