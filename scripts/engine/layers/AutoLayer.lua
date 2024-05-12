--- @class engine.layers.AutoLayer : engine.layers.Layer
--- @field tiles table
--- @field tileset engine.Tileset
local AutoLayer = {}
local mt = {}
mt.__index = AutoLayer

--- @param data table
--- @return engine.layers.AutoLayer
function AutoLayer.create(data)
    local al = {}
    local p = selene.project

    al.image = p.images[data.__tilesetRelPath]
    al.tileset = p.tilesets[data.__tilesetDefUid]
    al.tiles = data.autoLayerTiles

    return setmetatable(al, mt)
end

--- @param dt number
function AutoLayer:onUpdate(dt)
end

--- @param r core.Renderer
function AutoLayer:onRender(r)
    for _,tile in ipairs(self.tiles) do
        self.tileset.dest.x = tile.px[1]
        self.tileset.dest.y = tile.px[2]
        self.tileset.src.x = tile.src[1]
        self.tileset.src.y = tile.src[2]
        self.tileset:onRender(r)
    end
end

return AutoLayer