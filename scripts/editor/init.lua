local Workspace = require('editor.Workspace')
local MenuBar = require('editor.components.Menu.Bar')
local MenuItem = require('editor.components.Menu.Item')
--- @class editor
--- @field app App
--- @field topBar editor.components.Menu.Bar
--- @field workspace editor.Workspace
local editor = {}

function editor.setup(app)
    editor.app = app
    editor.topBar = MenuBar.create()
    table.insert(editor.topBar.items, MenuItem.create('File'))
    table.insert(editor.topBar.items, MenuItem.create('Project'))
    editor.workspace = Workspace.create()
end

function editor.update(dt)
    local width, height = editor.app.window:getSize()
    editor.topBar.rect.w = width
    editor.topBar:update(dt)
    editor.workspace:update(dt)
end

--- @param r Renderer
function editor.draw(r)
    editor.topBar:draw(r)
    editor.workspace:draw(r)
end

--- Process events
--- @param ev Event
function editor.onEvent(ev)
    editor.topBar:onEvent(ev)
    editor.workspace:onEvent(ev)
end

return editor