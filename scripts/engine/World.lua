local json = selene.json

local Level = require('engine.levels.Level')

--- @class engine.World
--- @field levels engine.levels.Level[]
--- @field currentLevel engine.levels.Level
local World = {}
local mt = {}
mt.__index = World

function World.create(data)
    local w = {}

    local p = selene.project

    w.data = data
    w.images = {}
    w.tilesets = {}
    w.levels = {}
    w.persistent = {}
    w.currentLevel = nil
    w.camera = nil

    if #data.levels > 0 then
        w.currentLevel = Level.create(w, data.levels[1])
    end
    canvas = require('core.graphics.Canvas').create(160, 95)

    return setmetatable(w, mt)
end

--- @param dt number
function World:onUpdate(dt)
    if self.currentLevel then self.currentLevel:onUpdate(dt) end
end

--- @param r core.Renderer
function World:onRender(r)
    if self.camera then
        self.camera:begin(r)
    end
    if self.currentLevel then self.currentLevel:onRender(r) end
    if self.camera then
        self.camera:finish(r)
        self.camera:onRender(r)
    end
    -- r:blit(canvas)
end

function World:gotoLevel(name)
    
end

return World