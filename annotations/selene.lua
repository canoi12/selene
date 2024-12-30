--- @meta
--- @class selene
--- @field Data selene.Data
--- @field __dir string
--- @field __exec string
selene = {}

--- @return string
function selene.get_version() end

--- Get executable path
--- @return string
function selene.get_exec_path() end

--- Set state of selene main loop
--- @param val boolean
function selene.set_running(val) end

--- Create data
--- @param size integer
--- @param copy_data selene.Data|nil
--- @return selene.Data
function selene.create_data(size, copy_data) end

--- Create cube data for simple examples
--- @return {vertices:userdata, indices:userdata, num_vertices:integer, num_indices:integer}
function selene.cube_data() end

--- Set step callback
--- @param func function
function selene.set_step(func) end

--- Set Quit callback
--- @param func function
function selene.set_quit(func) end

--- @enum host_name
local host_name = {
    'windows',
    'linux',
    'android'
}

--- Get system name
--- @return host_name
function os.host() end

--- Get system architecture
--- @return string|nil
function os.arch() end

--- @class selene.Data
local Data = {}

--- @return integer
function Data:size() end

--- @param offset integer
--- @return lightuserdata
function Data:root(offset) end

--- @return string
function Data:get_string() end

--- @param offset integer
--- @param count integer|nil
--- @return integer ...
function Data:read_bytes(offset, count) end

--- @param offset integer
--- @param count integer|nil
--- @return integer ...
function Data:read_ints(offset, count) end

--- @param offset integer
--- @param count integer|nil
--- @return integer ...
function Data:read_floats(offset, count) end

--- @param offset integer
--- @vararg integer
--- @return integer
function Data:write_bytes(offset, ...) end

--- @param offset integer
--- @vararg integer
--- @return integer
function Data:write_ints(offset, ...) end

--- @param offset integer
--- @vararg number
--- @return integer
function Data:write_floats(offset, ...) end

--- @param offset integer
--- @param str string
--- @return integer
function Data:write_string(offset, str) end

--- @class selene.fs
fs = {}

--- @param path string
function fs.exists(path) end

--- @param filename string
function fs.read_text(filename) end