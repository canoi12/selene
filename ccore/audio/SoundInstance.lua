local audio = require 'core.audio'
local SoundInstance = {}
local SoundInstance_mt = {
    __index = SoundInstance
}

function SoundInstance.create(stream, data)
    local s = {}
    s.playing = true
    s.loop = false
    s.data = data
    s.size = data:getSize()
    s.stream = stream
    s.offset = 0
    return setmetatable(s, SoundInstance_mt)
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
    self.stream:unbind(audio.dev)
    table.insert(audio.pool, self.stream)
end

return SoundInstance
