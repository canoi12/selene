--- @meta
--- @class engine.Entity
local Entity = {}

local mt = {
    __index = Entity,
    __call = function(data)
        
    end
}

--- @param data table
function Entity:onCreate(data)
end

--- @param dt number
function Entity:onUpdate(dt)
end

--- @param r core.Renderer
function Entity:onRender(r)
end

return Entity