local AssetManager = require('engine.assets.AssetManager')
local Color = require('graphics.Color')
local Settings = require('Settings')
--- @class engine
local engine = {}

--- @param app App
--- @return Settings
function engine.setup(app)
  --- @type Settings
  local config = app.projectFs:load('.selene/settings.lua')
  if type(config) == "function" then
    config = config()
  else
    config = Settings.default()
  end
  config.window.resizable = true
  app.assetManager = AssetManager.create(app.projectFs.basePath)

  app.editor = require('editor')
  app.editor.setup(app)

  app.update = function(s, dt)
    s.editor.update(dt)
  end

  local col = Color.rgb(75, 125, 125)
  app.draw = function(s, r)
    r:begin()
    r:clearColor(col)
    r:clear()
    s.editor.draw(r)
    r:finish()
  end

  app.onEvent = function(s, ev)
    s.editor.onEvent(ev)
  end

  return config
end

return engine
