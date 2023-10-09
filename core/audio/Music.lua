local class = require 'core.class'
local audio = require 'core.audio'
local Music = class:extend('Music')

function Music:constructor(path)
  self.pitch = 1
  self.volume = 100
  self.source = nil
  self.instances = {}
end

function Music:play()
  table.insert(self.instances, audio.play(self.source, true))
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
