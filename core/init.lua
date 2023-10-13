local sdl = selene.sdl2
local event = require 'core.event'
local filesystem = require 'core.filesystem'
local graphics = require 'core.graphics'
local audio = require 'core.audio'
local traceback = debug.traceback
local core = {}

local config = {
  audio = {
    sample_rate = 44100,
    channels = 2,
    samples = 2048,
    callback = selene.audio.GetCallback()
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
  local w, h = graphics.window:GetSize()
  graphics.set_size(w, h)
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
    local width, height = graphics.window:GetSize()
    graphics.set_color(255, 255, 255)
    graphics.print('error', 16)
    graphics.print_wrap(msg, 16, 16, width)
    graphics.print_wrap(trace, 16, 32, width)
  end
  selene.key_callback = function(pressed, key)
    if key == 'escape' then
      selene.SetRunning(false)
    end
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
  local basepath = "."
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
  print(
    selene.system.GetOS(),
    selene.system.GetArch()
  )

  selene.config(config)

  graphics.init(config)
  local state = xpcall(function()
      event.init()
      audio.init(config)
  end, _error)
  if not state then
    return nil
  end

  if filesystem.exists('main.lua') then
    xpcall(function() require('main') end, _error)
  else
    selene.update = function() end
    selene.draw = function()
      graphics.clear()
      local width, height = graphics.window:GetSize()
      graphics.print('no main.lua found', width / 2 - 54, height / 2)
    end
    selene.key_callback = function(pressed, key)
      if key == 'escape' then
        selene.SetRunning(false)
      end
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
  audio.deinit()
  graphics.deinit()
end

return core
