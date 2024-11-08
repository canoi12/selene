--- @meta
--- @class sdl
--- @field INIT_EVERYTHING string
--- @field INIT_SENSOR string
--- @field INIT_AUDIO string
--- @field INIT_VIDEO string
--- @field INIT_JOYSTICK string
--- @field INIT_GAMECONTROLLER string
--- @field INIT_TIMER string
--- @field INIT_EVENTS string
--- @field AUDIO_S16 string
--- @field AUDIO_S16SYS string
--- @field AUDIO_F32 string
--- @field QUIT string
--- @field APP_TERMINATING string
--- @field APP_LOWMEMORY string
--- @field APP_WILLENTERBACKGROUND string
--- @field APP_DIDENTERBACKGROUND string
--- @field APP_WILLENTERFOREGROUND string
--- @field APP_DIDENTERFOREGROUND string
--- @field LOCALECHANGED string
--- @field DISPLAYEVENT string
--- @field WINDOWEVENT string
--- @field KEYDOWN string
--- @field KEYUP string
--- @field TEXTEDITING string
--- @field TEXTINPUT string
--- @field KEYMAPCHANGED string
--- @field TEXTEDITING_EXT string
--- @field MOUSEMOTION string
--- @field MOUSEBUTTONDOWN string
--- @field MOUSEBUTTONUP string
--- @field MOUSEWHEEL string
--- @field JOYAXISMOTION string
--- @field JOYBALLMOTION string
--- @field JOYHATMOTION string
--- @field JOYBUTTONDOWN string
--- @field JOYBUTTONUP string
--- @field JOYDEVICEADDED string
--- @field JOYDEVICEREMOVED string
--- @field JOYBATTERYUPDATED string
--- @field CONTROLLERAXISMOTION string
--- @field CONTROLLERBUTTONDOWN string
--- @field CONTROLLERBUTTONUP string
--- @field CONTROLLERDEVICEADDED string
--- @field CONTROLLERDEVICEREMOVED string
--- @field CONTROLLERDEVICEREMAPPED string
--- @field CONTROLLERTOUCHPADDOWN string
--- @field CONTROLLERTOUCHPADMOTION string
--- @field CONTROLLERTOUCHPADUP string
--- @field CONTROLLERSENSORUPDATE string
--- @field FINGERDOWN string
--- @field FINGERUP string
--- @field FINGERMOTION string
--- @field DOLLARGESTURE string
--- @field DOLLARRECORD string
--- @field MULTIGESTURE string
--- @field CLIPBOARDUPDATE string
--- @field DROPFILE string
--- @field DROPTEXT string
--- @field DROPBEGIN string
--- @field DROPCOMPLETE string
--- @field AUDIODEVICEADDED string
--- @field AUDIODEVICEREMOVED string
--- @field SENSORUPDATE string
--- @field RENDER_TARGETS_RESET string
--- @field RENDER_DEVICE_RESET string
--- @field RENDERER_ACCELERATED string
--- @field RENDERER_TARGETTEXTURE string
--- @field WINDOWEVENT_CLOSE string
--- @field WINDOWEVENT_SIZE_CHANGED string
--- @field WINDOWEVENT_RESIZED string
--- @field WINDOWEVENT_MOVED string
--- @field WINDOWEVENT_MINIMIZED string
--- @field WINDOWEVENT_MAXIMIZED string
--- @field WINDOWEVENT_RESTORED string
--- @field WINDOWEVENT_SHOWN string
--- @field WINDOWEVENT_HIDDEN string
--- @field WINDOWEVENT_ENTER string
--- @field WINDOWEVENT_LEAVE string
--- @field WINDOWEVENT_FOCUS_GAINED string
--- @field WINDOWEVENT_FOCUS_LOST string
--- @field WINDOWEVENT_TAKE_FOCUS string
--- @field WINDOWEVENT_HIT_TEST string
--- @field WINDOW_SHOWN string
--- @field WINDOW_OPENGL string
--- @field WINDOW_RESIZABLE string
--- @field WINDOW_FULLSCREEN string
--- @field WINDOW_FULLSCREEN_DESKTOP string
--- @field WINDOW_BORDERLESS string
--- @field WINDOW_ALWAYS_ON_TOP string
--- @field WINDOWPOS_CENTERED string
--- @field MESSAGEBOX_ERROR string
--- @field MESSAGEBOX_WARNING string
--- @field MESSAGEBOX_INFORMATION string
--- @field GL_DOUBLEBUFFER string
--- @field GL_DEPTH_SIZE string
--- @field GL_STENCIL_SIZE string
--- @field GL_CONTEXT_PROFILE_MASK string
--- @field GL_CONTEXT_MAJOR_VERSION string
--- @field GL_CONTEXT_MINOR_VERSION string
--- @field GL_CONTEXT_PROFILE_CORE string
--- @field GL_CONTEXT_PROFILE_COMPATIBILITY string
--- @field GL_CONTEXT_PROFILE_ES string
--- @field TEXTUREACCESS_STATIC string
--- @field TEXTUREACCESS_STREAMING string
--- @field TEXTUREACCESS_TARGET string
--- @field PIXELFORMAT_UNKNOWN string
--- @field PIXELFORMAT_RGB332 string
--- @field PIXELFORMAT_RGB444 string
--- @field PIXELFORMAT_RGB555 string
--- @field PIXELFORMAT_BGR555 string
--- @field PIXELFORMAT_ARGB4444 string
--- @field PIXELFORMAT_RGBA4444 string
--- @field PIXELFORMAT_ABGR4444 string
--- @field PIXELFORMAT_BGRA4444 string
--- @field PIXELFORMAT_RGB565 string
--- @field PIXELFORMAT_BGR565 string
--- @field PIXELFORMAT_RGB24 string
--- @field PIXELFORMAT_BGR24 string
--- @field PIXELFORMAT_RGB888 string
--- @field PIXELFORMAT_BGR888 string
--- @field PIXELFORMAT_ARGB8888 string
--- @field PIXELFORMAT_RGBA8888 string
--- @field PIXELFORMAT_ABGR8888 string
--- @field PIXELFORMAT_BGRA8888 string
--- @field PIXELFORMAT_RGBA32 string
--- @field PIXELFORMAT_ARGB32 string
--- @field PIXELFORMAT_BGRA32 string
--- @field PIXELFORMAT_ABGR32 string
--- @field BLENDMODE_NONE string
--- @field BLENDMODE_BLEND string
--- @field BLENDMODE_ADD string
--- @field BLENDMODE_MOD string
--- @field BLENDMODE_MUL string
--- @field BLENDMODE_INVALID string
--- @field FLIP_NONE string
--- @field FLIP_VERTICAL string
--- @field FLIP_HORIZONTAL string
--- @field ScaleModeNearest string
--- @field ScaleModeLinear string
--- @field ScaleModeBest string
sdl = {}
-- Core
--- @param flags integer
function sdl.init(flags) end
function sdl.quit() end
-- Platforms
function sdl.is_android_tv() end
function sdl.is_chromebook() end
function sdl.is_dex_mode() end
function sdl.is_tablet() end
-- AudioDeviceID
--- @param device string
--- @param is_capture boolean
--- @param specs table
function sdl.open_audio_device(device, is_capture, specs) end
--- @param is_capture boolean
function sdl.gen_num_audio_devices(is_capture) end
--- @param index integer
--- @param is_capture boolean
function sdl.get_audio_device_name(index, is_capture) end
-- AudioStream
--- @param in_spec table
--- @param out_spec table
function sdl.new_audio_stream(in_spec, out_spec) end
-- Event
function sdl.create_event() end
-- Font
--- @param renderer sdlRenderer
--- @param font_data FontData
function sdl.create_font(renderer, font_data) end
-- Gamepad
--- @param index integer
function sdl.gamepad_open(index) end
--- @param joystick integer
function sdl.is_gamepad(joystick) end
--- @param axis_name string
function sdl.gamepad_get_axis_from_string(axis_name) end
--- @param button_name string
function sdl.gamepad_get_button_from_string(button_name) end
-- GLContext
--- @param win sdlWindow
function sdl.create_gl_context(win) end
--- @param win sdlWindow
--- @param ctx sdlGLContext
function sdl.gl_make_current(win, ctx) end
function sdl.get_gl_proc_address_func() end
--- @param proc_name string
function sdl.gl_get_proc_address(proc_name) end
--- @param attr integer
--- @param value integer
function sdl.gl_set_attribute(attr, value) end
--- @param value boolean
function sdl.gl_set_swap_interval(value) end
function sdl.gl_get_swap_interval() end
-- Joystick
--- @param id integer
function sdl.joystick_open(id) end
function sdl.num_joysticks() end
-- Renderer
--- @param win sdlWindow
--- @param index integer|nil
--- @param flags integer
function sdl.create_renderer(win, index, flags) end
-- RWops
--- @param path string
function sdl.load_file(path) end
--- @param path string
function sdl.read_file(path) end
--- @param path string
--- @param mode string
function sdl.rw_from_file(path, mode) end
-- Texture
--- @param render sdlRenderer
--- @param data_or_format ImageData|integer
--- @param access integer|nil
--- @param width integer|nil
--- @param height integer|nil
function sdl.create_texture(render, data_or_format, access, width, height) end
-- Window
--- @param title string
--- @param x integer
--- @param y integer
--- @param width integer
--- @param height integer
--- @param flags integer
function sdl.create_window(title, x, y, width, height, flags) end
-- Clipboard
function sdl.get_clipboard_text() end
function sdl.has_clipboard_text() end
--- @param text string
function sdl.set_clipboard_text(text) end
-- Filesystem
function sdl.get_base_path() end
function sdl.get_pref_path() end
-- Shared
--- @param filename string
function sdl.load_object(filename) end
--- @param handle lightuserdata
function sdl.unload_object(handle) end
--- @param handle lightuserdata
--- @param name string
function sdl.load_function(handle, name) end
-- Keyboard
function sdl.get_keyboard_state() end
--- @param keys lightuserdata
--- @param index integer
function sdl.check_key(keys, index) end
function sdl.has_screen_keyboard_support() end
--- @param win sdlWindow
function sdl.is_screen_keyboard_shown(win) end
--- @param name string
function sdl.get_scancode_from_name(name) end
--- @param scancode integer
function sdl.get_scancode_name(scancode) end
--- @param name string
function sdl.get_key_from_name(name) end
--- @param keycode integer
function sdl.get_key_name(keycode) end
function sdl.start_text_input() end
function sdl.stop_text_input() end
-- Mouse
--- @return integer, integer
function sdl.get_mouse_position() end
--- @return integer, integer
function sdl.get_relative_mouse_position() end
--- @param button integer
--- @return boolean
function sdl.is_mouse_down(button) end
-- Timer
--- @return integer
function sdl.get_ticks() end
--- @param ms integer
function sdl.delay(ms) end
--- @return integer
function sdl.get_performance_counter() end
--- @return integer
function sdl.get_performance_frequency() end
-- Video
function sdl.enable_screen_saver() end
-- Error
--- @return string
function sdl.get_error() end
--- @param msg string
function sdl.set_error(msg) end
-- System
--- @return string
function sdl.get_platform() end
--- @return integer
function sdl.get_cpu_cache_line_size() end
--- @return integer
function sdl.get_cpu_count() end
--- @return integer
function sdl.get_system_ram() end

