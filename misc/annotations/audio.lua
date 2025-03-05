--- @meta
--- @class audio
audio = {}

--- @alias audio.Instance integer

--- @class audio.Data : userdata
local AudioData = {}

--- @class audio.Decoder : userdata
local AudioDecoder = {}

--- @class audio.System : userdata
local AudioSystem = {}

function AudioSystem:close() end
function AudioSystem:update() end

--- Play an AudioData or AudioDecoder
--- @param pause boolean
function AudioSystem:pause_device(pause) end

--- Set audio instance volume
--- @param instance audio.Instance
--- @param volume number
function AudioSystem:set_volume(instance, volume) end

--- Set audio instance looping
--- @param instance audio.Instance
--- @param loop boolean
function AudioSystem:set_loop(instance, loop) end

--- Pause an AudioData or AudioDecoder
--- @param data audio.Data | audio.Decoder
--- @param loop boolean|nil
--- @param volume number|nil
function AudioSystem:play(data, loop, volume) end

--- @param spec table
--- @return audio.System
function audio.create_system(spec) end

--- @param path string
--- @return audio.Decoder
function audio.load_decoder(path) end

--- @param path string
--- @return audio.Data
function audio.load_data(path) end