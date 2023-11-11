local _tl_compat; if (tonumber((_VERSION or ''):match('[%d.]*$')) or 0) < 5.3 then local p, m = pcall(require, 'compat53.module'); if p then _tl_compat = m end end; local debug = _tl_compat and _tl_compat.debug or debug; local package = _tl_compat and _tl_compat.package or package; local pcall = _tl_compat and _tl_compat.pcall or pcall; local sdl = selene.sdl2
local traceback = debug.traceback
local App = require('core.App')









local core = {}

if not sdl.init(
   sdl.INIT_SENSOR, sdl.INIT_AUDIO,
   sdl.INIT_VIDEO, sdl.INIT_TIMER,
   sdl.INIT_JOYSTICK, sdl.INIT_GAMECONTROLLER,
   sdl.INIT_EVENTS) then

   error("Failed to init SDL2: " .. sdl.getError())
end

local args = selene.args
local projectPath = args[2] or "./"
package.path = projectPath .. '?.lua;' ..
projectPath .. '?/init.lua;' ..
package.path

local app

local state, ret = pcall(function() return require('main') end)
if state then
   app = ret
else
   app = App.createError(ret)
end

return {
   step = function() app:step() end,
   quit = function() app:destroy() end,
}