--- @class sdlAudioDeviceID
local AudioDeviceID = {}
--- @param pause boolean
function AudioDeviceID:pause(pause) end
function AudioDeviceID:close() end

--- @class sdlAudioStream
local AudioStream = {}
function AudioStream:clear() end
function AudioStream:free() end
function AudioStream:flush() end
--- @param dev AudioDeviceID
function AudioStream:bind(dev) end
function AudioStream:unbind() end
function AudioStream:put() end
--- @param out Data
--- @return integer
function AudioStream:get(out) end
function AudioStream:available() end

--- @class sdlEvent
local Event = {}
--- @return boolean
function Event:poll() end
--- @return integer
function Event:get_type() end
--- @return integer, integer, integer, integer
function Event:window_event() end
--- @return integer, integer, integer, integer, integer, integer
function Event:audio_device_event() end
--- @return integer, integer, integer, integer, integer, integer
function Event:mouse_motion_event() end
--- @return integer, integer, integer, integer, integer
function Event:mouse_button_event() end
--- @return integer, integer, integer, boolean
function Event:mouse_wheel_event() end
--- @return integer, integer, boolean, integer, integer
function Event:keyboard_event() end
--- @return integer, integer, integer
function Event:joy_hat_event() end
--- @return string
function Event:text_event() end
--- @return string, integer, integer
function Event:text_edit_event() end
--- @return integer
function Event:gamepad_device_event() end

