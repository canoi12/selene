local Camera = require('engine.Camera')
local Entity = require('engine.Entity')
--- @class engine.layers.EntitiesLayer : engine.layers.Layer
--- @field entities engine.Entity[]
local EntitiesLayer = {}
local mt = {}
mt.__index = EntitiesLayer

--- @param data table
--- @return engine.layers.EntitiesLayer
function EntitiesLayer.create(level, data)
    local el = {}

    el.entities = {}

    for _,e in ipairs(data.entityInstances) do
        if e.__identifier == "Camera" then
            level.world.camera = Camera.create(e)
        else
            local ee = Entity.create(e)
            level.entities[e.iid] = ee
            table.insert(el.entities, ee)
        end
    end

    return setmetatable(el, mt)
end

--- @param dt number
function EntitiesLayer:onUpdate(dt)
end

--- @param r core.Renderer
function EntitiesLayer:onRender(r)
    for _,e in ipairs(self.entities) do
        e:onRender(r)
    end
end



return EntitiesLayer