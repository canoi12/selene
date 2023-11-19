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
  if config then
    config = config()
  else
    config = Settings.default()
  end
  app.assetManager = AssetManager.create(app.projectFs.basePath)

  local col = Color.rgb(75, 125, 125)
  app.draw = function(s, r)
    r:begin()
    r:clearColor(col)
    r:clear()
    r:finish()
  end

  return config
end

return engine
