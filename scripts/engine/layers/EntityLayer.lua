--- @class engine.layers.EntityLayer : engine.layers.Layer
local EntityLayer = {}
local mt = {}
mt.__index = EntityLayer

--- @param data table
--- @return engine.layers.EntityLayer
function EntityLayer.create(data)
    local el = {}

    el.entities = {}

    return setmetatable(el, mt)
end

--- @param dt number
function EntityLayer:update(dt)
end

--- @param r core.Renderer
function EntityLayer:draw(r)
    for _,e in ipairs(self.entities) do
        e:draw(r)
    end
end



return EntityLayer