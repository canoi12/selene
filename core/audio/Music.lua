local class = require 'core.class'
local audio = require 'core.audio'
local filesystem = require 'core.filesystem'
local Music = class:extend('Music')

function Music:constructor(path)
  self.source = selene.audio.LoadOgg(filesystem.resolve(path))
end

function Music:play()
end

function Music:pause()
end

function Music:stop()
end

function Music:open()
end

function Music:close()
end

return Music
