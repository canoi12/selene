--- @meta
--- @class fs
local fs = {}

--- @param path string
--- @return boolean
function fs.exists(path) end

--- @param path string
--- @return selene.Data
function fs.read(path) end

--- @param path string
--- @return string
function fs.read_text(path) end

--- @param path string
--- @param text string
--- @return boolean
function fs.write(path, text) end

--- @param path string
function fs.mkdir(path) end

--- @param path string
function fs.rmdir(path) end

