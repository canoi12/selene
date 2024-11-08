--- @meta
--- @class selene
--- @field AudioDecoder selene.AudioDecoder
--- @field Data selene.Data
--- @field FontData selene.FontData
--- @field __dir string
--- @field __exec string
selene = {}

--- @return string
function selene.get_version() end

--- Set state of selene main loop
--- @param val boolean
function selene.set_running(val) end

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

--- @class selene.AudioDecoder
local AudioDecoder = {}

--- @param path string
--- @return selene.AudioDecoder
function AudioDecoder.load(path) end

function AudioDecoder:close() end

--- @param index integer
function AudioDecoder:seek(index) end

--- @param data selene.Data
--- @param len integer|nil
--- @return integer
function AudioDecoder:readS16(data, len) end

--- @param data selene.Data
--- @param len integer|nil
--- @return integer
function AudioDecoder:readF32(data, len) end

--- @param data selene.Data
--- @param len integer|nil
--- @return integer
function AudioDecoder:getChunk(data, len) end

--- @return integer
function AudioDecoder:getSampleRate() end

--- @return integer
function AudioDecoder:getChannels() end

--- @return integer
function AudioDecoder:getBitDepth() end

--- @return integer
function AudioDecoder:getFrameCount() end

--- @class selene.Data
--- @field size integer
--- @field root lightuserdata
local Data = {}

--- @param size integer
--- @param copy_data selene.Data|nil
--- @return selene.Data
function Data.create(size, copy_data) end

--- @param offset integer
--- @vararg number
function Data:writeFloats(offset, ...) end

--- @class selene.FontData
--- @field image_width integer
--- @field image_height integer
local FontData = {}

--- @return selene.FontData
function FontData.create8x8() end

--- @param filename string
--- @return selene.FontData
function FontData.loadFromTTF(filename) end

--- @class selene.ImageData
--- @field data_size integer
--- @field width integer
--- @field height integer
--- @field pixels lightuserdata
local ImageData = {}

--- @param filename string
---@param comp integer|nil
function ImageData.load(filename, comp) end

--- @param data selene.Data
--- @param comp integer|nil
function ImageData.fromMemory(data, comp) end


--- @class selene.fs
fs = {}

--- @param path string
function fs.exists(path) end

--- @param filename string
function fs.read_text(filename) end

--[[
*************************
*    linmath
*************************
]]
--- @class selene.linmath
lmath = {}

--- @return userdata
function lmath.alloc_mat4x4() end

--- Set the values of matrix to identity
--- @param mat userdata|lightuserdata
function lmath.mat4x4_identity(mat) end

--- Create an orthogonal matrix
--- @param mat userdata|lightuserdata
--- @param left number
--- @param right number
--- @param top number
--- @param bottom number
--- @param near number
--- @param far number
function lmath.mat4x4_ortho(mat, left, right, top, bottom, near, far) end

--- Duplicate matrix B to the matrix A
--- @param a userdata|lightuserdata
--- @param b userdata|lightuserdata
function lmath.mat4x4_dup(a, b) end