--- @meta
--- @class sdl
--- @field INIT_EVERYTHING integer
--- @field INIT_SENSOR integer
--- @field INIT_AUDIO integer
--- @field INIT_VIDEO integer
--- @field INIT_JOYSTICK integer
--- @field INIT_GAMECONTROLLER integer
--- @field INIT_TIMER integer
--- @field INIT_EVENTS integer
--- @field AUDIO_S16 integer
--- @field AUDIO_S16SYS integer
--- @field AUDIO_F32 integer
--- @field QUIT integer
--- @field APP_TERMINATING integer
--- @field APP_LOWMEMORY integer
--- @field APP_WILLENTERBACKGROUND integer
--- @field APP_DIDENTERBACKGROUND integer
--- @field APP_WILLENTERFOREGROUND integer
--- @field APP_DIDENTERFOREGROUND integer
--- @field LOCALECHANGED integer
--- @field DISPLAYEVENT integer
--- @field WINDOWEVENT integer
--- @field KEYDOWN integer
--- @field KEYUP integer
--- @field TEXTEDITING integer
--- @field TEXTINPUT integer
--- @field KEYMAPCHANGED integer
--- @field TEXTEDITING_EXT integer
--- @field MOUSEMOTION integer
--- @field MOUSEBUTTONDOWN integer
--- @field MOUSEBUTTONUP integer
--- @field MOUSEWHEEL integer
--- @field JOYAXISMOTION integer
--- @field JOYBALLMOTION integer
--- @field JOYHATMOTION integer
--- @field JOYBUTTONDOWN integer
--- @field JOYBUTTONUP integer
--- @field JOYDEVICEADDED integer
--- @field JOYDEVICEREMOVED integer
--- @field JOYBATTERYUPDATED integer
--- @field CONTROLLERAXISMOTION integer
--- @field CONTROLLERBUTTONDOWN integer
--- @field CONTROLLERBUTTONUP integer
--- @field CONTROLLERDEVICEADDED integer
--- @field CONTROLLERDEVICEREMOVED integer
--- @field CONTROLLERDEVICEREMAPPED integer
--- @field CONTROLLERTOUCHPADDOWN integer
--- @field CONTROLLERTOUCHPADMOTION integer
--- @field CONTROLLERTOUCHPADUP integer
--- @field CONTROLLERSENSORUPDATE integer
--- @field FINGERDOWN integer
--- @field FINGERUP integer
--- @field FINGERMOTION integer
--- @field DOLLARGESTURE integer
--- @field DOLLARRECORD integer
--- @field MULTIGESTURE integer
--- @field CLIPBOARDUPDATE integer
--- @field DROPFILE integer
--- @field DROPTEXT integer
--- @field DROPBEGIN integer
--- @field DROPCOMPLETE integer
--- @field AUDIODEVICEADDED integer
--- @field AUDIODEVICEREMOVED integer
--- @field SENSORUPDATE integer
--- @field RENDER_TARGETS_RESET integer
--- @field RENDER_DEVICE_RESET integer
--- @field RENDERER_ACCELERATED integer
--- @field RENDERER_TARGETTEXTURE integer
--- @field WINDOWEVENT_CLOSE integer
--- @field WINDOWEVENT_SIZE_CHANGED integer
--- @field WINDOWEVENT_RESIZED integer
--- @field WINDOWEVENT_MOVED integer
--- @field WINDOWEVENT_MINIMIZED integer
--- @field WINDOWEVENT_MAXIMIZED integer
--- @field WINDOWEVENT_RESTORED integer
--- @field WINDOWEVENT_SHOWN integer
--- @field WINDOWEVENT_HIDDEN integer
--- @field WINDOWEVENT_ENTER integer
--- @field WINDOWEVENT_LEAVE integer
--- @field WINDOWEVENT_FOCUS_GAINED integer
--- @field WINDOWEVENT_FOCUS_LOST integer
--- @field WINDOWEVENT_TAKE_FOCUS integer
--- @field WINDOWEVENT_HIT_TEST integer
--- @field WINDOW_SHOWN integer
--- @field WINDOW_OPENGL integer
--- @field WINDOW_RESIZABLE integer
--- @field WINDOW_FULLSCREEN integer
--- @field WINDOW_FULLSCREEN_DESKTOP integer
--- @field WINDOW_BORDERLESS integer
--- @field WINDOW_ALWAYS_ON_TOP integer
--- @field WINDOWPOS_CENTERED integer
--- @field MESSAGEBOX_ERROR integer
--- @field MESSAGEBOX_WARNING integer
--- @field MESSAGEBOX_INFORMATION integer
--- @field GL_DOUBLEBUFFER integer
--- @field GL_DEPTH_SIZE integer
--- @field GL_STENCIL_SIZE integer
--- @field GL_CONTEXT_PROFILE_MASK integer
--- @field GL_CONTEXT_MAJOR_VERSION integer
--- @field GL_CONTEXT_MINOR_VERSION integer
--- @field GL_CONTEXT_PROFILE_CORE integer
--- @field GL_CONTEXT_PROFILE_COMPATIBILITY integer
--- @field GL_CONTEXT_PROFILE_ES integer
--- @field TEXTUREACCESS_STATIC integer
--- @field TEXTUREACCESS_STREAMING integer
--- @field TEXTUREACCESS_TARGET integer
--- @field PIXELFORMAT_UNKNOWN integer
--- @field PIXELFORMAT_RGB332 integer
--- @field PIXELFORMAT_RGB444 integer
--- @field PIXELFORMAT_RGB555 integer
--- @field PIXELFORMAT_BGR555 integer
--- @field PIXELFORMAT_ARGB4444 integer
--- @field PIXELFORMAT_RGBA4444 integer
--- @field PIXELFORMAT_ABGR4444 integer
--- @field PIXELFORMAT_BGRA4444 integer
--- @field PIXELFORMAT_RGB565 integer
--- @field PIXELFORMAT_BGR565 integer
--- @field PIXELFORMAT_RGB24 integer
--- @field PIXELFORMAT_BGR24 integer
--- @field PIXELFORMAT_RGB888 integer
--- @field PIXELFORMAT_BGR888 integer
--- @field PIXELFORMAT_ARGB8888 integer
--- @field PIXELFORMAT_RGBA8888 integer
--- @field PIXELFORMAT_ABGR8888 integer
--- @field PIXELFORMAT_BGRA8888 integer
--- @field PIXELFORMAT_RGBA32 integer
--- @field PIXELFORMAT_ARGB32 integer
--- @field PIXELFORMAT_BGRA32 integer
--- @field PIXELFORMAT_ABGR32 integer
--- @field BLENDMODE_NONE integer
--- @field BLENDMODE_BLEND integer
--- @field BLENDMODE_ADD integer
--- @field BLENDMODE_MOD integer
--- @field BLENDMODE_MUL integer
--- @field BLENDMODE_INVALID integer
--- @field FLIP_NONE integer
--- @field FLIP_VERTICAL integer
--- @field FLIP_HORIZONTAL integer
--- @field ScaleModeNearest integer
--- @field ScaleModeLinear integer
--- @field ScaleModeBest integer
local sdl = {}

