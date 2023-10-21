local audio = require 'core.audio'
local Audio = require 'core.audio.Audio'
local SoundInstance = require 'core.audio.SoundInstance'
local filesystem = require 'core.filesystem'
local Sound = Audio:extend('Sound')

function Sound:constructor(path)
  local decoder = selene.audio.Decoder.Load(filesystem.resolve(path))
  local read = decoder:GetChunk(audio.aux_data, audio.spec.samples)
  if read < 0 then
    error('Decode error')
  end
  local stream = selene.sdl2.AudioStream.Create(
    selene.sdl2.AUDIO_S16SYS, decoder:GetChannels(), decoder:GetSampleRate(),
    audio.spec.format, audio.spec.channels, audio.spec.sample_rate
  )
  local size = read
  while read ~= 0 do
    local res = stream:Put(audio.aux_data:GetPointer(), read * 4)
    if res < 0 then
      error('AudioStream put error: ' .. selene.sdl2.GetError())
    end
    read = decoder:GetChunk(audio.aux_data, audio.spec.samples)
  end
  local size = stream:Available()
  self.data = selene.Data.New(size)
  read = stream:Get(self.data)
  if read < 0 then
    error('AudioStream read error: ' .. selene.sdl2.GetError())
  end
  stream:Free()
  decoder:Close()
end

function Sound:__gc()
  
end

function Sound:play()
  print(self.data)
  local instance = SoundInstance(table.remove(audio.pool), self.data)
  instance.stream:Bind(audio.dev)
  table.insert(audio.sounds, instance)
  return instance
end

function Sound:stop()
  for i,inst in ipairs(audio.sounds) do
    if inst.data == self.data then
      inst:stop()
    end
  end
end

function Sound:pause()
end

return Sound
