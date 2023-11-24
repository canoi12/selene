local Rect = require('Rect')
local Window = require('engine.ui.Window')

--- @class ui.MouseContext
--- @field x number
--- @field y number
--- @field dx number
--- @field dy number
--- @field scroll number
--- @field button integer
--- @field state {clicked:boolean,released:boolean,up:boolean,down:boolean,scrolling:boolean}
local MouseContext = {}

--- @class ui.Context
--- @field windows ui.Window[]
--- @field windowPool ui.Window[]
--- @field style ui.Style
--- @field currentWindow ui.Window | nil
--- @field mouse ui.MouseContext
local ui = {}
local ui_mt = {}
ui_mt.__index = ui

--- @param style ui.Style
--- @return ui.Context
function ui.create(style)
    --- @type ui.Context
    local obj = {
        style = style,
        mouse = {
            x = 0, y = 0,
            dx = 0, dy = 0,
            scroll = 0,
            button = 0,
            state = {
                clicked = false,
                released = false,
                up = false,
                down = false,
                scrolling = false
            }
        },
        windows = {},
        windowPool = {},
        currentWindow = nil
    }
    return setmetatable(obj, ui_mt)
end

function ui:begin()
end

--- Begin windows
--- @param win ui.Window
--- @param title string
local function beginWindow(win, title)
end

--- @param title string
--- @param rect Rect | nil
--- @return boolean
function ui:window(id, title, rect)
    if not self.windows[id] then
        rect = rect or Rect.create(64, 128, 128, 128)
        self.windows[id] = Window.create(title, rect, self.style.window)
    end
    return self.windows[id].isOpen
end

function ui:hbox(id)
    return {
        begin = function() end,
        finish = function () end
    }
end

function ui:label(id, label)
end

function ui:button(id, label)
    return true
end

function ui:item(id, label)
end

function ui:checkbox(id, value)
end

function ui:checklist(id, values)
end

function ui:finish()
end

--- @param ev Event
function ui:onEvent(ev)
    local tp = ev:getType()
    if tp == 'mouse motion' then
        local which, state, x, y, xrel, yrel = ev:mouseMotionEvent()
        self.mouse.x = x
        self.mouse.y = y
        self.mouse.dx = xrel
        self.mouse.dy = yrel
    elseif tp == 'mouse button down' or tp == 'mouse button up' then
        local which, btn, x, y = ev:mouseButtonEvent()

    end
end

return ui