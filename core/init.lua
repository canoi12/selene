local sdl = selene.sdl2
local event = require 'core.event'
local filesystem = require 'core.filesystem'
local graphics = require 'core.graphics'
local traceback = debug.traceback
local core = {}

local config = {
  audio = {
    sample_rate = 44100,
    channels = 2,
    samples = 2048,
  },
  window = {
    title = "selene " .. selene.GetVersion(),
    width = 640,
    height = 380,
    resizable = false,
    fullscreen = false,
    borderless = false,
    always_on_top = false,
  }
}

local timer = {
  delta = 0,
  last = 0,
}

function selene.config(t)
end

function selene.quit()
  return true
end

local _error_step = function()
  graphics.begin()
  if selene.update then selene.update() end
  if selene.draw then selene.draw() end
  graphics.finish()
  graphics.swap()
  event.poll()
end

local _error = function(msg)
  local trace = traceback('', 1)
  print(msg, trace)
  selene.update = function() end
  selene.draw = function()
    graphics.clear()
    graphics.set_color(255, 255, 255)
    graphics.print('error', 16)
    graphics.print(msg, 16, 16)
    graphics.print(trace, 16, 32)
  end
  core.loop = _error_step
end

core.handlers = {}

function selene.boot()
  timer.delta = 0
  if selene.load then selene.load(selene.args) end
  return function()
    local current = sdl.GetTicks()
    timer.delta = (current - timer.last) / 1000
    timer.last = current;
    graphics.begin()
    if selene.update then selene.update(timer.delta) end
    if selene.draw then selene.draw() end
    graphics.finish()
    graphics.swap()
    event.poll()
  end
end

function core.init()
  local args = selene.args
  local basepath = "./"
  if #args > 1 then
    basepath = args[2]
  end
  filesystem.set_basepath(basepath)
  local path = filesystem.get_basepath()
  package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path
  table.insert(
    package.searchers,
    function(libname)
      local path = filesystem.get_basepath() .. libname:gsub("%.", "/") .. ".lua"
      return filesystem.read(path)
    end
  )
  if filesystem.exists('conf.lua') then
    xpcall(function() require('conf') end, _error)
  end
  selene.config(config)

  event.init()
  graphics.init(config)

  if not sdl.OpenAudio(config.audio) then
    error('Failed to init SDL2 Audio')
  end

  if filesystem.exists('main.lua') then
    xpcall(function() require('main') end, _error)
  else
    selene.update = function() end
    selene.draw = function()
      graphics.clear(0.3, 0.4, 0.4)
    end
  end

  local state, _step = xpcall(selene.boot, _error)
  if state then
    if type(_step) ~= 'function' then
      _error('selene.boot must return a function')
    else
      local _loop = nil
      core.loop = function() xpcall(_step, _error) end
    end
  end
end

function core.deinit()
  graphics.deinit()
  sdl.CloseAudio()
end

return core
