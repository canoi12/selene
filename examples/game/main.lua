local graphics = require 'core.graphics'
local Canvas = require 'core.graphics.Canvas'
local Player = require 'player'
local keyboard = require 'core.keyboard'
local filesystem = require 'core.filesystem'

local tl = require 'tl'
tl.loader()
require 'teste'
local Class = require 'Class'
print(Class)
local Enemy = Class:Extend('Enemy')
print(Enemy)


function selene.load(args)
  player = Player(160, 32)
  canvas = Canvas(320, 190)
end

function selene.update(dt)
  player:update(dt)
end

function selene.draw(dt)
  graphics.clear(75, 125, 125)
  graphics.set_canvas(canvas)
  graphics.clear()
  player:draw()
  graphics.set_canvas()
  graphics.draw(canvas, nil, 0, 0, 2, 2)
end
