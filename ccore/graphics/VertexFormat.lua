local gl = poti.gl
local VertexFormat = {}

local Attribute = {}

local function make_attrib(name, size, stride, offset)
  local attrib = {}
  attrib.name = name
  attrib.size = size
  attrib.type = gl.FLOAT
  attrib.stride = stride
  attrib.offset = offset
end

function VertexFormat:constructor()
  self.attributes = {}
  self.stride = 0
end

function VertexFormat:add_vec(name, size)
  local offset = 0
  for i,v in ipairs(self.attributes) do
    offset = offset + v.stride
  end
  local attrib = make_attrib(name, size, size * 8, offset)
  table.insert(self.attributes, attrib)
  self.stride = self.stride + attrib.stride
end

return VertexFormat
