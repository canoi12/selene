local audio = require 'core.audio'
local graphics = require 'core.graphics'
local Sound = require 'core.audio.Sound'
local Music = require 'core.audio.Music'

function selene.load()
    music = Music("music.ogg")
    sound = Sound("sound.wav")
    instance = audio.play(music)
end

function selene.update(dt)
end

function selene.draw()
    graphics.clear()
    graphics.print('playing musics..')
end

function selene.key_callback(pressed, key, is_repeat)
    if pressed and key == 'space' and not rpt then
        audio.play(sound)
        audio.pool:Pause(instance, pause)
        pause = not pause
      end
end