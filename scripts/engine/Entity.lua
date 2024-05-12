local Rect = require('core.Rect')
--- @class engine.Entity
local Entity = {}
local mt = {}
mt.__index = Entity
mt.__call = Entity.extends

function Entity.extends(name)
    local e = setmetatable({}, mt)
end

--- @param data table
function Entity.create(data)
    local e = setmetatable({}, mt)
    e.dest = Rect.create(data.px[1], data.px[2], data.width, data.height)
    local tile = data.__tile
    e.src = Rect.create(tile.x, tile.y, tile.w, tile.h)
    e.image = selene.project.tilesets[tile.tilesetUid].image
    return e
end

--- @param dt number
function Entity:onUpdate(dt)
end

--- @param r core.Renderer
function Entity:onRender(r)
    r:blit(self.image, self.src, self.dest)
end

return Entity