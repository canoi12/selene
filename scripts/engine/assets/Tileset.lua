--- @class engine.assets.Tileset
local Tileset = {}
local mt = {}
mt.__index = Tileset

function Tileset.create(world, data)
    local t = {}

    t.image = world:getImage(selene.projectFs:resolve(data.relPath))
    
    t.sprite = {}

    setmetatable(t, mt)
end

function Tileset:update(dt)

end

return Tileset