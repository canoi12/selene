local class = require 'core.class'
local audio = require 'core.audio'
local SoundInstance = class:extend('SoundInstance')

function SoundInstance:constructor(stream, data)
    self.playing = true
    self.loop = false
    self.data = data
    self.size = data:GetSize()
    self.stream = stream
    self.offset = 0
end

function SoundInstance:__gc()
end

function SoundInstance:pause(value)
    self.playing = value
end

function SoundInstance:stop()
    for i=1,#audio.sounds do
        if audio.sounds[i] == self then
            table.remove(audio.sounds, i)
            break
        end
    end
    self.stream:Unbind(audio.dev)
    table.insert(audio.pool, self.stream)
end

return SoundInstance