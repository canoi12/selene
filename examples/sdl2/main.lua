sdl.init(sdl.INIT_VIDEO)
local window = sdl.create_window("SDL2 Test", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED, 640, 380, sdl.WINDOW_SHOWN | sdl.WINDOW_RESIZABLE)
if not window then
    error("failed to create SDL Window: " .. sdl.get_error())
end
local render = sdl.create_renderer(window)
local event = sdl.create_event()

local keys = sdl.get_keyboard_state()
local pos = {8, 64}
print(selene.__dir, selene.__exec)
print(fs.exists(selene.__dir .. '/perceba.jpg'))
print(fs.exists('tuiteiro.jpg'))

local current = sdl.get_ticks()
local last = sdl.get_ticks()

local speed = 320

-- local font_data = selene.FontData.create8x8()
local font_data = font.create8x8()
local ft, image = sdl.create_font(render, font_data)
print(ft, image, font_data.image_width)
render:set_blend_mode(sdl.BLENDMODE_BLEND)
image:set_blend_mode(sdl.BLENDMODE_BLEND)
print(render.set_blend_mode, image.set_blend_mode)
local str = ""
runner.set_step(
function()
    while event:poll() do
        local ev = event:get_type()
        if event:get_type() == sdl.QUIT then runner.set_running(false) end
        if event:get_type() == sdl.WINDOWEVENT then
            if event:window_event() == sdl.WINDOWEVENT_CLOSE then runner.set_running(false) end
        end
        if ev == sdl.KEYUP then
            local scan = event:keyboard_event()
            if sdl.get_scancode_from_name('backspace') == scan then
                str = string.sub(str, 1, #str-1)
            end
        end
        if event:get_type() == sdl.TEXTINPUT then
            local text = event:text_event()
            str = str .. text
        end
    end
    last = current
    current = sdl.get_ticks()
    local delta = (current - last) / 1000
    render:set_color(75, 75, 125, 255)
    render:clear()
    render:set_color(255, 255, 255, 255)
    local w, h = window:get_size()
    if sdl.check_key(keys, sdl.get_scancode_from_name('left')) then
        pos[1] = pos[1] - (speed * delta)
    elseif sdl.check_key(keys, sdl.get_scancode_from_name('right')) then
        pos[1] = pos[1] + (speed * delta)
    end
    render:draw_circle(pos[1], pos[2], 8)
    render:fill_circle(w/2, h/2, 32)
    render:print(ft, 'pois é meu amigo de fé, meu irmão, camarada.\n acho que tá funcionando', 64, 128)
    render:print(ft, str)
    -- render:copy(image, nil, {0, 0, 2048, 8})
    render:present()
end
)

runner.set_quit(
function ()
    image:destroy()
    render:destroy()
    window:destroy()
    sdl.quit()
end
)
