local gl = selene.gl
local record Effect
    program: gl.Program
    vertShader: gl.Shader
    fragShader: gl.Shader

    create: function(string | nil, string | nil) : Effect
    send: function(Effect, string, ...: number | {number} | selene.linmath.Mat4)

    metamethod __gc: function(Effect)
end

local defaultPosition : string = [[
vec4 position(vec2 pos, mat4 mvp, mat4 view) {
    return mvp * vec4(pos, 0.0, 1.0);
}
]]

local defaultPixel : string = [[
vec4 pixel(vec4 color, vec2 texcoord, sampler2D tex) {
    return color * texture(tex, texcoord);
}
]]

local record ShaderDef
  version: string
  attribute: string
  varying: {string}
  precision: string
end

local OS = selene.system.getOS()
local shaderBase: ShaderDef = {}
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

local effect_mt : metatable<Effect> = {
  __gc = function(effect: Effect)
    effect.program:destroy()
    effect.vertShader:destroy()
    effect.fragShader:destroy()
  end
}

function Effect.create(position: string | nil, pixel: string | nil) : Effect
  position = position or defaultPosition
  pixel = pixel or defaultPixel
  local effect: Effect = {}
  local attribs: {string} = {'vec2 a_Position', 'vec4 a_Color', 'vec2 a_Texcoord'}
  local varyings: {string} = {'vec4 v_Color', 'vec2 v_Texcoord'}

  local vertSource = shaderBase.version
  local fragSource = shaderBase.version .. '\n' .. shaderBase.precision
  for _,a in ipairs(attribs) do
    vertSource = vertSource .. '\n' .. shaderBase.attribute .. " " .. a .. ';'
  end

  for _,v in ipairs(varyings) do
    vertSource = vertSource .. "\n" ..
      shaderBase.varying[1] .. " " .. v .. ";"

    fragSource = fragSource .. "\n" ..
      shaderBase.varying[2] .. " " .. v .. ";"
  end

  vertSource = vertSource .. [[
uniform mat4 u_MVP;
uniform mat4 u_View;
]] .. position .. [[
void main() {
    gl_Position = position(a_Position, u_MVP, u_View);
    v_Color = a_Color;
    v_Texcoord = a_Texcoord;
}
]]
    fragSource = fragSource .. [[
uniform sampler2D u_Texture;
]] .. pixel .. [[
void main() {
    gl_FragColor = pixel(v_Color, v_Texcoord, u_Texture);
}
]]
    local vert = gl.Shader.create(gl.VERTEX_SHADER)
    vert:source(vertSource)
    vert:compile()

    local frag = gl.Shader.create(gl.FRAGMENT_SHADER)
    frag:source(fragSource)
    frag:compile()

    local prog = gl.Program.create()
    prog:attachShader(vert, frag)
    prog:link()

    effect.program = prog
    effect.vertShader = vert
    effect.fragShader = frag
    return setmetatable(effect, effect_mt)
end

local funcs: {string:function} = {
  ["number"] = function(prog: gl.Program, name: string, ...: number)
    local location = prog:getUniformLocation(name)
    gl.uniform1fv(location, ... as number)
  end,
  ["table"] = function(prog: gl.Program, name: string, ...: {number})
    local tables = {...}
    local len = #tables[1]
    local location = prog:getUniformLocation(name)
    if len == 2 then
      gl.uniform2fv(location, ...)
    elseif len == 3 then
      gl.uniform3fv(location, ...)
    elseif len == 4 then
      gl.uniform4fv(location, ...)
    else
      error("Invalid vector size")
    end
  end,
  ["userdata"] = function(prog: gl.Program, name: string, ...: selene.linmath.Mat4)
    local mat = {...}
    local location = prog:getUniformLocation(name)
    gl.uniformMatrix4fv(location, 1, false, mat[1])
  end
}

function Effect:send(name: string, ...: number | {number} | selene.linmath.Mat4)
  local values = {...}
  local t = type(values[1])
  if funcs[t] then
    funcs[t](self, name, ...)
  end
end

return Effect
