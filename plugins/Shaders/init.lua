-- Shader plugin.
--
-- Each shader lives in its own folder (plugins/Shaders/<name>/) with canonical
-- GLSL sources plus generated bin.lua / meta.lua. This module keeps the
-- previous loader API:
--
--   local shaders = require('plugins.Shaders')
--   local vs = shaders.get(renderer, 'sprite2d', 'vertex')
--   local ps = shaders.get(renderer, 'sprite2d', 'pixel')
--   local inputs = shaders.meta('sprite2d', 'vertex')
--
-- A single shader can also be required directly. Its init.lua returns get
-- bound to that shader:
--
--   local sprite2d = require('plugins.Shaders.sprite2d')
--   local vs = sprite2d(renderer, 'vertex')

local shaders = {}

local function load_module(name, suffix)
    local modname = 'plugins.Shaders.' .. name .. suffix
    local ok, data = pcall(require, modname)
    if not ok then
        error(('shaders: failed to load %q (%s)'):format(modname, tostring(data)), 3)
    end
    return data
end

-- Create an engine Shader for the renderer's active backend.
--   stage: 'vertex' | 'pixel'
function shaders.get(renderer, name, stage)
    if type(name) ~= 'string' or name == '' then
        error('shaders.get: expected a shader name', 2)
    end
    local get = load_module(name, '')
    if type(get) ~= 'function' then
        error(('shaders: shader %q did not return a get function'):format(name), 2)
    end
    return get(renderer, stage)
end

-- Return the reflection metadata table for a shader/stage.
function shaders.meta(name, stage)
    if type(name) ~= 'string' or name == '' then
        error('shaders.meta: expected a shader name', 2)
    end
    if stage ~= 'vertex' and stage ~= 'pixel' then
        error("shaders.meta: stage must be 'vertex' or 'pixel'", 2)
    end
    local meta_data = load_module(name, '.meta')
    if type(meta_data) ~= 'table' or type(meta_data[stage]) ~= 'table' then
        error(('shaders: shader %q has no stage %q'):format(name, stage), 2)
    end
    return meta_data[stage]
end

return shaders
