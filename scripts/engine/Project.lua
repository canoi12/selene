local Image = require('core.graphics.Image')
local Tileset = require('engine.Tileset')
local World = require('engine.World')
selene.json = require('third.json')
local json = selene.json

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

function Project.create(ldtk)
    --- @type engine.Project
    local p = setmetatable({}, mt)

    p.fs = selene.projectFs
    
    if type(ldtk) == "string" then
        p.data = json.load(p.fs:resolve(ldtk))
    elseif type(ldtk) == "table" then
        p.data = ldtk
    else
        error("Invalid type for project data")
    end
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
function Project:update(dt)
    local w = self.worlds[self.currentWorld]
    if w then
        w:update(dt)
    end
end

--- @param r core.Renderer
function Project:draw(r)
    local w = self.worlds[self.currentWorld]
    if w then
        w:draw(r)
    end
end

return Project