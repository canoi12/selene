local audio = {}
local sdl = selene.sdl2

audio.buffers = {}
audio.buffer_pool = {}

function audio.init(config)
  -- selene.audio.RegisterEngine(audio)
  local obtained = nil
  audio.dev, audio.spec = sdl.OpenAudioDevice(nil, false, config.audio)
  if not audio.dev then
    error("Failed to open audio device: ", sdl.GetError())
  end
  -- audio.dev:Pause(false)
end

function audio.deinit()
  audio.dev:Close()
end

function audio.load_source(path)
end

function audio.play(source)
  table.insert(audio.sources, selene.audio.NewBuffer())
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
