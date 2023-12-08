local Button = require('engine.ui.components.Button')
local Color = require('graphics.Color')
local Label = require('engine.ui.components.Label')
local Rect = require('Rect')
local Vec2 = require('math.Vec2')
local Window = require('engine.ui.Window')

--- @alias ui.ComponentState
---| "normal"
---| "hovered"
---| "pressed"
---| "released"
---| "down"
---| "drag"
---| "focused"

--- @class ui.Component
--- @field id string
--- @field size Vec2
--- @field state ui.ComponentState
local Component = {}

--- @class ui.Checkbox : ui.Component
--- @field checked boolean
local Checkbox = {}

--- @alias ui.WindowControl {current:ui.Window|nil,moving:boolean,resizing:boolean}

--- @alias ui.MouseDragState {changed:boolean,x:number,y:number}
--- @alias ui.MouseScrollState {changed:boolean,value:number}
--- @alias ui.MouseButtonState {clicked:boolean,released:boolean,up:boolean,down:boolean}

--- @class ui.MouseContext
--- @field x number
--- @field y number
--- @field dx number
--- @field dy number
--- @field drag ui.MouseDragState
--- @field scroll ui.MouseScrollState
--- @field button ui.MouseButtonState[]
local MouseContext = {}

--- @alias ui.WindowEditMode
---| "none"
---| "move"
---| "resize"

--- @alias ui.WindowState {current:ui.Window}

--- @class ui.State
--- @field hovered {window:ui.Window|nil,component:ui.Component|nil}
--- @field focused {window:ui.Window,component:ui.Component}
--- @field dragging {window:ui.Window,component:ui.Component}
--- @field resizing {window:ui.Window,component:ui.Component}
--- @field active {window:ui.Window,component:ui.Component}

--- @class ui.Context
--- @field drawCursor Vec2
--- @field windows ui.Window[]
--- @field windowPool ui.Window[]
--- @field style ui.Style
--- @field activeComponent ui.Component | nil
--- @field currentWindow ui.Window | nil
--- @field windowControl ui.WindowControl
--- @field editWindow {window:ui.Window|nil,state:string}
--- @field mouse ui.MouseContext
--- @field text string
--- @field components ui.Component[]
local ui = {}
local ui_mt = {}
ui_mt.__index = ui

--- @param style ui.Style
--- @return ui.Context
function ui.create(style)
    --- @type ui.Context
    local obj = {
        style = style,
        drawCursor = Vec2.create(0, 0),
        currentWindow = nil,
        mouse = {
            x = 0,
            y = 0,
            dx = 0,
            dy = 0,
            drag = {
                changed = false,
                x = 0,
                y = 0
            },
            scroll = {
                changed = false,
                value = 0
            },
            button = {
                {
                    clicked = false,
                    released = false,
                    up = false,
                    down = false
                },
                {
                    clicked = false,
                    released = false,
                    up = false,
                    down = false
                },
                {
                    clicked = false,
                    released = false,
                    down = false
                }
            }
        },
        windows = {},
        windowPool = {},
        windowControl = {
            current = nil,
            moving = false,
            resizing = false
        },
        activeComponent = nil,
        editWindow = { window = nil, state = "none" },
        text = "",
        components = {}
    }
    return setmetatable(obj, ui_mt)
end

function ui:begin()
end

--- Begin window
--- @param s ui.Context
--- @param win ui.Window
--- @param title string
local function beginWindow(s, win, title)
end

--- @param title string
--- @param rect Rect | nil
--- @return boolean
function ui:window(id, title, rect)
    if not self.windowPool[id] then
        rect = rect or Rect.create(64, 128, 128, 128)
        self.windowPool[id] = Window.create(title, rect, self.style)
        table.insert(self.windows, self.windowPool[id])
    end
    local win = self.windowPool[id]
    win.components = {}
    self.windowPool[id].cursorPos[1] = win.rect.x
    self.windowPool[id].cursorPos[2] = win.rect.y + 18
    self.currentWindow = win
    return self.windowPool[id].isOpen
end

function ui:hbox(id)
    return {
        begin = function() end,
        finish = function() end
    }
end

