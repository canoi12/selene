sdl.init(sdl.INIT_VIDEO | sdl.INIT_AUDIO)
local window = sdl.create_window("SDL2 Test", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED, 640, 380, sdl.WINDOW_SHOWN | sdl.WINDOW_RESIZABLE)
if not window then
    error("failed to create SDL Window: " .. sdl.get_error())
end
local render = sdl.create_renderer(window)
local event = sdl.create_event()

local audio_system = audio.create_system()
audio_system:pause_device(false)
print(audio_system)
local dec = audio.load_decoder(selene.__dir .. '/music.ogg')
local d = audio.load_data(selene.__dir .. '/sound.wav')
print(dec)

audio_system:play(dec, true, 0.1)

runner.set_step(function()
    audio_system:update()
    while event:poll() do
        local t = event:get_type()
        if t == sdl.QUIT then runner.set_running(false) end
        if t == sdl.WINDOWEVENT then
            if t == sdl.WINDOWEVENT_CLOSE then runner.set_running(false) end
        end
        if t == sdl.KEYUP then
            local keyev = {event:keyboard_event()}
            if keyev[1] == sdl.get_scancode_from_name('a') then
                audio_system:play(d, false, 0.4)
            end
        end
    end
    render:set_color(75, 125, 125, 255)
    render:clear()
    render:present()
end)
runner.set_quit(function()
    audio_system:close()
    render:destroy()
    window:destroy()
    sdl.quit()
end)