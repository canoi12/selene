local AutoLayer = require('engine.layers.AutoLayer')
local EntitiesLayer = require('engine.layers.EntitiesLayer')
local Entity = require('engine.Entity')
--- @class engine.levels.Level
--- @field entities table
--- @field layers engine.layers.Layer[]
local Level = {}
local mt = {}
mt.__index = Level

--- @param world engine.World
--- @param data table
--- @return engine.levels.Level
function Level.create(world, data)
    local l = setmetatable({}, mt)
    l.entities = {}
    l.layers = {}
    l.world = world

    for _,layer in ipairs(data.layerInstances) do
        if layer.__type == "Entities" then
            table.insert(l.layers, EntitiesLayer.create(l, layer))
        elseif layer.__type == "AutoLayer" then
            print('AutoLayer', layer)
            table.insert(l.layers, AutoLayer.create(layer))
        end
    end
    return l
end

--- @param dt number
function Level:onUpdate(dt)
end

--- @param r core.Renderer
function Level:onRender(r)
    for _,layer in ipairs(self.layers) do
        layer:onRender(r)
    end
end

return Level