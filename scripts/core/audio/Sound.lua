--- @class Sound
--- @field playing boolean
--- @field looping boolean
--- @field offset integer
--- @field size integer
local Sound = {}

local sound_mt = {}
sound_mt.__index = Sound
sound_mt.__gc = function (s)
    s.data:free()
end

function Sound.create(sys, decoder)
    local sound = {}

    local read = decoder:getChunk(sys.auxData, sys.spec.samples)
    if read < 0 then
        error('Decoder error')
    end
    local spec = {
        format = selene.sdl2.AUDIO_S16SYS,
        channels = decoder:getChannels(),
        sampleRate = decoder:getSampleRate()
    }
    local stream = selene.sdl2.AudioStream.create(spec, sys.spec)
    while read ~= 0 do
        local res = stream:put(sys.auxData:getPointer(), read * 4)
        if res < 0 then
            error('AudioStream put error: ' .. selene.sdl2.getError())
        end
        read = decoder:getChunk(sys.auxData, sys.spec.samples)
    end
    local size = stream:available()
    local data = selene.Data.create(size)
    read = stream:get(data)
    if read < 0 then
       error('AudioStream read error: ' .. selene.sdl2.getError()) 
    end
    stream:free()

    sound.data = data
    return setmetatable(sound, sound_mt)
end

function Sound.load(sys, path)
    local decoder = selene.audio.Decoder.load(path)
    return Sound.create(sys, decoder)
end

return Sound