--- @param flags integer
--- @return boolean
function sdl.init(flags) end

function sdl.quit() end

--- @return string
function sdl.get_error() end

--- @return boolean
function sdl.is_android_tv() end

--- @return boolean
function sdl.is_chromebook() end

--- @return boolean
function sdl.is_dex_mode() end

--- @return boolean
function sdl.is_tablet() end

--- @param device string
--- @param is_capture boolean
--- @param specs table
--- @return sdlAudioDeviceID
function sdl.open_audio_device(device, is_capture, specs) end

--- @param is_capture boolean
--- @return integer
function sdl.get_num_audio_devices(is_capture) end

--- @param index integer
--- @param is_capture boolean
--- @return integer
function sdl.get_audio_device_name(index, is_capture) end

--- @param in_spec table
--- @param out_spec table
--- @return sdlAudioStream
function sdl.new_audio_stream(in_spec, out_spec) end

--- @return sdlEvent
function sdl.create_event() end

--- @param renderer sdlRenderer
--- @param font_data FontData
--- @return sdlFont, sdlTexture
function sdl.create_font(renderer, font_data) end

--- @param index integer
--- @return sdlGamepad
function sdl.gamepad_open(index) end

--- @param joystick integer
--- @return boolean
function sdl.is_gamepad(joystick) end