--- @class sdlGamepad
local Gamepad = {}
function Gamepad:get_name() end
function Gamepad:get_vendor() end
function Gamepad:get_product() end
function Gamepad:get_product_version() end
function Gamepad:get_axis() end
function Gamepad:get_button() end
function Gamepad:rumble() end
function Gamepad:current_power_level() end
function Gamepad:close() end

--- @class sdlGLContext
local GLContext = {}
function GLContext:destroy() end

--- @class sdlJoystick
local Joystick = {}
function Joystick:get_name() end
function Joystick:get_vendor() end
function Joystick:get_product() end
function Joystick:get_product_version() end
function Joystick:get_type() end
function Joystick:get_type_string() end
function Joystick:num_axes() end
function Joystick:num_balls() end
function Joystick:num_hats() end
function Joystick:num_buttons() end
function Joystick:get_axis() end
function Joystick:get_ball() end
function Joystick:get_hat() end
function Joystick:get_button() end
function Joystick:rumble() end
function Joystick:current_power_level() end
function Joystick:close() end

--- @class sdlRenderer
local Renderer = {}
function Renderer:destroy() end
function Renderer:clear() end
function Renderer:set_color() end
function Renderer:set_target() end
function Renderer:set_clip_rect() end
function Renderer:set_blend_mode() end
function Renderer:draw_point() end
function Renderer:draw_line() end
function Renderer:draw_triangle() end
function Renderer:fill_triangle() end
function Renderer:draw_rect() end
function Renderer:fill_rect() end
function Renderer:draw_circle() end
function Renderer:fill_circle() end
function Renderer:copy() end
function Renderer:copy_ex() end
function Renderer:print() end
function Renderer:flush() end
function Renderer:present() end

--- @class sdlRWops
local RWops = {}
--- @param data lightuserdata
--- @param size integer
--- @param maxnum integer|nil
--- @return integer
function RWops:read(data, size, maxnum) end
--- @param data lightuserdata
--- @param size integer
--- @param num integer|nil
--- @return integer
function RWops:write(data, size, num) end
function RWops:seek() end
function RWops:size() end
function RWops:tell() end
function RWops:close() end

--- @class sdlTexture
local Texture = {}
function Texture:destroy() end
--- @return integer, integer, integer, integer
function Texture:query() end
function Texture:set_alpha_mod() end
function Texture:set_color_mod() end
function Texture:set_blend_mode() end
--- @param scale_mode integer
function Texture:set_scale_mode(scale_mode) end

--- @class sdlWindow
local Window = {}
function Window:destroy() end
--- @return integer, integer
function Window:get_size() end
function Window:set_size() end
--- @return integer, integer
function Window:get_position() end
function Window:set_position() end
function Window:gl_swap() end
function Window:set_bordered() end
function Window:maximize() end
function Window:minimize() end
function Window:restore() end
function Window:show_simple_message_box() end

