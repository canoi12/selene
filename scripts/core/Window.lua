local sdl = selene.sdl2

--- @class Window
--- @field handle selene.sdl2.Window
--- @field width integer
--- @field height integer
--- @field fullscreen boolean
--- @field borderless boolean
local Window = {}

--- Create a new window with settings
--- @param settings Settings
--- @return Window
function Window.create(settings)
    --- @type Window
    local window = {}
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

    if settings.window.resizable then
        table.insert(flags, sdl.WINDOW_RESIZABLE)
    end
    if settings.window.borderless then
        table.insert(flags, sdl.WINDOW_BORDERLESS)
    end
    if settings.window.fullscreen then
        table.insert(flags, sdl.WINDOW_FULLSCREEN_DESKTOP)
    end
    if settings.window.alwaysOnTop then
        table.insert(flags, sdl.WINDOW_ALWAYS_ON_TOP)
    end

    window.fullscreen = settings.window.fullscreen
    window.borderless = settings.window.borderless
    window.width = settings.window.width
    window.height = settings.window.height

    window.handle = sdl.Window.create(
        settings.window.title,
        sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED,
        settings.window.width, settings.window.height,
        flags
    )
    return setmetatable(window, { __index = Window })
end

function Window:destroy()
    self.handle:destroy()
end

--- @return integer, integer
function Window:getSize()
    return self.handle:getSize()
end

function Window:swap()
    self.handle:swap()
end

return Window