--- @param name string
--- @return integer
function sdl.gamepad_get_axis_from_string(name) end

--- @param name string
--- @return integer
function sdl.gamepad_get_button_from_string(name) end

--- @param win sdlWindow
--- @return sdlGLContext
function sdl.create_gl_context(win) end

--- @param win sdlWindow
--- @param ctx sdlGLContext
function sdl.gl_make_current(win, ctx) end

--- @return lightuserdata
function sdl.get_gl_proc_address_func() end

--- @param name string
--- @return lightuserdata
function sdl.gl_get_proc_address(name) end

--- @param attr integer
--- @param value integer
function sdl.gl_set_attribute(attr, value) end

--- @param value boolean
--- @return boolean
function sdl.gl_set_swap_interval(value) end

--- @return integer
function sdl.gl_get_swap_interval() end

--- @param id integer
--- @return sdlJoystick|nil
function sdl.joystick_open(id) end

--- @return integer
function sdl.num_joysticks() end

--- @param win sdlWindow
--- @param index integer|nil
--- @param flags integer|nil
--- @return sdlRenderer
function sdl.create_renderer(win, index, flags) end

--- @param path string
--- @return lightuserdata, integer
function sdl.load_file(path) end

--- @param path string
--- @return sdlRWops
function sdl.read_file(path) end

--- @param path string
--- @param mode string
--- @return sdlRWops
function sdl.rw_from_file(path, mode) end

--- @param render sdlRenderer
--- @param data_or_format ImageData|integer
--- @param access integer|nil
--- @param width integer|nil
--- @param height integer|nil
--- @return sdlTexture
function sdl.create_texture(render, data_or_format, access, width, height) end

--- @param title string
--- @param x integer
--- @param y integer
--- @param width integer
--- @param height integer
--- @param flags integer
--- @return sdlWindow
function sdl.create_window(title, x, y, width, height, flags) end

--- @return string
function sdl.get_clipboard_text() end

--- @return boolean
function sdl.has_clipboard_text() end

--- @param text string
function sdl.set_clipboard_text(text) end

--- @return string
function sdl.get_base_path() end

--- @param org string
--- @param app string
--- @return string
function sdl.get_pref_path(org, app) end

--- @param so_file string
--- @return lightuserdata
function sdl.load_object(so_file) end

--- @param handle lightuserdata
function sdl.unload_object(handle) end

--- @param handle lightuserdata
--- @param name string
--- @return lightuserdata
function sdl.load_function(handle, name) end

--- @return lightuserdata
function sdl.get_keyboard_state() end

--- @param handle lightuserdata
--- @param key integer
function sdl.check_key(handle, key) end

--- @return boolean
function sdl.had_screen_keyboard_support() end

--- @param win sdlWindow
--- @return boolean
function sdl.is_screen_keyboard_shown(win) end

--- @param name string
--- @return integer
function sdl.get_scancode_from_name(name) end

--- @param scancode integer
--- @return string
function sdl.get_scancode_name(scancode) end

--- @param name string
--- @return integer
function sdl.get_key_from_name(name) end

--- @param keycode integer
--- @return string
function sdl.get_key_name(keycode) end

function sdl.start_text_input() end

function sdl.stop_text_input() end

--- @return number, number
function sdl.get_mouse_position() end

--- @return number, number
function sdl.get_relative_mouse_position() end

--- @param button integer
--- @return boolean
function sdl.is_mouse_down(button) end

--- @return integer
function sdl.get_ticks() end

--- @param ms uinteger
function sdl.delay(ms) end

--- @return integer
function sdl.get_performance_counter() end

--- @return integer
function sdl.get_performance_frequency() end

function sdl.set_timer() end

