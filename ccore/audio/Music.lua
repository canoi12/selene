local sdl = selene.sdl2
local audio = require('core.audio')
local Music = {}
local Music_mt = {
  __index = Music,
  __gc = function(m)
    m.decoder:close()
    m.stream:free()
    m.chunk:free()
  end
}

function Music.create(decoder)
  local m = {}
  m.playing = false
  m.loop = false
  m.decoder = decoder
  m.stream = sdl.AudioStream.create(
    sdl.AUDIO_S16SYS, m.decoder:getChannels(), m.decoder:getSampleRate(),
    audio.spec.format, audio.spec.channels, audio.spec.sampleRate
  )
  m.chunk = selene.Data.create(audio.spec.size)
  return setmetatable(m, Music_mt)
end

function Music.load(path)
  local m = {}
  m.playing = true
  m.loop = false
  m.decoder = selene.audio.Decoder.load(path)
  m.stream = sdl.AudioStream.create(
    sdl.AUDIO_S16SYS, m.decoder:getChannels(), m.decoder:getSampleRate(),
    audio.spec.format, audio.spec.channels, audio.spec.sample_rate
  )
  m.chunk = selene.Data.create(audio.spec.size)
  return setmetatable(m, Music_mt)
end

function Music:play()
  if audio.music then
    audio.music.stream:unbind()
  end
  audio.music = self
  self.stream:bind()
end

function Music:pause(value)
  self.playing = value
end

function Music:stop()
  if audio.music then
    audio.music.stream:unbind()
    audio.music = nil
  end
end

function Music:open()
end

function Music:close()
end

return Music
