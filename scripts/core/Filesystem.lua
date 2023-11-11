local _tl_compat; if (tonumber((_VERSION or ''):match('[%d.]*$')) or 0) < 5.3 then local p, m = pcall(require, 'compat53.module'); if p then _tl_compat = m end end; local os = _tl_compat and _tl_compat.os or os; local string = _tl_compat and _tl_compat.string or string; local sdl = selene.sdl2
local Filesystem = {}















function Filesystem.create(path)
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

   return setmetatable(fs, { __index = Filesystem })
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
   selene.fs.exists(self:resolve(path))
end

function Filesystem:open(path)
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
