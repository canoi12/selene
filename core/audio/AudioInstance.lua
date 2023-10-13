local class = require 'core.class'
local AudioInstance = class:AudioInstance('AudioInstance')

function AudioInstance:constructor()
end

function AudioInstance:play()
end

function AudioInstance:pause()
end

function AudioInstance:stop()
end

function AudioInstance:set_volume(volume)
end

function AudioInstance:set_pitch(pitch)
end

return AudioInstance
