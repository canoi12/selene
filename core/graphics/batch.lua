local gl = selene.gl
local Drawable = require 'core.graphics.drawable'
local Batch = Drawable:extend('Batch')

local vertex_size = 32

function Batch:constructor(size)
  self.size = size * vertex_size
  self.vbo = gl.NewBuffer()
  self.data = selene.utils.NewData(self.size)

  gl.BindBuffer(gl.ARRAY_BUFFER, self.vbo)
  gl.BufferData(gl.ARRAY_BUFFER, self.size, gl.DYNAMIC_DRAW)
  gl.BindBuffer(gl.ARRAY_BUFFER)
end

local function push_vertex(batch, vertex)
end

function Batch:push(x, y, r, g, b, a, u, v)
  self.data:WriteFloat(x, y, r, g, b, a, u, v)
end

function Batch:clear()
  self.data:SetOffset(0)
end

function Batch:flush()
  local offset = self.data:GetOffset()
  if offset <= 0 then return false end
  gl.BindBuffer(gl.ARRAY_BUFFER, self.vbo)
  gl.BufferSubData(gl.ARRAY_BUFFER, 0, offset, self.data)
  gl.BindBuffer(gl.ARRAY_BUFFER)
  return true
end

function Batch:count()
  return self.data:GetOffset() / vertex_size
end

return Batch
