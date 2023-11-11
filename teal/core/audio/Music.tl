local record Music
    playing: boolean
    looping: boolean
    chunk: selene.Data
    decoder: selene.audio.Decoder
    stream: selene.sdl2.AudioStream

    metamethod __index: Music
    metamethod __gc: function(Music)
end

local music_mt : metatable<Music> = {}
music_mt.__index = Music
music_mt.__gc = function(s: Music)
    s.chunk:free()
    s.decoder:close()
    s.stream:free()
end

return Music