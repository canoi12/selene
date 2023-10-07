local filesystem = {}

local basepath = "./"

function filesystem.set_basepath(path)
  local os = selene.system.GetOS()
  if os == "Windows" then
    if path:sub(#path) == '/' then
      path[#path] = '\\'
    end
    if path:sub(#path) ~= "\\" then
      path = path .. "\\"
    end
  else
    if path:sub(#path) ~= "/" then
      path = path .. "/"
    end
  end
  basepath = path
end

function filesystem.get_basepath()
  return basepath
end

function filesystem.open(path, mode)
  return selene.fs.open(basepath .. path)
end

function filesystem.resolve(path)
  if selene.system.GetOS() == "Windows" then
    path = path:gsub("%/", "\\")
  else
    path = path:gsub("%\\", "/")
  end
  return basepath .. path
end

function filesystem.exists(path)
  return selene.fs.exists(basepath .. path)
end

function filesystem.read(path)
  return selene.fs.read(basepath .. path)
end

function filesystem.write(path, text)
  return selene.fs.write(basepath .. path, text)
end

function filesystem.mkdir(path)
  selene.fs.mkdir(basepath .. path)
end

function filesystem.rmdir(path)
  selene.fs.rmdir(basepath .. path)
end

function filesystem.load(path)
  return selene.fs.load(basepath .. path)
end

return filesystem
