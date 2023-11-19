--- @meta
--- @class selene.Data
local Data = {}

---Create a new data
---@param size integer
---@return selene.Data
function Data.create(size) end

---Realloc the data size
---@param size integer
function Data:realloc(size) end

---Free the data
function Data:free() end

---Get size of the data
---@return integer
function Data:getSize() end
---Get pointer of the data
---@return lightuserdata
function Data:getPointer() end
---Get data as string
---@return string
function Data:getString() end

---Read [count] bytes
---@param offset integer
---@param count integer | nil
---@return ... integer
function Data:readBytes(offset, count) end
---Read [count] integers
---@param offset integer
---@param count integer | nil
---@return ... integer
function Data:readInts(offset, count) end
---Read [count] floats
---@param offset integer
---@param count integer | nil
---@return ... number
function Data:readFloats(offset, count) end

---Wirte bytes
---@param offset integer
---@param ... integer
---@return integer
function Data:writeBytes(offset, ...) end
---Wirte bytes
---@param offset integer
---@param ... integer
---@return integer
function Data:writeInts(offset, ...) end
---Wirte bytes
---@param offset integer
---@param ... number
---@return integer
function Data:writeFloats(offset, ...) end
---Wirte bytes
---@param offset integer
---@param value string
---@return integer
function Data:writeString(offset, value) end

return Data