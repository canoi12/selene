selene('Audio Example', '1.0.0', 'org.selene.AudioExample')
local audio_system = selene.audio.create_system()
audio_system:resume_device()
print(audio_system)
local dec = selene.audio.load_decoder(selene.__dir .. '/music.ogg')
local d = selene.audio.load_data(selene.__dir .. '/sound.wav')
print(dec)
local inst = audio_system:play(dec, true, 0.1)
local playing = true
local events = {}
events['key'] = function(key, pressed, repeating)
    if key == selene.KEY_A and pressed and not repeating then
        audio_system:play(d, false, 0.4)
    elseif key == selene.KEY_D and pressed and not repeating then
        playing = not playing
        audio_system:pause(inst, playing)
    end
end
render:set_projection(function(left, top, bottom, right, near, far)
end)

render:translate()
render:rotate()
render:scale()
render:set_draw_mode('line')
render:push_triangle()

selene.set_event(function(name, ...)
    if name == 'quit' then selene.set_running(false)
    elseif name == 'window closed' then selene.set_running(false) end
    if events[name] then events[name](...) end
end)
selene.set_step(function()
    audio_system:update()
    gl.clear_color(0.3, 0.4, 0.4, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT)
end)
selene.set_quit(function()
    audio_system:close()
    sdl.quit()
end)