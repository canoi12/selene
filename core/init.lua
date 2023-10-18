local sdl = selene.sdl2
local audio = require 'core.audio'
local event = require 'core.event'
local filesystem = require 'core.filesystem'
local graphics = require 'core.graphics'
local traceback = debug.traceback
local core = {}

local config = {
  audio = {
    sample_rate = 44100,
    channels = 2,
    samples = 4096
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

function selene.quit_callback()
  return true
end

local function _setup_error_callback()
  selene.key_callback = function(pressed, key)
    if key == 'escape' then
      selene.SetRunning(false)
    end
  end
  selene.window_callback = function(ev, wid, d1, d2)
    if ev == "resized" then
      graphics.update_size(d1, d2)
    end
  end
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
  _setup_error_callback()
end

function core.init(args)
  if not sdl.Init(
    sdl.INIT_SENSOR, sdl.INIT_AUDIO,
    sdl.INIT_VIDEO, sdl.INIT_TIMER,
    sdl.INIT_JOYSTICK, sdl.INIT_GAMECONTROLLER,
    sdl.INIT_EVENTS
  ) then
    error("Failed to init SDL2: " .. sdl.GetError())
  end

  filesystem.init(args[2])

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
      graphics.set_font()
      local width, height = graphics.window:GetSize()
      graphics.print('no main.lua found', width / 2 - 68, height / 2 - 4)
    end
    _setup_error_callback()
  end

  local state, _step = xpcall(function()
      timer.delta = 0
      if selene.load then selene.load(args) end
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
    end, _error)

  if state then
    core.step = function() xpcall(_step, _error) end
  end
end

function core.step()
  error("Empty step")
end

function core.quit()
  audio.deinit()
  graphics.deinit()
  sdl.Quit()
end

return core
