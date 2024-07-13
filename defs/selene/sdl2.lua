--- @meta

--- @class AudioSpec
--- @field channels integer
--- @field sampleRate integer
--- @field samples integer
--- @field format integer
--- @field size integer
local AudioSpec = {}

---------------- AudioDeviceID

--- @class selene.sdl2.AudioDeviceID
local AudioDeviceID = {}

--- Open AudioDeviceID
--- @param name string | nil
--- @param isCapture boolean
--- @param spec AudioSpec
--- @return selene.sdl2.AudioDeviceID, AudioSpec
function AudioDeviceID.open(name, isCapture, spec) end

--- Get count of available audio devices
--- @param isCapture boolean
--- @return integer
function AudioDeviceID.getCount(isCapture) end

--- Get name of the audio device by index
--- @param deviceId integer
--- @param isCapture boolean
--- @return string
function AudioDeviceID.getName(deviceId, isCapture) end

--- Pause or unpause the audio device
--- @param pause boolean
function AudioDeviceID:pause(pause) end

--- Close the audio device
function AudioDeviceID:close() end

---------------- AudioStream

--- @class selene.sdl2.AudioStream
local AudioStream = {}

--- Creates a new audio stream
--- @param inSpec AudioSpec
--- @param outSpec AudioSpec
--- @return selene.sdl2.AudioStream
function AudioStream.create(inSpec, outSpec) end

--- Clear audio stream internal buffer
function AudioStream:clear() end

--- Free audio stream
function AudioStream:free() end

---
function AudioStream:flush() end

--- Bind audio stream to device
--- @param device selene.sdl2.AudioDeviceID
function AudioStream:bind(device) end

--- Unbind audio stream to device
--- @param device selene.sdl2.AudioDeviceID
function AudioStream:unbind(device) end

--- Put data to audio stream
--- If a lightuserdata is provided, you'll need to set the size
--- @param data selene.Data | lightuserdata
--- @param size integer | nil
function AudioStream:put(data, size) end

--- Get audio stream converted data
--- @param out selene.Data
--- @param size integer
--- @return integer
function AudioStream:get(out, size) end

--- Get available data in audio stream
--- @return integer
function AudioStream:available() end

---------------- Event

--- @class selene.sdl2.Event
local Event = {}

--- Create a new SDL2 Event
--- @return selene.sdl2.Event
function Event.create() end

--- Poll events
--- @return boolean
function Event:poll() end

--- Event type ID
--- @return integer
function Event:getType() end

--- Return the data of the window event
--- @return integer, integer, integer, integer
function Event:windowEvent() end

--- Return the data of the mouse motion event
--- @return integer, integer, integer, integer, integer, integer
function Event:mouseMotionEvent() end

--- Return the data of the mouse button event
--- @return integer, integer, integer, integer, integer
function Event:mouseButtonEvent() end

--- Return the data of the mouse wheel event
--- @return integer, integer, integer, integer
function Event:mouseWheelEvent() end

--- Return the data of the keyboard event
--- @return integer, integer, integer, integer, integer
function Event:KeyboardEvent() end


---------------- Gamepad

--- @class selene.sdl2.Gamepad
local Gamepad = {}

---------------- GLContext
--- @class selene.sdl2.GLContext
local GLContext = {}

--- Create a new GLContext
--- @param window selene.sdl2.Window
function GLContext.create(window) end

function GLContext:destroy() end

---------------- Joystick

--- @class selene.sdl2.Joystick
local Joystick = {}

---------------- Window

--- @class selene.sdl2.Window
local Window = {}

--- @param title string
--- @param x integer
--- @param y integer
--- @param width integer
--- @param height integer
--- @param flags table
--- @return selene.sdl2.Window
function Window.create(title, x, y, width, height, flags) end

--- @return integer, integer
function Window:getSize() end

--- Swap window
function Window:swap() end

--- Destroy window
function Window:destroy() end

---------------- SDL2

