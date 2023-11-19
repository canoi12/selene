local json = require('3rd.json')
local Filesystem = require('Filesystem')
--- @class AssetManager
--- @field assets table
--- @field loaders table
--- @field fs Filesystem
local AssetManager = {}

local manager_mt = {}
manager_mt.__index = AssetManager

function AssetManager.create(path)
  local am = {}

  am.assets = {}
  am.loaders = {}
  am.fs = Filesystem.create(path .. '.selene/assets/')

  local assets = am.fs:load('init.lua')()
  for i,v in ipairs(assets) do
    local data = am.fs:load(v)()
    if not am.assets[data.__type] then
      am.assets[data.__type] = {}
    end
    am.assets[data.__type][data.__name] = {
      path = v,
      data = data
    }
  end

  return setmetatable(am, manager_mt)
end

--- Default image loader
--- @param manager AssetManager
--- @param path string
--- @return ImageAsset
local function imageLoader(manager, path)
  local str = manager.fs:readText(path)
  --- @type ImageAsset
  local data = json.decode(str)
  return data
end

function AssetManager.default()
  local am = {}

  am.assets = {}
  am.loaders = {
    image = imageLoader
  }

  return setmetatable(am, manager_mt)
end

function AssetManager:destroy()
end

return AssetManager
