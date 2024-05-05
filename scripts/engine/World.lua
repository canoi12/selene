local json = selene.json

--- @class engine.World
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

    return setmetatable(w, mt)
end

--- @param dt number
function World:update(dt)
end

--- @param r core.Renderer
function World:draw(r)
end

function World:gotoLevel(name)
    
end

return World