--- @class selene.sdl2
--- @field AudioDeviceID selene.sdl2.AudioDeviceID
--- @field AudioStream selene.sdl2.AudioStream
--- @field Event selene.sdl2.Event
--- @field Gamepad selene.sdl2.Gamepad
--- @field GLContext selene.sdl2.GLContext
--- @field Joystick selene.sdl2.Joystick
--- @field Window selene.sdl2.Window
-- Enums
--- @field INIT_EVERYTHING integer
--- @field INIT_SENSOR integer
--- @field INIT_AUDIO integer
--- @field INIT_VIDEO integer
--- @field INIT_JOYSTICK integer
--- @field INIT_GAMECONTROLLER integer
--- @field INIT_TIMER integer
--- @field INIT_EVENTS integer
-- Audio
--- @field AUDIO_S16 integer
--- @field AUDIO_S16SYS integer
--- @field AUDIO_F32 integer
-- Application events
--- @field QUIT integer
--- @field APP_TERMINATING integer
--- @field APP_LOWMEMORY integer
--- @field APP_WILLENTERBACKGROUND integer
--- @field APP_DIDENTERBACKGROUND integer
--- @field APP_WILLENTERFOREGROUND integer
--- @field APP_DIDENTERFOREGROUND integer
--- @field LOCALECHANGED integer
-- Display events
--- @field DISPLAYEVENT integer
-- Window events
--- @field WINDOWEVENT integer
-- Keyboard events
--- @field KEYDOWN integer
--- @field KEYUP integer
--- @field TEXTEDITING integer
--- @field TEXTINPUT integer
--- @field KEYMAPCHANGED integer
--- @field TEXTEDITING_EXT integer
-- Mouse events
--- @field MOUSEMOTION integer
--- @field MOUSEBUTTONDOWN integer
--- @field MOUSEBUTTONUP integer
--- @field MOUSEWHEEL integer
-- Joystick events
--- @field JOYAXISMOTION integer
--- @field JOYBALLMOTION integer
--- @field JOYHATMOTION integer
--- @field JOYBUTTONDOWN integer
--- @field JOYBUTTONUP integer
--- @field JOYDEVICEADDED integer
--- @field JOYDEVICEREMOVED integer
--- @field JOYBATTERYUPDATED integer
-- Game controller events
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
-- Touch events
--- @field FINGERDOWN integer
--- @field FINGERUP integer
--- @field FINGERMOTION integer
-- Gesture events
--- @field DOLLARGESTURE integer
--- @field DOLLARRECORD integer
--- @field MULTIGESTURE integer
-- Clipboard events
--- @field CLIPBOARDUPDATE integer
-- Drag and drop events
--- @field DROPFILE integer
--- @field DROPTEXT integer
--- @field DROPBEGIN integer
--- @field DROPCOMPLETE integer
-- Audio hotplug events
--- @field AUDIODEVICEADDED integer
--- @field AUDIODEVICEREMOVED integer
-- Sensor events
--- @field SENSORUPDATE integer
-- Render events
--- @field RENDER_TARGETS_RESET integer
--- @field RENDER_DEVICE_RESET integer
-- Window Events
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
-- Window Flags
--- @field WINDOW_SHOWN integer
--- @field WINDOW_OPENGL integer
--- @field WINDOW_RESIZABLE integer
--- @field WINDOW_FULLSCREEN integer
--- @field WINDOW_FULLSCREEN_DESKTOP integer
--- @field WINDOW_BORDERLESS integer
--- @field WINDOW_ALWAYS_ON_TOP integer
--- @field WINDOWPOS_CENTERED integer
-- Message Box
--- @field MESSAGEBOX_ERROR integer
--- @field MESSAGEBOX_WARNING integer
--- @field MESSAGEBOX_INFORMATION integer
-- GL
--- @field GL_DOUBLEBUFFER integer
--- @field GL_DEPTH_SIZE integer
--- @field GL_STENCIL_SIZE integer
--- @field GL_CONTEXT_PROFILE_MASK integer
--- @field GL_CONTEXT_MAJOR_VERSION integer
--- @field GL_CONTEXT_MINOR_VERSION integer
--- @field GL_CONTEXT_PROFILE_CORE integer
--- @field GL_CONTEXT_PROFILE_COMPATIBILITY integer
--- @field GL_CONTEXT_PROFILE_ES integer
local sdl2 = {}

--- Init SDL2
--- @param ... integer
--- @return boolean
function sdl2.init(...) end

--- Quit SDL2
function sdl2.quit() end

-- OpenGL

--- Set window current OpenGL context
--- @param window selene.sdl2.Window
--- @param context selene.sdl2.GLContext
function sdl2.glMakeCurrent(window, context) end

--- @return lightuserdata
function sdl2.glGetProcAddress() end

--- @param attr integer
--- @param value integer
function sdl2.glSetAttribute(attr, value) end

--- @param value boolean
function sdl2.glSetSwapInterval(value) end

--- @return boolean
function sdl2.getGetSwapInterval() end

-- Clipboard

--- @return string
function sdl2.getClipboardText() end

--- @return boolean
function sdl2.hasClipboardText() end

--- @param text string
function sdl2.setClipboardText(text) end

-- Filesystem

--- Get path from selene executable
--- @return string
function sdl2.getBasePath() end

--- Get safe use path to write
--- @param org string
--- @param name string
--- @return string
function sdl2.getPrefPath(org, name) end

-- Shared object
--- Load a shared object
--- @param name string
--- @return lightuserdata
function sdl2.loadObject(name) end

--- Unload previous loaded object
--- @param data lightuserdata
function sdl2.unloadObject(data) end

function sdl2.loadFunction(pointer, string) end

-- Keyboard
function sdl2.checkKeyState(key) end

--- @return lightuserdata
function sdl2.getKeyboardState() end

--- @return boolean
function sdl2.hasScreenKeyboardSupport() end

--- Check if screen keyboard is actually active
--- @param win selene.sdl2.Window
--- @return boolean
function sdl2.isScreenKeyboardShown(win) end

--- @param name string
--- @return integer
function sdl2.getScancodeFromName(name) end

--- @param key integer
--- @return string
function sdl2.getScancodeName(key) end

-- Mouse
--- @return number, number
function sdl2.getMousePosition() end

--- @return number, number
function sdl2.getRelativeMousePosition() end

--- @param btn integer
--- @return boolean
function sdl2.isMouseDown(btn) end

-- Timer
--- @return integer
function sdl2.getTicks() end

--- @param n number
function sdl2.delay(n) end

--- @return integer
function sdl2.getPerformanceCounter() end

--- @return integer
function sdl2.getPerformanceFrequency() end

-- Error
--- @return string
function sdl2.getError() end

--- @param text string
function sdl2.setError(text) end

-- System
--- @return string
function sdl2.getPlatform() end

--- @return integer
function sdl2.getCPUCacheLineSize() end

--- @return integer
function sdl2.getCPUCount() end

--- @return integer
function sdl2.getSystemRAM() end

return sdl2
