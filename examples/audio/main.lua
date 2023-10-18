local audio = require 'core.audio'
local graphics = require 'core.graphics'
local Sound = require 'core.audio.Sound'
local Music = require 'core.audio.Music'

function selene.load()
    music = Music("music.ogg")
    print(
        audio.spec.samples,
        audio.spec.channels,
        audio.spec.sample_rate,
        audio.spec.format,
        audio.spec.size
    )
    -- sound = Sound("sound.wav")
    -- instance = audio.play(music)
    sound = Music('sound.wav')
    audio.play(music)
    print(music.stream)
    local dec = music.decoder
    local stream = music.stream
    print(
        'Music',
        dec:GetBitDepth(),
        dec:GetChannels(),
        dec:GetSampleRate()
    )
    dec = sound.decoder
    print(
        'Sound',
        dec:GetBitDepth(),
        dec:GetChannels(),
        dec:GetSampleRate()
    )
end

local function process_audio(obj)
    local read = obj.decoder:GetChunk(obj.chunk, audio.spec.samples)
    if read == 0 then
        obj.decoder:Seek(0)
    end
    if read < 0 then
        error('Decoder error')
    end
    local res = obj.stream:Put(obj.chunk:GetPointer(), read * 4)
    if res < 0 then
        error('Stream put error')
    end
end

function selene.update(dt)
    process_audio(sound)
end

function selene.draw()
    graphics.clear()
    graphics.print('playing musics..')
end

function selene.key_callback(pressed, key, is_repeat)
    if pressed and key == 'space' and not rpt then
        -- audio.play(sound)
        -- audio.pool:Pause(instance, pause)
        pause = not pause
      end
end
