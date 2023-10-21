local sdl = selene.sdl2
local class = require 'core.class'
local audio = require 'core.audio'
local filesystem = require 'core.filesystem'
local Music = class:extend('Music')

function Music:constructor(path)
  self.playing = true
  self.loop = false
  self.decoder = selene.audio.Decoder.Load(filesystem.resolve(path))
  self.stream = sdl.AudioStream.Create(
    sdl.AUDIO_S16SYS, self.decoder:GetChannels(), self.decoder:GetSampleRate(),
    audio.spec.format, audio.spec.channels, audio.spec.sample_rate
  )
  self.chunk = selene.Data.New(audio.spec.size)
end

function Music:__gc()
  self.decoder:Close()
  self.stream:Free()
  self.chunk:Free()
end

function Music:play()
  if audio.music then
    audio.music.stream:Unbind()
  end
  audio.music = self
  self.stream:Bind()
end

function Music:pause(value)
  self.playing = value
end

function Music:stop()
  if audio.music then
    audio.music.stream:Unbind()
    audio.music = nil
  end
end

function Music:open()
end

function Music:close()
end

return Music
