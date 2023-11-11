local Config = {Info = {}, Audio = {}, Window = {}, }




























function Config.create(title, width, height)
   local config = {}
   local audio = {}
   local info = {}
   local window = {}

   info.org = "selene"
   info.name = title
   info.version = { 0, 1, 0 }

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

function Config.default()
   local config = {}
   config.info = {}
   config.audio = {}
   config.window = {}

   config.info.org = "selene"
   config.info.name = "selene"
   config.info.version = { 0, 1, 0 }

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
