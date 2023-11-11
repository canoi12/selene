local gl = selene.gl
local Effect = {}

local defaultPosition = [[
vec4 position(vec2 pos, mat4 mvp, mat4 view) {
    return mvp * vec4(pos, 0.0, 1.0);
}
]]

local defaultPixel = [[
vec4 pixel(vec4 color, vec2 texcoord, sampler2D tex) {
    return color * texture(tex, texcoord);
}
]]

local OS = selene.system.getOS()
local shaderBase = {}
if OS == "Emscripten" then
  shaderBase.version = "#version 100"
  shaderBase.attribute = "attribute"
  shaderBase.varying = { "varying", "varying" }
  shaderBase.precision = "#define texture texture2D\nprecision mediump float;"
else
  shaderBase.version = "#version 140"
  shaderBase.attribute = "in"
  shaderBase.varying = { "out", "in" }
  shaderBase.precision = ""
end

function Effect.create(position, pixel)
  local self = {}
  position = position or defaultPosition
  pixel = pixel or defaultPixel
  local attribs = {'vec2 a_Position', 'vec4 a_Color', 'vec2 a_Texcoord'}
  local varyings = {'vec4 v_Color', 'vec2 v_Texcoord'}

  local vert_source = shaderBase.version
  local frag_source = shaderBase.version .. "\n" .. shaderBase.precision
  for i,a in ipairs(attribs) do
    vert_source = vert_source .. "\n" .. shaderBase.attribute .. " " .. a .. ';'
  end

  for i,v in ipairs(varyings) do
    vert_source = vert_source .. "\n" .. shaderBase.varying[1] .. " " .. v .. ';'
    frag_source = frag_source .. "\n" .. shaderBase.varying[2] .. " " .. v .. ';'
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

  local vert = gl.Effect.create(gl.VERTEX_SHADER)
  vert:source(vert_source)
  vert:compile()

  local frag = gl.Effect.create(gl.FRAGMENT_SHADER)
  frag:source(frag_source)
  frag:compile()

  local prog = gl.Program.create()
  prog:attachEffect(vert, frag)
  prog:link()

  self.program = prog
  self.vert = vert
  self.frag = frag
end

local funcs = {
  ["number"] = function(prog, name, ...)
    local nums = {...}
    local location = prog.program:getUniformLocation(name)
    gl.uniform1fv(location, ...)
  end,
  ["table"] = function(prog, name, ...)
    local tables = {...}
    local len = #tables[1]
    if len == 2 then
      local location = prog.program:getUniformLocation(name)
      gl.uniform2fv(location, ...)
    elseif len == 3 then
      local location = prog.program:getUniformLocation(name)
      gl.uniform3fv(location, ...)
    elseif len == 4 then
      local location = prog.program:getUniformLocation(name)
      gl.uniform4fv(location, ...)
    end
  end,
  ["userdata"] = function(prog, name, ...)
    local mat = {...}
    local location = prog.program:getUniformLocation(name)
    gl.uniformMatrix4fv(location, 1, false, mat[1])
  end
}

function Effect:send(name, ...)
  local args = {...}
  funcs[type(args[1])](self, name, ...)
end

function Effect:__gc()
  self.program:destroy()
  self.vert:destroy()
  self.frag:destroy()
end

return Effect
