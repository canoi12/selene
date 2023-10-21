local audio = require 'core.audio'
local graphics = require 'core.graphics'
local Sound = require 'core.audio.Sound'
local Music = require 'core.audio.Music'
local Sound = require 'core.audio.Sound'

function selene.load()
    selene.Data.__gc = function(d)
        print('Freeing Data')
        d:Free()
    end
    music = Music("music.ogg")
    sound = Sound('sound.wav')
end

local offset = {
    0, 0
}
local function process_sound(index, s)
    local stream = audio.pool[index]
    local size = s.data:GetSize()
    local len = audio.spec.size
    if offset[index] + len > size then
        len = size - offset[index]
    end
    local res = stream:Put(s.data:GetPointer(offset[index]), len)
    offset[index] = offset[index] + len
    if res < 0 then
        error('Stream put error')
    end
end

local function process_music(obj)
    local read = obj.decoder:GetChunk(obj.chunk, audio.spec.samples)
    if read < 0 then
        error('Decoder error')
    end
    local res = obj.stream:Put(obj.chunk:GetPointer(), read * 4)
    if res < 0 then
        error('Stream put error')
    end
end

function selene.update(dt)
    -- process_audio(music)
    -- process_sound(1, sound)
    -- process_music(music)
end

function selene.draw()
    graphics.clear()
    graphics.print('playing musics..')
end

function selene.key_callback(pressed, key, is_repeat)
    if pressed and key == 'space' and not rpt then
        -- sound.decoder:Seek(0)
        local instance = sound:play()
        print(sound, instance)
        -- audio.play(sound)
        -- audio.pool:Pause(instance, pause)
        pause = not pause
      end
end
