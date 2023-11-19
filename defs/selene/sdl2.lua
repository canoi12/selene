--- @meta

--- @class AudioSpec
--- @field channels integer
--- @field sampleRate integer
--- @field samples integer
--- @field format integer
--- @field size integer
local AudioSpec = {}

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

--- @class selene.sdl2.AudioStream
local AudioStream = {}

--- Creates a new audio stream
--- @param inSpec AudioSpec
--- @param outSpec AudioSpec
--- @return selene.sdl2.AudioStream
function AudioStream.create(inSpec, outSpec) end

--- @class selene.sdl2
local sdl2 = {}

return sdl2