--- @meta
--- @class selene
--- @field __version string
--- @field __dir string
--- @field __exec string
selene = {}

--- @param filename string
--- @return userdata, integer
function selene.load_file(filename) end

--- @param filename string
--- @return string
function selene.read_file(filename) end

--- @param size integer
function selene.alloc(size) end

--- Delay time in milliseconds
--- @param ms integer
function selene.delay(ms) end

--- @return integer
function selene.get_ticks() end

--- Set state of selene main loop
--- @param val boolean
function selene.set_running(val) end

--- Set the event callback
--- @param func function(string, ...)
function selene.set_event(func) end

--- Set step callback
--- @param func function
function selene.set_step(func) end

--- Set Quit callback
--- @param func function
function selene.set_quit(func) end

--- Create a new Window
--- @param title string
--- @param width integer
--- @param height integer
--- @param opt table
--- @return sdlWindow
function selene.create_window(title, width, height, opt) end

--- Create a new Renderer
--- @param win sdlWindow
function selene.create_renderer(win) end

--- Get selene context
--- @return {window:sdlWindow|nil,renderer:Renderer|nil,audio_system:AudioSystem|nil}
function selene.get_context() end

--- @alias host_name
--- | 'windows'
--- | 'linux'
--- | 'macos'
--- | 'android'
--- | 'emscripten'

--- Get system name
--- @return host_name
function os.host() end

--- Get system architecture
--- @return string|nil
function os.arch() end

selene.filesystem = {}
--- Read the content of the specified file
--- @param filename string
--- @param to_userdata boolean
--- @return string|userdata
function selene.filesystem.read(filename, to_userdata) end

--- Write to the specified file
--- @param filename string
--- @param data string|userdata
--- @param size integer|nil
function selene.filesystem.write(filename, data, size) end