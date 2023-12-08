--- @class AudioSettings
--- @field sampleRate integer
--- @field channels integer
--- @field samples integer
local AudioSettings = {}

--- @class WindowSettings
--- @field title string
--- @field width integer
--- @field height integer
--- @field resizable boolean
--- @field fullscreen boolean
--- @field borderless boolean
--- @field alwaysOnTop boolean
local WindowSettings = {}


--- @class Settings
--- @field org string
--- @field name string
--- @field version string
--- @field audio AudioSettings
--- @field window WindowSettings
local Settings = {}

--- Create new settings
--- @param title string
--- @param width integer
--- @param height integer
---@return Settings
function Settings.create(title, width, height)
    local config = {
        audio = {},
        window = {}
    }

    config.name = title
    config.org = "selene"
    config.version = "0.1.0"

    config.audio.sampleRate = 44100
    config.audio.channels = 2
    config.audio.samples = 4096

    config.window.title = title
    config.window.width = width
    config.window.height = height
    config.window.resizable = false
    config.window.fullscreen = false
    config.window.borderless = false
    config.window.alwaysOnTop = false

    return config
end

--- Default settings
---@return Settings
function Settings.default()
    local config = {
        audio = {},
        window = {}
    }

    config.name = "selene"
    config.org = "selene"
    config.version = "0.1.0"

    config.audio.sampleRate = 44100
    config.audio.channels = 2
    config.audio.samples = 4096

    config.window.title = "selene " .. selene.getVersion()
    config.window.width = 640
    config.window.height = 380
    config.window.resizable = false
    config.window.fullscreen = false
    config.window.borderless = false
    config.window.alwaysOnTop = false

    return config
end

--- Load Settings from a lua file
--- @param path string
--- @param fs Filesystem | nil
function Settings.load(path, fs)
    local cfg = nil
    if fs then
        cfg = fs:load(path)
    else
        cfg = selene.fs.load(path)
    end
    if type(cfg) == "function" then
        return cfg()
    else
        error("Invalid settings path")
    end
end

return Settings
