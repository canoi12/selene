local audio = {}
local sdl = selene.sdl2

function audio.init(config)
  local obtained = nil
  audio.dev, audio.spec = sdl.OpenAudioDevice(nil, false, config.audio)
  if not audio.dev then
    error("Failed to open audio device: ", sdl.GetError())
  end
end

function audio.deinit()
  audio.dev:Close()
end

return audio
