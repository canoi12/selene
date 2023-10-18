local audio = {}
local sdl = selene.sdl2

local function get_first_device()
  local num = sdl.GetNumAudioDevice(false)
  if num > 0 then
    return sdl.GetAudioDeviceName(0)
  end
  error("No audio device found")
end

function audio.init(config)
  -- selene.audio.RegisterEngine(audio)
  local num = sdl.AudioDeviceID.GetCount(false)
  if num <= 0 then
    error("No audio device found")
  end
  audio.dev, audio.spec = sdl.AudioDeviceID.Open(nil, false, config.audio)
  if not audio.dev then
    local name = sdl.AudioDeviceID.GetName(0, false)
    audio.dev, audio.spec = sdl.AudioDeviceID.Open(name, false, config.audio)
    if not audio.dev then
      error("Failed to open audio device: " .. sdl.GetError())
    end
  end
  -- audio.pool = selene.audio.NewBufferPool(256)
  -- selene.audio.SetBufferPool(audio.pool)
  audio.dev:Pause(false)
end

function audio.deinit()
  if audio.dev then
    audio.dev:Pause(true)
    audio.dev:Close()
  end
  -- audio.pool:Free()
end

function audio.set_volume(volume)
end

function audio.set_pitch(pitch)
end

function audio.load_source(path)
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
