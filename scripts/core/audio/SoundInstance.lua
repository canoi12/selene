--- @class SoundInstance
--- @field playing boolean
--- @field looping boolean
--- @field data selene.Data
--- @field size integer
--- @field stream selene.sdl2.AudioStream
--- @field offset integer
local SoundInstance = {}

local instance_mt = {}
instance_mt.__index = SoundInstance

--- Create a new sound instance
--- @param stream selene.sdl2.AudioStream
--- @param data selene.Data
---@return SoundInstance
function SoundInstance.create(stream, data)
    local s = {}
    s.playing = true
    s.looping = false
    s.data = data
    s.size = data:getSize()
    s.stream = stream
    s.offset = 0
    return setmetatable(s, instance_mt)
end

return SoundInstance