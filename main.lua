local App = require('App')
local app = App.defaultEngine()
app:initUI('pastel-pop16')
return app

--[[
    local Settings = require('Settings')
local Color = require('graphics.Color')

local Mouse = require('input.Mouse')
local Keyboard = require('input.Keyboard')

local settings = Settings.create("Editor", 640, 380)
settings.window.resizable = true
local app = App.create(settings)

local Rect = require('Rect')
local Window = require('editor.Window')
local Frame = require('editor.Frame')

local Image = require('graphics.Image')

local style = require('editor.style')

local img = Image.load('examples/draw/sprites.png')
print(img, img.handle)

local window = Window.create("Window", Rect.create(128, 64, 320, 96))
window.children = Rect.create(128+8, 64+16+8, 320-16, 96-16)

local windows = {}
table.insert(windows, window)
table.insert(windows, Window.create("Test", Rect.create(320, 128, 117, 64)))
table.insert(windows, Window.create("Image viewer", Rect.create(320, 128, img:getWidth() + 32, img:getHeight() + 48)))
windows[3].image = img

local x = 0
function app:update(dt)
    if Mouse.isDown(1) then
        window.x, window.y = Mouse.getPosition()
    end
end

local menu = {
    "File",
    "Project",
    "About"
}

local menu_rect = Rect.create(0, 0, 640, 16)

local rr = Rect.create(128, 64, img.width + 64, img.height + 64)

print(img.width, img.height)
--- @param r Renderer
function app:draw(r)
    local width, height = app.window:getSize()
    menu_rect.w = width
    r:begin()
    r:clearColor(Color.rgb(75, 125, 125))
    r:clear()

    Frame.draw(r, menu_rect)

    local mx, my = Mouse.getPosition()

    local xx = 4
    local rect = Rect.create(0, 0, 16, 16)
    for i,name in ipairs(menu) do
        local size = #name * 8

        rect.w = size + 7

        if rect:intersect(mx, my) then
            Frame.draw(r, rect, style.button.hover)
        else
            Frame.draw(r, rect)
        end

        local c1 = Color.black
        local c2 = Color.white
        if mx >= (xx -3) and mx <= (xx+size+4) and
            my >= 0 and my <= 16 then
                c1 = Color.white
                c2 = Color.black
            end
        -- r:setDrawColor(c2)
        -- r:drawRectangle(xx-3, 1, size + 7, 15)
        -- r:setDrawColor(c1)
        -- r:drawLine(xx-3, 16, xx + size + 4, 16)
        -- r:drawLine(xx + size + 4, 16, xx + size + 4, 0)
        r:setDrawColor(Color.black)
        r:print(name, rect.x + 5, rect.y + 5)
        r:setDrawColor(Color.white)
        r:print(name, rect.x + 4, rect.y + 4)
        xx = xx + size + 8
        rect.x = rect.x + rect.w + 1
    end

    for i,v in ipairs(windows) do
        v:draw(r)
        if v.children then
            v.children.x = v.availableRegion.x + 8
            v.children.y = v.availableRegion.y + 8
            v.children.w = v.availableRegion.w - 16
            v.children.h = v.availableRegion.h - 16
            Frame.draw(r, v.children, style.button.hover)
        end
        if v.image then
            r:copy(v.image, nil, Rect.create(v.rect.x + 16, v.availableRegion.y + 8, v.image:getWidth(), v.image:getHeight()))
        end
    end

    r:finish()
end

local down = false

--- @param ev Event
function App:onEvent(ev)
    local tp = ev:getType()
    if tp == "mouse button down" then
        local which, btn, x, y = ev.handle:mouseButtonEvent()
        if btn == 1 then
            for i,v in ipairs(windows) do
                if v.topBar:intersect(x, y) then
                    down = true
                    window = table.remove(windows, i)
                    table.insert(windows, window)
                    break
                end
            end
        end
    elseif tp == "mouse button up" then
        down = false
    elseif tp == 'mouse motion' then
        local which, state, x, y, xrel, yrel = ev.handle:mouseMotionEvent()
        if down then
            window.rect.x = window.rect.x + xrel
            window.rect.y = window.rect.y + yrel
        end
    end
end

return app
]]
