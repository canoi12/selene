local audio = {}
local sdl = selene.sdl2

local function get_first_device()
  local num = sdl.getNumAudioDevice(false)
  if num > 0 then
    return sdl.getAudioDeviceName(0)
  end
  error("No audio device found")
end

function audio.init(config)
  -- selene.audio.RegisterEngine(audio)
  local num = sdl.AudioDeviceID.getCount(false)
  if num <= 0 then
    error("No audio device found")
  end
  audio.dev, audio.spec = sdl.AudioDeviceID.open(nil, false, config.audio)
  if not audio.dev then
    local name = sdl.AudioDeviceID.getName(0, false)
    audio.dev, audio.spec = sdl.AudioDeviceID.open(name, false, config.audio)
    if not audio.dev then
      error("Failed to open audio device: " .. sdl.getError())
    end
  end
  audio.music = nil
  audio.sounds = {}
  -- audio.pool = selene.audio.NewBufferPool(256)
  -- selene.audio.SetBufferPool(audio.pool)
  audio.aux_data = selene.Data.create(audio.spec.size)
  audio.pool = {}
  for i=1,64 do
    table.insert(audio.pool, 
      sdl.AudioStream.create(
        audio.spec.format, audio.spec.channels, audio.spec.sample_rate,
        audio.spec.format, audio.spec.channels, audio.spec.sample_rate
      )
    )
  end
  audio.dev:pause(false)
end

function audio.deinit()
  if audio.dev then
    audio.dev:pause(true)
    audio.dev:close()
  end
  -- audio.pool:Free()
end

function audio.set_volume(volume)
end

function audio.set_pitch(pitch)
end

function audio.load_source(path)
end

function audio.update()
  if audio.music and audio.music.playing then
    local music = audio.music
    local read = music.decoder:GetChunk(audio.aux_data, audio.spec.samples)
    if read < 0 then
      error('Error decoding music')
    elseif read == 0 then
      local wait = music.stream:Available()
      if wait == 0 then
        if music.loop then
          music.decoder:Seek(0)
        else
          music.stream:Unbind()
          audio.music = nil
        end
      end
    else
      local res = music.stream:Put(music.chunk:GetPointer(), read * 4)
      if res < 0 then
        error('Stream put error: ' .. sdl.GetError())
      end
    end
  end

  local sounds_to_remove = {}
  for i,sound in ipairs(audio.sounds) do
    if sound.playing then
      local stream = sound.stream
      local size = sound.size
      local len = audio.spec.size
      if sound.offset + len > size then
        len = size - sound.offset
      end
      local res = 0
      if len ~= 0 then
        res = stream:Put(sound.data:GetPointer(sound.offset), len)
        sound.offset = sound.offset + len
      end
      if res < 0 then
        error('Stream put error: ' .. sdl.GetError())
      elseif stream:Available() == 0 then
        if sound.loop then sound.offset = 0 
        else table.insert(sounds_to_remove, i) end
      end
    end
  end

  for i,s in ipairs(sounds_to_remove) do
    local sound = table.remove(audio.sounds, s)
    sound.stream:Unbind(audio.dev)
    table.insert(audio.pool, sound.stream)
  end
end

function audio.play(obj)
  obj.stream:Bind(audio.dev)
end

function audio.pause(buffer)
end

function audio.stop(buffer)
  for i,b in ipairs(audio.buffers) do
    if b == buffer then
      table.remove(audio.buffers, i)
      break
    end
  end
  table.insert(audio.buffer_pool, buffer)
end

return audio
