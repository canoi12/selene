selene{meta={org='Selene', name='AudioExample'}}
local audio_system = selene.create_audio_system()
audio_system:resume_device()
print(audio_system)

-- Use filesystem protocol for assets (audio module is global)
local dec = audio.load_decoder('root://music.ogg')
local d = audio.load_data('root://sound.wav')
print(dec)

local inst = audio_system:play(dec, true, 0.1)
local playing = true
local keys = {}

selene.set_event(function(name, ...)
    if name == 'quit' or name == 'window closed' then
        selene.set_running(false)
    elseif name == 'key' then
        local key, pressed, repeating = ...
        keys[string.lower(key)] = pressed
        if key == 'A' and pressed and not repeating then
            audio_system:play(d, false, 0.4)
        elseif key == 'D' and pressed and not repeating then
            playing = not playing
            audio_system:pause(inst, playing)
        end
    end
end)

selene.set_step(function()
    audio_system:update()
    -- Create a minimal window for rendering the clear color
    -- Note: In a real app, you'd have a window and renderer
    selene.delay(16)
end)

selene.set_quit(function()
    audio_system:close()
end)