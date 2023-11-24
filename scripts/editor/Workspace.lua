local Rect = require('Rect')

--- @alias ActionMode
---| 'none'
---| 'window move'
---| 'window resized'

--- @class editor.Workspace
--- @field actionMode ActionMode
--- @field currentWindow editor.Window | nil
--- @field windows editor.Window[]
--- @field mousePressed boolean
--- @field mouse table
local Workspace = {}

local workspace_mt = {}
workspace_mt.__index = Workspace

--- @return editor.Workspace
function Workspace.create()
    local w = {}
    w.windows = {}
    w.currentWindow = nil
    w.mousePressed = false
    w.mouse = {
        x = 0,
        y = 0
    }
    w.actionMode = 'none'
    return setmetatable(w, workspace_mt)
end

--- @param dt number
function Workspace:update(dt)
end

--- @param r Renderer
function Workspace:draw(r)
    for i,win in ipairs(self.windows) do
        local win = self.windows[i]
        win:draw(r)
    end
end

--- @param ev Event
function Workspace:onEvent(ev)
    local eventType = ev:getType()
    if eventType == 'mouse button down' then
        local which, btn, x, y = ev.handle:mouseButtonEvent()
        for i=#self.windows,1,-1 do
            local win = self.windows[i]
            if win.rect:intersect(x, y) then
                self.currentWindow = win
                table.remove(self.windows, i)
                table.insert(self.windows, #self.windows + 1, win)
                break
            end
        end
    elseif eventType == 'mouse button up' then
        if self.currentWindow then
            self.currentWindow:onEvent(ev)
            self.currentWindow = nil
        end
    end
    -- if eventType == 'mouse motion' then
    --     local which, state, x, y, xrel, yrel = ev.handle:mouseMotionEvent()
    --     local win = self.currentWindow
    --     if win and self.actionMode == 'window move' then
    --         win.rect.x = win.rect.x + xrel
    --         win.rect.y = win.rect.y + yrel
    --     end
    -- end

    if self.currentWindow then
        self.currentWindow:onEvent(ev)
    end
end

return Workspace