local sdl = selene.sdl2
local Config = require('core.Config')
local Sound = require('core.audio.Sound')
local Music = require('core.audio.Music')
local record audio
    record AudioSystem

        device: sdl.AudioDeviceID
        spec: sdl.AudioSpec
        music: Music | nil
        sounds: {Sound}
        auxData: selene.Data
        pool: {sdl.AudioStream}

        destroy: function(AudioSystem)
    end

    current: AudioSystem | nil

    create: function(Config) : AudioSystem

    setCurrent: function(AudioSystem)
end

local audio_mt : metatable<audio.AudioSystem> = {}
audio_mt.__index = audio.AudioSystem

function audio.create(cfg: Config) : audio.AudioSystem
    local num = sdl.AudioDeviceID.getCount(false)
    if num <= 0 then
        error('No audio device found')
    end
    local sys : audio.AudioSystem = {}
    local dev, spec = sdl.AudioDeviceID.open(nil, false, cfg.audio as sdl.AudioSpec)
    if not dev then
        local name = sdl.AudioDeviceID.getName(0, false)
        dev, spec = sdl.AudioDeviceID.open(name, false, cfg.audio as sdl.AudioSpec)
        if not dev then
            error('Failed to open audio device: ' .. sdl.getError())
        end
    end
    sys.device = dev
    sys.spec = spec
    sys.music = nil
    sys.sounds = {}
    sys.auxData = selene.Data.create(spec.size as integer)
    sys.pool = {}
    for i=1,64 do
        table.insert(
            sys.pool,
            sdl.AudioStream.create(
                spec.format, spec.channels, spec.sampleRate,
                spec.format, spec.channels, spec.sampleRate
            )
        )
    end
    dev:pause(false)
    return setmetatable(sys, audio_mt)
end

function audio.setCurrent(curr: audio.AudioSystem)
    audio.current = curr
end

function audio.AudioSystem:destroy()
    if self.device then
        self.device:pause(true)
        self.device:close()
    end
    for _,stream in ipairs(self.pool) do
        stream:free()
    end
end

function audio.AudioSystem:update()
    local music = self.music as Music
    if self.music and music.playing then
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
        else
            local res = music.stream:put(music.chunk:getPointer(), read * 4)
            if res < 0 then
                error('Stream put error: ' .. sdl.getError())
            end
        end
    end

    local soundsToRemove : {Sound} = {}
    for i,sound in ipairs(self.sounds) do
        if sound.playing then
            local stream = sound.stream

        end
    end
end

return audio