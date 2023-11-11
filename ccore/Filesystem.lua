local sdl = selene.sdl2
local Filesystem

function Filesystem.create(path)
    asset(type(path) == "string", "Invalid path")

    local fs = {}

    local os = selene.system.getOS()
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
    fs.basePath = path

    return setmetatable(fs, {
        __index = Filesystem
    })
end

function Filesystem:resolve(path)
    if selene.system.getOS() == "Windows" then
        path = path:gsub("%/", "\\")
    else
        path = path:gsub("%\\", "/")
    end
    return self.basePath .. path
end

function Filesystem:exists(path)
    return selene.fs.exists(self:resolve(path))
end

function Filesystem:open(path: string)
    return selene.fs.File.open(self:resolve(path))
end

function Filesystem:read(path) 
    return selene.fs.read(self:resolve(path))
end

function Filesystem:write(path, text)
    return selene.fs.write(self:resolve(path), text)
end

function Filesystem:createDir(path)
    selene.fs.createDir(self:resolve(path))
end

function Filesystem:removeDir(path)
    selene.fs.removeDir(self:resolve(path))
end

function Filesystem:load(path)
    return selene.fs.load(self:resolve(path))
end

return Filesystem