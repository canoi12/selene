local Image = require('graphics.Image')
local Music = require('graphics.Music')
local Sound = require('graphics.Sound')
local Filesystem = require('Filesystem')
--- @class AssetManager
--- @field assets table
--- @field loaders table
--- @field fs Filesystem
local AssetManager = {}

local manager_mt = {}
manager_mt.__index = AssetManager

--- Default image loader
--- @param manager AssetManager
--- @param asset ImageAsset
--- @return Image
local function imageLoader(manager, asset)
  local rpath = manager.fs:resolve(asset.path)
  return Image.load(rpath)
end

--- Default music loader
--- @param manager AssetManager
--- @param asset MusicAsset
--- @return Music
local function musicLoader(manager, asset)
  local rpath = manager.fs:resolve(asset.path)
  return Music.load(rpath)
end

--- Default sound loader
--- @param manager AssetManager
--- @param asset SoundAsset
--- @return Sound
local function soundLoader(manager, asset)
  local rpath = manager.fs:resolve(asset.path)
  return Sound.load(rpath)
end

function AssetManager.create(path)
  local am = {}

  am.assets = {}
  am.loaders = {
    image = imageLoader,
    music = musicLoader,
    sound = soundLoader,
  }
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

--- Load assets
--- @param type_ string
--- @param name string
--- @return Image | Music | Sound | nil
function AssetManager:load(type_, name)
  if not self.loaders[type_] then return nil end
  local loader = self.loaders[type_]
  return loader(self, self.assets[type_][name])
end

function AssetManager:destroy()
end

return AssetManager
