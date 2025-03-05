--- @meta

image = {}

--- @alias image_data {width: integer, height: integer, data: selene.Data}

--- @param filename string
--- @return image_data
function image.from_file(filename) end

--- @param data selene.Data
--- @param size integer
--- @return image_data
function image.from_memory(data, size) end