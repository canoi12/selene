local audio = require('core.audio')
local SoundInstance = require('core.audio.SoundInstance')
local Sound = {}
local Sound_mt = {
  __index = Sound,
  __gc = function(s)
    s.data:free()
  end
}

function Sound.load(path)
  local decoder = selene.audio.Decoder.load(path)
  local read = decoder:getChunk(audio.aux_data, audio.spec.samples)
  if read < 0 then
    error('Decode error')
  end
  local stream = selene.sdl2.AudioStream.create(
    selene.sdl2.AUDIO_S16SYS, decoder:getChannels(), decoder:getSampleRate(),
    audio.spec.format, audio.spec.channels, audio.spec.sample_rate
  )
  local size = read
  while read ~= 0 do
    local res = stream:Put(audio.aux_data:getPointer(), read * 4)
    if res < 0 then
      error('AudioStream put error: ' .. selene.sdl2.getError())
    end
    read = decoder:getChunk(audio.aux_data, audio.spec.samples)
  end
  local size = stream:available()
  self.data = selene.Data.create(size)
  read = stream:get(self.data)
  if read < 0 then
    error('AudioStream read error: ' .. selene.sdl2.getError())
  end
  stream:free()
  decoder:close()
end

function Sound:play()
  -- print(self.data)
  local instance = SoundInstance(table.remove(audio.pool), self.data)
  instance.stream:bind(audio.dev)
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
