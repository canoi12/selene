local Image = require('core.graphics.Image')
local Tileset = require('engine.Tileset')
local World = require('engine.World')

--- @class engine.Project
--- @field data table
--- @field currentWorld integer
--- @field worlds engine.World[]
--- @field images Image[]
--- @field tilesets engine.Tileset[]
--- @field fs Filesystem
local Project = {}
local mt = {}
mt.__index = Project

function Project.create(data)
    if not data then
        error('Project data cannot be null')
    end
    --- @type engine.Project
    local p = setmetatable({}, mt)

    p.fs = selene.engine.projectFs
    p.data = data

    selene.project = p

    p.worlds = {}
    p.images = {}
    p.tilesets = {}
    local defs = p.data.defs
    local tsets = defs.tilesets
    for _,tset in ipairs(tsets) do
        p.images[tset.relPath] = Image.load(p.fs:resolve(tset.relPath))
        p.tilesets[tset.uid] = Tileset.create(tset)
    end

    p.worlds[1] = World.create(p.data)
    p.currentWorld = 1

    return p
end

--- @param dt number
function Project:onUpdate(dt)
    local w = self.worlds[self.currentWorld]
    if w then
        w:onUpdate(dt)
    end
end

--- @param r core.Renderer
function Project:onRender(r)
    local w = self.worlds[self.currentWorld]
    if w then
        w:onRender(r)
    end
end

return Project