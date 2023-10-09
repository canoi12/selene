local gl = selene.gl
local class = require 'core.class'
local Shader = class:extend("Shader")

local default_position = [[
vec4 position(vec2 pos, mat4 mvp, mat4 view) {
    return mvp * vec4(pos, 0.0, 1.0);
}
]]

local default_pixel = [[
vec4 pixel(vec4 color, vec2 texcoord, sampler2D tex) {
    return color * texture(tex, texcoord);
}
]]

function Shader:constructor(position, pixel)
  position = position or default_position
  pixel = pixel or default_pixel
  local vert_source = {
    [[
#version 140
in vec2 a_Position;
in vec4 a_Color;
in vec2 a_Texcoord;

out vec4 v_Color;
out vec2 v_Texcoord;

uniform mat4 u_MVP;
uniform mat4 u_View;
]],
    position,
    [[

void main() {
    gl_Position = position(a_Position, u_MVP, u_View);
    v_Color = a_Color;
    v_Texcoord = a_Texcoord;
}
]]
  }

  local frag_source = {
    [[
#version 140
in vec4 v_Color;
in vec2 v_Texcoord;
uniform sampler2D u_Texture;
]],
    pixel,
    [[
void main() {
    gl_FragColor = pixel(v_Color, v_Texcoord, u_Texture);
}
]]
  }

  local temp = ""
  for i,src in ipairs(vert_source) do
    temp = temp .. src
  end

  local vert = gl.NewShader(gl.VERTEX_SHADER)
  vert:Source(temp)
  vert:Compile()

  temp = ""
  for i,src in ipairs(frag_source) do
    temp = temp .. src
  end
  local frag = gl.NewShader(gl.FRAGMENT_SHADER)
  frag:Source(temp)
  frag:Compile()

  local prog = gl.NewProgram()
  prog:AttachShader(vert, frag)
  prog:Link()

  self.program = prog
  self.vert = vert
  self.frag = frag
end

local funcs = {
  ["number"] = function(prog, name, ...)
    local nums = {...}
    local location = prog.program:GetUniformLocation(name)
    gl.Uniform1fv(location, ...)
  end,
  ["table"] = function(prog, name, ...)
    local tables = {...}
    local len = #tables[1]
    if len == 2 then
      local location = prog.program:GetUniformLocation(name)
      gl.Uniform2fv(location, ...)
    elseif len == 3 then
      local location = prog.program:GetUniformLocation(name)
      gl.Uniform3fv(location, ...)
    elseif len == 4 then
      local location = prog.program:GetUniformLocation(name)
      gl.Uniform4fv(location, ...)
    end
  end,
  ["userdata"] = function(prog, name, ...)
    local mat = {...}
    local location = prog.program:GetUniformLocation(name)
    gl.UniformMatrix4fv(location, 1, false, mat[1])
  end
}

function Shader:send(name, ...)
  local args = {...}
  funcs[type(args[1])](self, name, ...)
end

return Shader
