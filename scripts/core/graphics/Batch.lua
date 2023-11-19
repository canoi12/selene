local gl = selene.gl

VertexFormat = {}










function VertexFormat.create()
   local v

   v.handle = gl.VertexArray.create()

   return v
end

local vertexSize = 32

local Batch = {}

















function Batch.create(size)
   local b = {}
   b.offset = 0
   b.size = size * vertexSize
   b.buffer = gl.Buffer.create()
   b.data = selene.Data.create(b.size)

   gl.Buffer.bind(gl.ARRAY_BUFFER, b.buffer)
   gl.Buffer.data(gl.ARRAY_BUFFER, b.size, gl.DYNAMIC_DRAW)
   gl.Buffer.bind(gl.ARRAY_BUFFER)

   return setmetatable(b, {
      __index = Batch,
      __gc = function(batch)
         batch.buffer:destroy()
         batch.data:free()
      end,
   })
end

function Batch:push(x, y, r, g, b, a, u, v)
   if self.offset + vertexSize >= self.size then
      self.size = self.size * 2
      self.data:realloc(self.size)
      gl.Buffer.bind(gl.ARRAY_BUFFER, self.buffer)
      gl.Buffer.data(gl.ARRAY_BUFFER, self.size, gl.DYNAMIC_DRAW)
      gl.Buffer.bind(gl.ARRAY_BUFFER)
   end
   self.data:writeFloats(self.offset, x, y, r, g, b, a, u, v)
   self.offset = self.offset + vertexSize
end

function Batch:clear()
   self.offset = 0
end

function Batch:flush()
   local offset = self.offset
   if offset <= 0 then return end
   gl.Buffer.bind(gl.ARRAY_BUFFER, self.buffer)
   gl.Buffer.subData(gl.ARRAY_BUFFER, 0, offset, self.data)
   gl.Buffer.bind(gl.ARRAY_BUFFER)
end

function Batch:count()
   return self.offset / vertexSize
end

return Batch
