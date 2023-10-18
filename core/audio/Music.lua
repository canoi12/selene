local sdl = selene.sdl2
local class = require 'core.class'
local audio = require 'core.audio'
local filesystem = require 'core.filesystem'
local Music = class:extend('Music')

function Music:constructor(path)
  self.decoder = selene.audio.Decoder.Load(filesystem.resolve(path))
  self.stream = sdl.AudioStream.Create(
    sdl.AUDIO_S16, self.decoder:GetChannels(), self.decoder:GetSampleRate(),
    sdl.AUDIO_S16, audio.spec.channels, audio.spec.sample_rate
  )
  self.chunk = selene.Data.New(audio.spec.size)
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
