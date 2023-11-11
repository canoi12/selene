local sdl = selene.sdl2
local Config = require 'core.Config'

local record Window
    handle: sdl.Window

    width: integer
    height: integer

    fullscreen: boolean
    borderless: boolean

    create: function(Config) : Window
    destroy: function(Window)

    getSize: function(Window) : integer, integer
    swap: function(Window)
end

function Window.createFrom(width: integer, height: integer, title: string, flags: {integer}) : Window
    local window : Window = {}
    sdl.glSetAttribute(sdl.GL_DOUBLEBUFFER, 1)
    sdl.glSetAttribute(sdl.GL_DEPTH_SIZE, 24)
    sdl.glSetAttribute(sdl.GL_STENCIL_SIZE, 8)
    if selene.system.getOS() == "Emscripten" then
        sdl.glSetAttribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_ES)
        sdl.glSetAttribute(sdl.GL_CONTEXT_MAJOR_VERSION, 2);
        sdl.glSetAttribute(sdl.GL_CONTEXT_MINOR_VERSION, 0);
    else
        sdl.glSetAttribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_CORE)
        sdl.glSetAttribute(sdl.GL_CONTEXT_MAJOR_VERSION, 3);
        sdl.glSetAttribute(sdl.GL_CONTEXT_MINOR_VERSION, 3);
    end

    window.width = width
    window.height = height
    window.handle = sdl.Window.create(
        title,
        sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED,
        width, height,
        flags
    )

    return setmetatable(window, {
        __index = Window
    })
end

function Window.create(config: Config) : Window
    local window : Window = {}
    sdl.glSetAttribute(sdl.GL_DOUBLEBUFFER, 1)
    sdl.glSetAttribute(sdl.GL_DEPTH_SIZE, 24)
    sdl.glSetAttribute(sdl.GL_STENCIL_SIZE, 8)
    if selene.system.getOS() == "Emscripten" then
        sdl.glSetAttribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_ES)
        sdl.glSetAttribute(sdl.GL_CONTEXT_MAJOR_VERSION, 2);
        sdl.glSetAttribute(sdl.GL_CONTEXT_MINOR_VERSION, 0);
    else
        sdl.glSetAttribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_CORE)
        sdl.glSetAttribute(sdl.GL_CONTEXT_MAJOR_VERSION, 3);
        sdl.glSetAttribute(sdl.GL_CONTEXT_MINOR_VERSION, 3);
    end

    local flags = {sdl.WINDOW_SHOWN, sdl.WINDOW_OPENGL}

    if config.window.resizable then
        table.insert(flags, sdl.WINDOW_RESIZABLE)
    end
    if config.window.borderless then
        table.insert(flags, sdl.WINDOW_BORDERLESS)
    end
    if config.window.fullscreen then
        table.insert(flags, sdl.WINDOW_FULLSCREEN_DESKTOP)
    end
    if config.window.alwaysOnTop then
        table.insert(flags, sdl.WINDOW_ALWAYS_ON_TOP)
    end

    window.fullscreen = config.window.fullscreen
    window.borderless = config.window.borderless
    window.width = config.window.width
    window.height = config.window.height

    window.handle = sdl.Window.create(
        config.window.title,
        sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED,
        config.window.width, config.window.height,
        flags
    )
    return setmetatable(window, { __index = Window })
end

function Window:destroy()
    self.handle:destroy()
end

function Window:getSize() : integer, integer
    return self.handle:getSize()
end

function Window:swap()
    self.handle:swap()
end

return Window