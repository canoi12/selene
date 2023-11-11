local class = require 'core.class'
local graphics = require 'core.graphics'
local keyboard = require 'core.keyboard'
local Image = require 'core.graphics.Image'
local Sound = require 'core.audio.Sound'
local Rect = require 'core.graphics.Rect'
local Player = class:extend('Player')

function Player:constructor(x, y)
  self.image = Image('sprites.png')
  self.rect = Rect(0, 192, 16, 16)
  self.jump_sound = Sound('sound.wav')
  self.sound_timer = 0
  self.x = x or 0
  self.y = y or 0
  self.velocity = {
    x = 0,
    y = 0
  }
  self.speed = 80
  self.gravity = 120
  self.jump_force = 60
end

function Player:update(dt)
  if keyboard.is_down('a') then
    self.velocity.x = -self.speed
  elseif keyboard.is_down('d') then
    self.velocity.x = self.speed
  else
    self.velocity.x = 0
  end

  if self.y + 16 < 190 then
    self.velocity.y = self.velocity.y + (self.gravity * dt)
  else
    self.velocity.y = 0
    self.y = 190 - 16
  end

  if keyboard.is_down('space') then
    self.velocity.y = -self.jump_force
    if self.sound_timer <= 0 then
      self.jump_sound:play()
      self.sound_timer = 0.3
    end
  else
    self.sound_timer = self.sound_timer - dt
  end

  self.x = self.x + (self.velocity.x * dt)
  self.y = self.y + (self.velocity.y * dt)
end

function Player:draw()
  graphics.draw(self.image, self.rect, self.x, self.y)
end

return Player
