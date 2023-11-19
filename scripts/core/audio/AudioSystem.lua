--- @type selene.sdl2
local sdl = selene.sdl2
local SoundInstance = require('audio.SoundInstance')
--- @class AudioSystem
--- @field music Music | nil
--- @field sounds table
--- @field device selene.sdl2.AudioDeviceID
--- @field spec AudioSpec
--- @field pool table
local AudioSystem = {}

local system_mt = {}
system_mt.__index = AudioSystem

--- Creates a new audio system
--- @return AudioSystem
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

--- Destroy audio system
function AudioSystem:destroy()
    if self.device then
        self.device:pause(false)
        self.device:close()
    end
end

function AudioSystem:update()
    if self.music and self.music.playing then
        --- @type Music
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
                    music.stream:unbind(self.device)
                    self.music = nil
                end
            end
        else
            local res = music.stream:put(self.auxData:getPointer(), read * 4)
            if res < 0 then
                error('Stream put error: ' .. sdl.getError())
            end
        end
    end

    local sounds_to_remove = {}
    for i,sound in ipairs(self.sounds) do
        if sound.playing then
            local stream = sound.stream
            local size = sound.size
            local len = self.spec.size
            if sound.offset + len > size then
                len = size - sound.offset
            end
            local res = 0
            if len ~= 0 then
                res = stream:put(sound.data:getPointer(sound.offset), len)
                sound.offset = sound.offset + len
            end
            if res < 0 then
                error('Stream put error: ' .. sdl.getError())
            elseif stream:available() == 0 then
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

--- Play music
--- @param music Music
function AudioSystem:playMusic(music)
    if self.music then
        self.music.stream:unbind(self.device)
    end
    if music then music.playing = true end
    self.music = music
    self.music.stream:bind(self.device)
end

function AudioSystem:stopMusic()
    if self.music then
        self.music.stream:unbind(self.device)
        self.music = nil
    end
end

function AudioSystem:playSound(sound)
    local instance = SoundInstance.create(table.remove(self.pool), sound.data)
    instance.stream:bind(self.device)
    table.insert(self.sounds, instance)
    return instance
end

--- Stop a sound instance
--- @param instance SoundInstance
function AudioSystem:stopSound(instance)
    for i=#self.sounds,1,-1 do
        if self.sounds[i] == instance then
            table.remove(self.sounds, i)
            break
        end
    end
    instance.stream:unbind(self.device)
    table.insert(self.pool, instance.stream)
end

return AudioSystem
