local App = require('App')
local Canvas = require('graphics.Canvas')
local Color = require('graphics.Color')
local Image = require('graphics.Image')
local Rect = require('Rect')
local Keyboard = require('input.Keyboard')
local app = App.defaultEngine()

json = require('libs.json')

local source = app.projectFs:read('game.ldtk'):getString()
local project = json.decode(source)

local canvas = Canvas.create(160, 95)

local tilesets = project.defs.tilesets
local sprites = Image.load(app.projectFs:resolve(tilesets[1].relPath))
local levels = project.levels
local level = levels[1]

local d = Rect.create(0, 0, 16, 16)
local s = Rect.create(32, 0, 16, 16)

local entities = level.layerInstances[1]
local layer = level.layerInstances[2]

local player = entities.entityInstances[1].__tile

function app:onInit()
    
end

for i,c in ipairs(entities.entityInstances) do
        
    if c.__identifier == "Player" then
        player = c
        print(c)
        local p = c.__tile
        print(p.x, p.y)
    end
end

local time = 0

function app:onUpdate(dt)
    time = time + dt
    if Keyboard.isDown('left') then
        player.__worldX = player.__worldX - (80 * dt)
    elseif Keyboard.isDown('right') then
        player.__worldX = player.__worldX + (80 * dt)
    end

    if Keyboard.isDown('up') then
        player.__worldY = player.__worldY - (80 * dt)
    elseif Keyboard.isDown('down') then
        player.__worldY = player.__worldY + (80 * dt)
    end
end

print(canvas.handle)
local cr = Rect.create(0, 0, 640, 380)
--- @param r Renderer
function app:onRender(r)
    r:setClearColor(Color.gray)
    r:clear()
    r:setCanvas(canvas)
    r:setClearColor(Color.black)
    r:clear()

    d.x = 0
    d.y = 0
    -- r:translate(80, 47)
    -- r:rotate(time)
    -- r:translate(-player.__worldX, -player.__worldY)
    for i,c in ipairs(layer.autoLayerTiles) do
        d.x = c.px[1]
        d.y = c.px[2]
        s.x = c.src[1]
        s.y = c.src[2]
        r:blit(sprites, s, d)
    end

    for i,c in ipairs(entities.entityInstances) do
        d.x = c.__worldX
        d.y = c.__worldY
        s.x = c.__tile.x
        s.y = c.__tile.y
        r:blit(sprites, s, d)
    end
    r:setCanvas()
    r:blit(canvas, nil, cr)
end

return app
