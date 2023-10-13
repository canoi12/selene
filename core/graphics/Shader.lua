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

local OS = selene.system.GetOS()
local shader_base = {}
if OS == "Emscripten" then
  shader_base.version = "#version 100"
  shader_base.attribute = "attribute"
  shader_base.varying = { "varying", "varying" }
  shader_base.precision = "#define texture texture2D\nprecision mediump float;"
else
  shader_base.version = "#version 140"
  shader_base.attribute = "in"
  shader_base.varying = { "out", "in" }
  shader_base.precision = ""
end

function Shader:constructor(position, pixel)
  position = position or default_position
  pixel = pixel or default_pixel
  local attribs = {'vec2 a_Position', 'vec4 a_Color', 'vec2 a_Texcoord'}
  local varyings = {'vec4 v_Color', 'vec2 v_Texcoord'}

  local vert_source = shader_base.version
  local frag_source = shader_base.version .. "\n" .. shader_base.precision
  for i,a in ipairs(attribs) do
    vert_source = vert_source .. "\n" .. shader_base.attribute .. " " .. a .. ';'
  end

  for i,v in ipairs(varyings) do
    vert_source = vert_source .. "\n" .. shader_base.varying[1] .. " " .. v .. ';'
    frag_source = frag_source .. "\n" .. shader_base.varying[2] .. " " .. v .. ';'
  end

  vert_source = vert_source .. [[
uniform mat4 u_MVP;
uniform mat4 u_View;
]] .. position .. [[
void main() {
    gl_Position = position(a_Position, u_MVP, u_View);
    v_Color = a_Color;
    v_Texcoord = a_Texcoord;
}
]]
frag_source = frag_source .. [[
uniform sampler2D u_Texture;
]] .. pixel .. [[
void main() {
    gl_FragColor = pixel(v_Color, v_Texcoord, u_Texture);
}
]]

  local vert = gl.NewShader(gl.VERTEX_SHADER)
  vert:Source(vert_source)
  vert:Compile()

  local frag = gl.NewShader(gl.FRAGMENT_SHADER)
  frag:Source(frag_source)
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
