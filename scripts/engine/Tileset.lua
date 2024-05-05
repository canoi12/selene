local Rect = require('core.Rect')
--- @class engine.Tileset
--- @field gridSize integer
--- @field dest Rect
--- @field src Rect
--- @field image Image
local Tileset = {}
local mt = {}
mt.__index = Tileset

function Tileset.create(data)
    local t = setmetatable({}, mt)
    local p = selene.project
    t.gridSize = data.tileGridSize
    t.dest = Rect.create(0, 0, t.gridSize, t.gridSize)
    t.src = Rect.create(0, 0, t.gridSize, t.gridSize)
    t.image = p.images[data.relPath]
    return t
end

--- @param r core.Renderer
function Tileset:draw(r)
    r:blit(self.image, self.dest, self.src)
end

return Tileset