--- @class sdlAudioDeviceID
local sdlAudioDeviceID = {}

--- @param pause boolean
function sdlAudioDeviceID:pause(pause) end

function sdlAudioDeviceID:close() end

--- @class sdlAudioStream
local sdlAudioStream = {}

function sdlAudioStream:clear() end

function sdlAudioStream:free() end

function sdlAudioStream:flush() end

--- @param dev sdlAudioDeviceID
function sdlAudioStream:bind(dev) end

--- @param dev sdlAudioDeviceID
function sdlAudioStream:unbind(dev) end

--- @return integer
function sdlAudioStream:put() end

--- @param out Data
--- @return integer
function sdlAudioStream:get(out) end

--- @return integer
function sdlAudioStream:available() end

--- @class sdlEvent
local sdlEvent = {}

--- @return boolean
function sdlEvent:poll() end

--- @return integer
function sdlEvent:get_type() end

--- @return integer, integer, integer, integer
function sdlEvent:window_event() end

--- @return integer, integer, integer, integer, integer, integer
function sdlEvent:mouse_motion_event() end

--- @return integer, integer, integer, integer, integer
function sdlEvent:mouse_button_event() end

--- @return integer, integer, integer, integer
function sdlEvent:mouse_wheel_event() end

--- @param out Data
--- @return integer, integer, boolean, integer, integer
function sdlEvent:keyboard_event(out) end

--- @return integer
function sdlEvent:audio_device_event() end

--- @return integer
function sdlEvent:gamepad_device_event() end

--- @return integer, integer, integer
function sdlEvent:gamepad_axis_event() end

--- @return integer, integer, integer
function sdlEvent:gamepad_button_event() end

--- @return integer
function sdlEvent:joy_device_event() end

--- @return integer, integer, integer
function sdlEvent:joy_axis_event() end

--- @return integer, integer, integer, integer
function sdlEvent:joy_ball_event() end

--- @return integer, integer, integer
function sdlEvent:joy_button_event() end

--- @return integer, integer, integer
function sdlEvent:joy_hat_event() end

--- @return integer, integer, integer, integer, integer, integer
function sdlEvent:dollar_gesture_event() end

--- @return integer
function sdlEvent:text_event() end

--- @return integer, integer, integer
function sdlEvent:text_edit_event() end

--- @class sdlGamepad
local sdlGamepad = {}

--- @return string
function sdlGamepad:get_name() end

--- @return integer
function sdlGamepad:get_vendor() end

--- @return integer
function sdlGamepad:get_product() end

--- @return integer
function sdlGamepad:get_product_version() end

--- @param axis integer
--- @return number
function sdlGamepad:get_axis(axis) end

--- @param button integer
--- @return boolean
function sdlGamepad:get_button(button) end

--- @param low integer
--- @param high integer
--- @param freq integer|nil
--- @return number
function sdlGamepad:rumble(low, high, freq) end

--- @return string
function sdlGamepad:current_power_level() end

function sdlGamepad:close() end

--- @class sdlGLContext
local sdlGLContext = {}

function sdlGLContext:destroy() end

--- @class sdlJoystick
local sdlJoystick = {}

--- @return string
function sdlJoystick:get_name() end

--- @return integer
function sdlJoystick:get_vendor() end

--- @return integer
function sdlJoystick:get_product() end

--- @return integer
function sdlJoystick:get_product_version() end

--- @return integer
function sdlJoystick:get_type() end

--- @return string
function sdlJoystick:get_type_string() end

--- @return integer
function sdlJoystick:num_axes() end

--- @return integer
function sdlJoystick:num_balls() end

--- @return integer
function sdlJoystick:num_hats() end

--- @return integer
function sdlJoystick:num_buttons() end

--- @param axis integer
--- @return integer
function sdlJoystick:get_axis(axis) end

--- @param ball integer
--- @return integer, integer
function sdlJoystick:get_ball(ball) end

--- @param hat integer
--- @return integer
function sdlJoystick:get_hat(hat) end

--- @param button integer
--- @return integer
function sdlJoystick:get_button(button) end

