local sdl = selene.sdl2
local filesystem = {}

local executable_path = "./"
local user_path = "./"

local workspace_path = "./"

function filesystem.init(path)
  path = path or "./"
  local os = selene.system.GetOS()
  local b1 = '\\'
  local b2 = '/'

  if os == "Windows" then
    b1 = '/'
    b2 = '\\'
  end

  path:gsub("%" .. b1, b2)
  if path:sub(#path) ~= b2 then
    path = path .. b2
  end
  workspace_path = path
  package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path

  executable_path = sdl.GetBasePath()
  user_path = sdl.GetPrefPath("selene", "game")
end

function filesystem.get_workspace()
  return workspace_path
end

function filesystem.open(path, mode)
  return selene.fs.File.Open(workspace_path .. path)
end

function filesystem.resolve(path)
  if selene.system.GetOS() == "Windows" then
    path = path:gsub("%/", "\\")
  else
    path = path:gsub("%\\", "/")
  end
  return workspace_path .. path
end

function filesystem.exists(path)
  return selene.fs.Exists(workspace_path .. path)
end

-- function filesystem.read(path)
--   return selene.fs.read(workspace_path .. path)
-- end

-- function filesystem.write(path, text)
--   return selene.fs.write(workspace_path .. path, text)
-- end

function filesystem.mkdir(path)
  selene.fs.CreateDir(workspace_path .. path)
end

function filesystem.rmdir(path)
  selene.fs.RemoveDir(workspace_path .. path)
end

function filesystem.load(path)
  return selene.fs.Load(workspace_path .. path)
end

return filesystem
