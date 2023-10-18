local sdl = selene.sdl2
local event = {}

local ev = nil
local handlers = {
  WindowEvent = function(ev)
    local e, wid, data1, data2 = ev:WindowEvent()
    local callbacks = callback_list[e]
    for i,c in ipairs(callbacks) do
      c(wid, data1, data2)
    end
  end,
}

local window_events = {
  [sdl.WINDOWEVENT_CLOSE] = "close",
  [sdl.WINDOWEVENT_SIZE_CHANGED] = "resized",
  [sdl.WINDOWEVENT_RESIZED] = "resized",
  [sdl.WINDOWEVENT_MOVED] = "moved",
  [sdl.WINDOWEVENT_MINIMIZED] = "minimized",
  [sdl.WINDOWEVENT_MAXIMIZED] = "maximized",
  [sdl.WINDOWEVENT_RESTORED] = "restored",
  [sdl.WINDOWEVENT_SHOWN] = "shown",
  [sdl.WINDOWEVENT_HIDDEN] = "hidden",
  [sdl.WINDOWEVENT_ENTER] = "enter",
  [sdl.WINDOWEVENT_LEAVE] = "leave",
  [sdl.WINDOWEVENT_FOCUS_GAINED] = "focus gained",
  [sdl.WINDOWEVENT_FOCUS_LOST] = "focus lost",
  [sdl.WINDOWEVENT_TAKE_FOCUS] = "take focus",
  [sdl.WINDOWEVENT_HIT_TEST] = "hit test",
}

handlers[sdl.QUIT] = function(ev)
  if selene.quit_callback then
    selene.SetRunning(not selene.quit_callback())
  else
    selene.SetRunning(false)
  end
end

handlers[sdl.WINDOWEVENT] = function(ev)
  local we, wid, data1, data2 = ev:WindowEvent()
  if we == sdl.WINDOWEVENT_CLOSE then
    selene.SetRunning(false)
  end
  if selene.window_callback then
    selene.window_callback(window_events[we], wid, data1, data2)
  end
end

handlers[sdl.KEYDOWN] = function(ev)
  local key, rpt = ev:KeyboardEvent()
  if selene.key_callback then
    selene.key_callback(true, key:lower(), rpt)
  end
end

handlers[sdl.KEYUP] = function(ev)
  local key, rpt = ev:KeyboardEvent()
  if selene.key_callback then
    selene.key_callback(false, key:lower(), rpt)
  end
end

handlers[sdl.MOUSEMOTION] = function(ev)
  local which, state, x, y, xrel, yrel = ev:MouseMotionEvent()
  if selene.mouse_motion_callback then
    selene.mouse_motion_callback(which, state, x, y, xrel, yrel)
  end
end

handlers[sdl.MOUSEBUTTONDOWN] = function(ev)
  local which, state, x, y, clicks = ev:MouseButtonEvent()
  if selene.mouse_button_callback then
    selene.mouse_button_callback(true, which, state, x, y, clicks)
  end
end

handlers[sdl.MOUSEBUTTONUP] = function(ev)
  local which, state, x, y, clicks = ev:MouseButtonEvent()
  if selene.mouse_button_callback then
    selene.mouse_button_callback(false, which, state, x, y, clicks)
  end
end

handlers[sdl.AUDIODEVICEADDED] = function(ev)
  local id = ev:AudioDeviceEvent()
  if selene.audio_device_callback then
    selene.audio_device_callback(true, id)
  end
end

handlers[sdl.AUDIODEVICEREMOVED] = function(ev)
  local id = ev:AudioDeviceEvent()
  if selene.audio_device_callback then
    selene.audio_device_callback(false, id)
  end
end

function event.init()
  ev = sdl.Event.Create()
end

function event.poll()
  while ev:Poll() do
    local tp = ev:GetType()
    if handlers[tp] then handlers[tp](ev) end
  end
end

return event
