--- @type selene.sdl2
local sdl = selene.sdl2
--- @class AudioSystem
--- @field music nil
--- @field sounds table
--- @field device selene.sdl2.AudioDeviceID
local AudioSystem = {}

local system_mt = {}
system_mt.__index = AudioSystem

function AudioSystem.create(settings)
    local num = sdl.AudioDeviceID.getCount(false)
    if num <= 0 then
        error('No audio device found')
    end
    local audio = {}
    audio.device, audio.spec = sdl.AudioDeviceID.open(nil, false, settings.audio)
    if not audio.device then
        local name = sdl.AudioDeviceID.getName(0, false)
        audio.device, audio.spec = sdl.AudioDeviceID.open(name, false, settings.audio)
        if not audio.device then
            error('Failed to open audio device: ' .. sdl.getError())
        end
    end
    audio.music = nil
    audio.sounds = {}

    audio.auxData = selene.Data.create(audio.spec.size)
    audio.pool = {}
    for i=1,64 do
        table.insert(
            audio.pool,
            sdl.AudioStream.create(audio.spec, audio.spec)
        )
    end
    audio.device:pause(false)
    return setmetatable(audio, system_mt)
end

function AudioSystem:destroy()
    if self.device then
        self.device:pause(false)
        self.device:close()
    end
end

function AudioSystem:update()
    if self.music and self.music.playing then
        local music = self.music
        local read = music.decoder:getChunk(self.auxData, self.spec.samples)
        if read < 0 then
            error('Error decoding music')
        elseif read == 0 then
            local wait = music.stream:available()
            if wait == 0 then
                if music.looping then
                    music.decoder:seek(0)
                else
                    music.stream:unbind()
                    self.music = nil
                end
            end
        end
    end

    local sounds_to_remove = {}
    for i,sound in ipairs(audio.sounds) do
        if sound.playing then
            local stream = sound.stream
            local size = sound.size
            local len = self.spec.size
            if sound.offset + len > size then
                len = size - sound.offset
            end
            local res = 0
            if len ~= 0 then
                res = stream:Put(sound.data:GetPointer(sound.offset), len)
                sound.offset = sound.offset + len
            end
            if res < 0 then
                error('Stream put error: ' .. sdl.getError())
            elseif stream:Available() == 0 then
                if sound.loop then sound.offset = 0 
                else table.insert(sounds_to_remove, i) end
            end
        end
    end

    for i,s in ipairs(sounds_to_remove) do
        local sound = table.remove(self.sounds, s)
        sound.stream:unbind(self.device)
        table.insert(self.pool, sound.stream)
    end
end

return AudioSystem