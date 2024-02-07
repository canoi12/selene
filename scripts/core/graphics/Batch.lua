local gl = selene.gl

--- @class Batch
--- @field offset integer
--- @field size integer
--- @field vertexSize integer
--- @field buffer selene.gl.Buffer
--- @field data selene.Data
local Batch = {}
local batch_mt = {
    __index = Batch,
    __gc = function(b)
        b.buffer:destroy()
        b.data:free()
    end
}

---Creates a new Batch file
---@param size integer
---@return Batch
function Batch.create(size, vertexSize)
    local b = {}
    b.offset = 0
    b.vertexSize = vertexSize or 32 -- 8 floats (8 * 4 = 32 bytes)
    b.size = size * b.vertexSize
    b.buffer = gl.Buffer.create()
    b.data = selene.Data.create(b.size)

    gl.Buffer.bind(gl.ARRAY_BUFFER, b.buffer)
    gl.Buffer.data(gl.ARRAY_BUFFER, gl.DYNAMIC_DRAW, b.size)
    gl.Buffer.bind(gl.ARRAY_BUFFER)

    return setmetatable(b, batch_mt)
end

function Batch:push(x, y, r, g, b, a, u, v)
    if self.offset + self.vertexSize >= self.size then
        self.size = self.size * 2
        self.data:realloc(self.size)
        gl.Buffer.bind(gl.ARRAY_BUFFER, self.buffer)
        gl.Buffer.data(gl.ARRAY_BUFFER, gl.DYNAMIC_DRAW, self.size)
        gl.Buffer.bind(gl.ARRAY_BUFFER)
    end
    self.data:writeFloats(self.offset, x, y, r, g, b, a, u, v)
    self.offset = self.offset + self.vertexSize
end

function Batch:clear()
    self.offset = 0
end

function Batch:flush()
    local offset = self.offset
    if offset <= 0 then return false end
    gl.Buffer.bind(gl.ARRAY_BUFFER, self.buffer)
    gl.Buffer.subData(gl.ARRAY_BUFFER, 0, self.offset, self.data)
    gl.Buffer.bind(gl.ARRAY_BUFFER)
    return true
end

function Batch:getCount()
    return self.offset / self.vertexSize
end

function Batch:getSize()
    return self.size
end

return Batch