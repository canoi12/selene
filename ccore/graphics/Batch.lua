local gl = selene.gl
local Batch = {}

local vertex_size = 32

local Batch_mt = {
  __index = Batch,
  __gc = function(b)
    b.vbo:destroy()
    b.data:free()
  end
}

function Batch.create(size)
  local b = {}
  b.offset = 0
  b.size = size * vertex_size
  b.vbo = gl.Buffer.create()
  b.data = selene.Data.create(b.size)

  gl.Buffer.bind(gl.ARRAY_BUFFER, b.vbo)
  gl.Buffer.data(gl.ARRAY_BUFFER, b.size, gl.DYNAMIC_DRAW)
  gl.Buffer.bind(gl.ARRAY_BUFFER)
  return setmetatable(
    b,
    Batch_mt
  )
end

local function push_vertex(batch, vertex)
end

function Batch:push(x, y, r, g, b, a, u, v)
  if (self.offset + vertex_size) >= self.size then
    self.size = self.size * 2
    self.data:Realloc(self.size)
    gl.Buffer.bind(gl.ARRAY_BUFFER, self.vbo)
    gl.Buffer.data(gl.ARRAY_BUFFER, self.size, gl.DYNAMIC_DRAW)
    gl.Buffer.bind(gl.ARRAY_BUFFER)
  end
  self.data:WriteFloat(self.offset, x, y, r, g, b, a, u, v)
  self.offset = self.offset + vertex_size
end

function Batch:clear()
  self.offset = 0
end

function Batch:flush()
  local offset = self.offset
  if offset <= 0 then return false end
  gl.Buffer.bind(gl.ARRAY_BUFFER, self.vbo)
  gl.Buffer.subData(gl.ARRAY_BUFFER, 0, offset, self.data)
  gl.Buffer.bind(gl.ARRAY_BUFFER)
  return true
end

function Batch:count()
  return self.offset / vertex_size
end

function Batch:__gc()
  self.vbo:destroy()
  self.data:free()
end

return Batch