--- @param low integer
--- @param high integer
--- @param freq integer|nil
--- @return boolean
function sdlJoystick:rumble(low, high, freq) end

--- @return integer
function sdlJoystick:current_power_level() end

function sdlJoystick:close() end

--- @class sdlRenderer
local sdlRenderer = {}

function sdlRenderer:destroy() end

function sdlRenderer:clear() end

--- @param r integer
--- @param g integer
--- @param b integer
--- @param a integer
function sdlRenderer:set_color(r, g, b, a) end

--- @param tex sdlTexture|nil
function sdlRenderer:set_target(tex) end

--- @param rect table|nil
function sdlRenderer:set_clip_rect(rect) end

--- @param blend_mode integer
function sdlRenderer:set_blend_mode(blend_mode) end

--- @param x number
--- @param y number
function sdlRenderer:draw_point(x, y) end

--- @param x0 number
--- @param y0 number
--- @param x1 number
--- @param y1 number
function sdlRenderer:draw_line(x0, y0, x1, y1) end

--- @param x0 number
--- @param y0 number
--- @param x1 number
--- @param y1 number
--- @param x2 number
--- @param y2 number
function sdlRenderer:draw_triangle(x0, y0, x1, y1, x2, y2) end

--- @param x0 number
--- @param y0 number
--- @param x1 number
--- @param y1 number
--- @param x2 number
--- @param y2 number
function sdlRenderer:fill_triangle(x0, y0, x1, y1, x2, y2) end

--- @param x number
--- @param y number
--- @param w number
--- @param h number
function sdlRenderer:draw_rect(x, y, w, h) end

--- @param x number
--- @param y number
--- @param w number
--- @param h number
function sdlRenderer:fill_rect(x, y, w, h) end

--- @param x number
--- @param y number
--- @param radius number
--- @param sides integer|nil
function sdlRenderer:draw_circle(x, y, radius, sides) end

--- @param x number
--- @param y number
--- @param radius number
--- @param sides integer|nil
function sdlRenderer:fill_circle(x, y, radius, sides) end

--- @param tex sdlTexture
--- @param src table|nil
--- @param dest table|nil
function sdlRenderer:copy(tex, src, dest) end

--- @param tex sdlTexture
--- @param src table|nil
--- @param dest table|nil
function sdlRenderer:copy_ex(tex, src, dest) end

--- @param font sdlFont
--- @param text string
--- @param x number|nil
--- @param y number|nil
function sdlRenderer:print(font, text, x, y) end

function sdlRenderer:flush() end

function sdlRenderer:present() end

--- @class sdlRWops
local sdlRWops = {}

--- @param data lightuserdata
--- @param size integer
--- @param maxnum integer|nil
--- @return integer
function sdlRWops:read(data, size, maxnum) end

--- @param data lightuserdata
--- @param size integer
--- @param num integer|nil
--- @return integer
function sdlRWops:writen(data, size, num) end

--- @param offset integer
--- @param whence integer
--- @return integer
function sdlRWops:seek(offset, whence) end

--- @return integer
function sdlRWops:size() end

--- @return integer
function sdlRWops:tell() end

--- @return boolean
function sdlRWops:close() end

--- @class sdlTexture
local sdlTexture = {}

function sdlTexture:destroy() end

--- @return integer, integer, integer, integer
function sdlTexture:query() end

--- @param alpha integer
function sdlTexture:set_alpha_mod(alpha) end

--- @param r integer
--- @param g integer
--- @param b integer
function sdlTexture:set_color_mod(r, g, b) end

--- @param blend_mode integer
function sdlTexture:set_blend_mode(blend_mode) end

--- @param scale_mode integer
function sdlTexture:set_scale_mode(scale_mode) end

--- @class sdlWindow
local sdlWindow = {}

function sdlWindow:destroy() end

--- @return integer, integer
function sdlWindow:get_size() end

function sdlWindow:set_size() end

--- @return integer, integer
function sdlWindow:get_position() end

function sdlWindow:set_position() end

function sdlWindow:gl_swap() end

--- @class sdlFont
local sdlFont = {}

