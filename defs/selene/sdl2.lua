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
--- @return selene.sdl2.AudioDeviceID
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
--- @param data selene.Data
--- @param size integer
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

---------------- SDL2

--- @class selene.sdl2
local sdl2 = {}

--- Init SDL2
--- @param ... integer
--- @return boolean
function sdl2.init(...) end

--- Quit SDL2
function sdl2.quit() end

--- Set window current OpenGL context
--- @param window selene.sdl2.Window
--- @param context selene.sdl2.GLContext
function sdl2.glMakeCurrent(window, context) end

--- Get path from selene executable
--- @return string
function sdl2.getBasePath() end

--- Get safe use path to write
--- @param org string
--- @param name string
--- @return string
function sdl2.getPrefPath(org, name) end

return sdl2
