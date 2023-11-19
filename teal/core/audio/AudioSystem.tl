local sdl = selene.sdl2
local Config = require('core.Config')
local Music = require('core.audio.Music')
local Sound = require('core.audio.Sound')
local SoundInstance = require('core.audio.SoundInstance')
local record AudioSystem

    device: sdl.AudioDeviceID
    spec: sdl.AudioSpec
    music: Music | nil
    sounds: {Sound}
    auxData: selene.Data
    pool: {sdl.AudioStream}


    create: function(Config) : AudioSystem
    destroy: function(AudioSystem)

    loadMusic: function(AudioSystem, string) : Music
    loadSound: function(AudioSystem, string) : Sound

    playMusic: function(AudioSystem, Music)
    playSound: function(AudioSystem, Sound) : SoundInstance

    stopAll: function(AudioSystem)

    stopMusic: function(AudioSystem)
    stopSound: function(AudioSystem, Sound)
    stopInstance: function(AudioSystem, SoundInstance)
end

local system_mt : metatable<AudioSystem> = {}
system_mt.__index = AudioSystem

function AudioSystem.create(cfg: Config) : AudioSystem
    local sys : AudioSystem = {}
    local num = sdl.AudioDeviceID.getCount(false)
    if num <= 0 then
        error("No audio device found")
    end
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

    return setmetatable(sys, system_mt)
end

function AudioSystem:destroy()
    if self.device then
        self.device:pause(true)
        self.device:close()
    end
    for _,stream in ipairs(self.pool) do
        stream:free()
    end
end

return AudioSystem