--- @param id string
--- @param label string
function ui:label(id, label)
    if not self.components[id] then
        self.components[id] = Label.create(id, label)
    end
    self.components[id].text = label
    local size = { #label * 8 + 4, 8 }
    local cursor = self.currentWindow.cursorPos
    table.insert(
        self.currentWindow.components,
        {
            id = id,
            pos = { cursor[1], cursor[2] },
            size = size,
        })
    cursor[1] = cursor[1] + size[1]
end

--- @param id string
--- @param label string
--- @param size Vec2 | nil
--- @return boolean
function ui:button(id, label, size)
    if not self.components[id] then
        self.components[id] = Button.create(id, label)
    end
    self.components[id].label = label
    --- @type ui.component.Button
    local btn = self.components[id]
    local win = self.currentWindow
    local cursor = self.currentWindow.cursorPos
    local sz = { #label * 8 + 8, 16 }
    table.insert(
        win.components,
        {
            id = id,
            pos = { cursor[1], cursor[2] },
            size = sz
        }
    )
    local pressed = btn.state == "released"
    if pressed then btn.state = "normal" end
    cursor[1] = cursor[1] + sz[1]
    return pressed
end

function ui:item(id, label)
end

function ui:checkbox(id, value)
    if not self.components[id] then
        self.components[id] = {}
    end
end

function ui:checklist(id, values)
end

function ui:finish(r)
    --- draw ui stuff
    for i, win in ipairs(self.windows) do
        win:draw(r)
    end
    for i, button in ipairs(self.mouse.button) do
        button.clicked = false
        button.released = false
    end
end

--- @param r Renderer
function ui:render(r)
    for _, win in ipairs(self.windows) do
        win:draw(r)
        local cursor = win.cursorPos
        for _, c in ipairs(win.components) do
            local cc = self.components[c.id]
            local x0 = c.pos[1]
            local y0 = c.pos[2]
            cc:draw(r, x0, y0, x0 + c.size[1], y0 + c.size[2], self.style)
            --[[
            local cc = self.components[c.id]
            r:setDrawColor(self.style.text.shadow)
            r:print(cc.text, cursor[1]+2, cursor[2]+2)
            r:setDrawColor(self.style.text.fg)
            r:print(cc.text, cursor[1]+1, cursor[2]+1)
            ]]
        end
        --[[
        r:setDrawColor(self.style.text.shadow)
        r:print(self.text, win.rect.x+2, win.rect.y+18)
        r:setDrawColor(self.style.text.fg)
        r:print(self.text, win.rect.x+1, win.rect.y+17)
        ]]
    end
    r:setClipRect()
    for i, button in ipairs(self.mouse.button) do
        button.clicked = false
        button.released = false
    end
end

--- @param ev Event
function ui:onEvent(ev)
    local tp = ev:getType()
    if ev.name == 'text input' then
        self.text = self.text .. ev.text.text
    elseif ev.name == 'key down' then
        local name = selene.sdl2.getScancodeName(ev.key.scancode)
        if name == "Backspace" then
            self.text = string.sub(self.text, 1, #self.text - 1)
        elseif name == "Return" then
            self.text = self.text .. '\n'
        end
    end
    if ev.name == 'mouse motion' then
        self:onMouseMotion(ev.mouse.x, ev.mouse.y, ev.mouse.motion.x, ev.mouse.motion.y)
    elseif ev.name == 'mouse button' then
        self:onMouseButton(ev.mouse.button.index, ev.mouse.button.pressed, ev.mouse.x, ev.mouse.y)
    end
end

function ui:onMouseMotion(x, y, dx, dy)
    self.mouse.x = x
    self.mouse.y = y
    self.mouse.dx = dx
    self.mouse.dy = dy
    if self.editWindow.state ~= 'none' then
        --- @type ui.Window
        local win = self.editWindow.window
        if self.editWindow.state == "move" then
            win.rect.x = win.rect.x + dx
            win.rect.y = win.rect.y + dy
        elseif self.editWindow.state == "resize" then
            win.rect.w = math.max(win.rect.w + dx, 64)
            win.rect.h = math.max(win.rect.h + dy, 64)
        end
    end

    local win = nil
    for i = #self.windows, 1, -1 do
        if self.windows[i].rect:intersect(x, y) then
            win = self.windows[i]
            break
        end
    end
    if self.hoveredWindow ~= win then
        if self.hoveredComponent then
            self.hoveredComponent.state = "normal"
        end
        self.hoveredWindow = win
    end

    if win then
        for _, c in ipairs(win.components) do
            local cc = self.components[c.id]
            local rr = {
                x = c.pos[1],
                y = c.pos[2],
                w = c.size[1],
                h = c.size[2]
            }
            local mt = getmetatable(cc)
            if mt.__index == Button and cc.state ~= "pressed" then
                if Rect.intersect(rr, x, y) then
                    if cc.state ~= "pressed" then
                        cc.state = "hovered"
                        self.hoveredComponent = cc
                    end
                else
                    cc.state = "normal"
                end
            end
        end
    end

    for _, btn in ipairs(self.mouse.button) do
        if btn.down then
            self.mouse.drag.changed = true
            self.mouse.drag.x = dx
            self.mouse.drag.y = dy
        end
    end
end

function ui:onMouseButton(btn, pressed, x, y)
    self.mouse.button[btn].clicked = pressed
    self.mouse.button[btn].down = pressed

    for i = #self.windows, 1, -1 do
        local win = self.windows[i]
        local wm = {
            x = win.rect.x,
            y = win.rect.y,
            w = win.rect.w,
            h = 16
        }
        local wr = {
            x = win.rect.x + win.rect.w - 8,
            y = win.rect.y + win.rect.h - 8,
            w = 8,
            h = 8
        }
        for _, c in ipairs(win.components) do
            local cc = self.components[c.id]
            local rr = {
                x = c.pos[1],
                y = c.pos[2],
                w = c.size[1],
                h = c.size[2]
            }
            local mt = getmetatable(cc)
            if mt.__index == Button then
                if Rect.intersect(rr, x, y) then
                    if pressed then
                        cc.state = "pressed"
                    elseif cc.state == "pressed" then
                        cc.state = "released"
                    else
                        cc.state = "hovered"
                    end
                else
                    cc.state = "normal"
                end
            end
        end
        if pressed and win.rect:intersect(x, y) then
            if i ~= #self.windows then
                table.insert(self.windows, table.remove(self.windows, i))
            end
            if btn == 1 then
                self.editWindow.window = win
                if Rect.intersect(wm, x, y) then
                    self.editWindow.state = "move"
                    break
                elseif Rect.intersect(wr, x, y) then
                    self.editWindow.state = "resize"
                    break
                end
            end
            break
        end
    end
    if not pressed then
        self.editWindow.window = nil
        self.editWindow.state = "none"
        self.mouse.drag.changed = false
    end
end

return ui
