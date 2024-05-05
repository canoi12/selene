local AutoLayer = require('engine.layers.AutoLayer')
local EntityLayer = require('engine.layers.EntityLayer')
--- @class engine.levels.Level
--- @field entities table
--- @field layers engine.layers.Layer[]
local Level = {}
local mt = {}
mt.__index = Level

--- @param data table
--- @return engine.levels.Level
function Level.create(data)
    local l = setmetatable({}, mt)
    l.entities = {}
    l.layers = {}

    for _,layer in ipairs(data.layerInstances) do
        if layer.__type == "EntityLayer" then
            table.insert(EntityLayer.create(layer))
        end
    end
    return l
end

--- @param dt number
function Level:update(dt)
end

--- @param r core.Renderer
function Level:draw(r)
end

return Level