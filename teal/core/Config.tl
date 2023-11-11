local record Config
    record Info
        org: string
        name: string
        version: {integer, integer, integer}
    end
    record Audio
        sampleRate: integer
        channels: integer
        samples: integer
    end
    record Window
        title: string
        width: integer
        height: integer
        resizable: boolean
        fullscreen: boolean
        borderless: boolean
        alwaysOnTop: boolean
    end

    info : Info
    audio : Audio
    window : Window

    create: function(title: string, width: integer, height: integer) : Config
    default: function() : Config
end

function Config.create(title: string, width: integer, height: integer) : Config
    local config : Config = {}
    local audio : Config.Audio = {}
    local info : Config.Info = {}
    local window : Config.Window = {}

    info.org = "selene"
    info.name = title
    info.version = {0, 1, 0}

    audio.sampleRate = 44100
    audio.channels = 2
    audio.samples = 4096

    window.title = title
    window.width = width
    window.height = height
    window.resizable = false
    window.fullscreen = false
    window.borderless = false
    window.alwaysOnTop = false

    config.audio = audio
    config.info = info
    config.window = window

    return config
end

function Config.default() : Config
    local config : Config = {}
    config.info = {}
    config.audio = {}
    config.window = {}

    config.info.org = "selene"
    config.info.name = "selene"
    config.info.version = {0, 1, 0}

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

return Config