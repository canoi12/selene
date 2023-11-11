local record Filesystem
    basePath: string

    create: function(path: string) : Filesystem

    resolve: function(Filesystem, path: string) : string
    exists: function(Filesystem, path: string) : boolean
    open: function(Filesystem, path: string) : selene.fs.File
    read: function(Filesystem, path: string) : selene.Data
    write: function(Filesystem, path: string, text: string) : boolean
    createDir: function(Filesystem, path: string)
    removeDir: function(Filesystem, path: string)
    
    load: function(Filesystem, path: string) : any
end

function Filesystem.create(path: string) : Filesystem
    local fs: Filesystem = {}
    local b1 = '\\'
    local b2 = '/'

    if selene.system.getOS() == "Windows" then
        b1 = '/'
        b2 = '\\'
    end

    path:gsub("%" .. b1, b2)
    if path:sub(#path) ~= b2 then
        path = path .. b2
    end
    fs.basePath = path

    return setmetatable(fs, { __index = Filesystem })
end

function Filesystem:resolve(path: string) : string
    if selene.system.getOS() == "Windows" then
        path = path:gsub("%/", "\\")
    else
        path = path:gsub("%\\", "/")
    end
    return self.basePath .. path 
end

function Filesystem:exists(path: string) : boolean
    selene.fs.exists(self:resolve(path))
end

function Filesystem:open(path: string) : selene.fs.File
    return selene.fs.File.open(self:resolve(path))
end

function Filesystem:read(path: string) : selene.Data
    return selene.fs.read(self:resolve(path))
end

function Filesystem:write(path: string, text: string) : boolean
    return selene.fs.write(self:resolve(path), text)
end

function Filesystem:createDir(path: string)
    selene.fs.createDir(self:resolve(path))
end

function Filesystem:removeDir(path: string)
    selene.fs.removeDir(self:resolve(path))
end

function Filesystem:load(path: string) : any
    return selene.fs.load(self:resolve(path))
end

return Filesystem