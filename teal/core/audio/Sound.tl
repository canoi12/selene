local App = require('core.App')
local record Sound
    playing: boolean
    looping: boolean
    offset: integer
    size: integer

    load: function(App, string) : Sound
end

function Sound.load(app: App, path: string) : Sound
    local decoder = selene.audio.Decoder.load(path)
    local sys = app.audioSystem
    local read = decoder:getChunk(sys.auxData, sys.spec.samples)
    if read < 0 then
        error('Decoder error')
    end
    local spec = sys.spec
    local stream = selene.sdl2.AudioStream.create(
        selene.sdl2.AUDIO_S16SYS, decoder:getChannels(), decoder:getSampleRate(),
        spec.format, spec.channels, spec.sampleRate
    )
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
    decoder:close()
    local sound : Sound = {}

    return setmetatable(sound, { __index = Sound })
end

return Sound