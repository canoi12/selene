--- @class engine.layers.AutoLayer : engine.layers.Layer
--- @field tiles table
local AutoLayer = {}
local mt = {}
mt.__index = AutoLayer

--- @param data table
--- @return engine.layers.AutoLayer
function AutoLayer.create(data)
    local al = {}
    local p = selene.project

    al.image = p.images[data.__tilesetRelPath]
    al.tiles = data.autoLayerTiles

    

    return setmetatable(al, mt)
end

--- @param dt number
function AutoLayer:update(dt)
end

--- @param r core.Renderer
function AutoLayer:draw(r)
    for _,tile in ipairs(self.tiles) do
        al.tileset:draw(tile)
    end
end

return AutoLayer