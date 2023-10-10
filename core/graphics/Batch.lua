local gl = selene.gl
local Drawable = require 'core.graphics.Drawable'
local Batch = Drawable:extend('Batch')

local vertex_size = 32

function Batch:constructor(size)
  self.offset = 0
  self.size = size * vertex_size
  self.vbo = gl.NewBuffer()
  self.data = selene.data.NewData(self.size)

  gl.BindBuffer(gl.ARRAY_BUFFER, self.vbo)
  gl.BufferData(gl.ARRAY_BUFFER, self.size, gl.DYNAMIC_DRAW)
  gl.BindBuffer(gl.ARRAY_BUFFER)
end

local function push_vertex(batch, vertex)
end

function Batch:push(x, y, r, g, b, a, u, v)
  if (self.offset + vertex_size) >= self.size then
    self.size = self.size * 2
    self.data:Realloc(self.size)
    print('Realloc', self.size)
    gl.BindBuffer(gl.ARRAY_BUFFER, self.vbo)
    gl.BufferData(gl.ARRAY_BUFFER, self.size, gl.DYNAMIC_DRAW)
    gl.BindBuffer(gl.ARRAY_BUFFER)
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
  gl.BindBuffer(gl.ARRAY_BUFFER, self.vbo)
  gl.BufferSubData(gl.ARRAY_BUFFER, 0, offset, self.data)
  gl.BindBuffer(gl.ARRAY_BUFFER)
  return true
end

function Batch:count()
  return self.offset / vertex_size
end

return Batch
