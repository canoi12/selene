local class = require 'core.class'
local filesystem = require 'core.filesystem'
local Sound = class:extend('Sound')

function Sound:constructor(path)
  self.source = selene.audio.LoadWav(filesystem.resolve(path))
end

return Sound
