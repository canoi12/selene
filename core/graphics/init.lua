local gl = selene.gl
local sdl = selene.sdl2
local graphics = {}

local Image = require 'core.graphics.Image'
local Batch = require 'core.graphics.Batch'
local Shader = require 'core.graphics.Shader'
local Canvas = require 'core.graphics.Canvas'
local Font = require 'core.graphics.Font'

local current = {
  mode = 'triangles',
  font = {},
  image = {},
  drawable = {},
  canvas = {},
  shader = {},
  projection = selene.utils.NewMat4(),
  modelview = selene.utils.NewMat4(),
  draw_color = { 1.0, 1.0, 1.0, 1.0 },
  clear_color = { 1.0, 1.0, 1.0, 1.0 },
}

local default = {
  vao = nil,
  batch = {},
  font = {},
  image = {},
  canvas = {},
  shader = {},
}

function graphics.init(config)
  local flags = 0

  sdl.GL_SetAttribute(sdl.GL_DOUBLEBUFFER, 1)
  sdl.GL_SetAttribute(sdl.GL_DEPTH_SIZE, 24)
  sdl.GL_SetAttribute(sdl.GL_STENCIL_SIZE, 8)
  if selene.system.GetOS() == "Emscripten" then
    sdl.GL_SetAttribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_ES)
    sdl.GL_SetAttribute(sdl.GL_CONTEXT_MAJOR_VERSION, 2);
    sdl.GL_SetAttribute(sdl.GL_CONTEXT_MINOR_VERSION, 0);
  else
    sdl.GL_SetAttribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_CORE)
    sdl.GL_SetAttribute(sdl.GL_CONTEXT_MAJOR_VERSION, 3);
    sdl.GL_SetAttribute(sdl.GL_CONTEXT_MINOR_VERSION, 3);
  end

  print(selene.system.GetOS())
  print(selene.system.GetArch())

  local window = sdl.CreateWindow(
    config.window.title,
    config.window.width, config.window.height
  )
  local ctx = sdl.GL_CreateContext(window)
  sdl.GL_MakeCurrent(ctx, window)

  gl.LoadGlad()
  gl.PrintVersion()

  graphics.window = window
  graphics.gl_context = ctx

  default.shader = Shader:new()

  default.vao = gl.NewVertexArray()
  default.batch = Batch:new(1000)

  gl.BindVertexArray(default.vao)
  gl.BindBuffer(gl.ARRAY_BUFFER, default.batch.vbo)

  gl.EnableVertexAttribArray(0)
  gl.EnableVertexAttribArray(1)
  gl.EnableVertexAttribArray(2)

  local program = default.shader.program
  gl.VertexAttribPointer(program:GetAttribLocation("a_Position"), 2, gl.FLOAT, false, 32, 0)
  gl.VertexAttribPointer(program:GetAttribLocation("a_Color"), 4, gl.FLOAT, false, 32, 8)
  gl.VertexAttribPointer(program:GetAttribLocation("a_Texcoord"), 2, gl.FLOAT, false, 32, 24)

  gl.BindVertexArray()
  gl.BindBuffer(gl.ARRAY_BUFFER)

  local image_data = selene.data.NewData(4)
  image_data:Write(0, 255, 255, 255, 255)
  default.image = setmetatable({}, { __index = Image })
  default.image.target = gl.TEXTURE_2D
  default.image.texture = gl.NewTexture()
  gl.BindTexture(gl.TEXTURE_2D, default.image.texture)
  gl.TexImage2D(gl.TEXTURE_2D, gl.RGBA, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, image_data)
  gl.BindTexture(gl.TEXTURE_2D)
  default.image.width = 1
  default.image.height = 1
  default.image.data = image_data

  local width, height = graphics.window:GetSize()

  default.canvas = setmetatable({}, { __index = Canvas })
  default.canvas.target = gl.FRAMEBUFFER
  default.canvas.width = width
  default.canvas.height = height
  default.canvas.framebuffer = nil

  default.font = Font:new("core/font.ttf")
end

function graphics.deinit()
  sdl.GL_DeleteContext(graphics.gl_context)
  sdl.DestroyWindow(graphics.window)
end

local function set_image(image)
  image = image or default.image
  if image ~= current.image then
    graphics.finish()
    default.batch:clear()
    current.image = image
    gl.BindTexture(gl.TEXTURE_2D, image.texture)
  end
end

local draw_modes = {
  points = gl.POINTS,
  lines = gl.LINES,
  triangles = gl.TRIANGLES
}

local function set_draw_mode(mode)
  mode = mode or 'triangles'
  if mode ~= current.draw_mode then
    graphics.finish()
    default.batch:clear()
    current.mode = mode
  end
end

function graphics.set_font(font)
  font = font or default.font
  current.font = font
end

function graphics.set_shader(shader)
  shader = shader or default.shader
  if shader ~= current.shader then
    graphics.finish()
    default.batch:clear()
    current.shader = shader
    gl.UseProgram(shader.program)
    shader:send("u_MVP", current.projection)
  end
end

function graphics.set_canvas(canvas)
  canvas = canvas or default.canvas
  if canvas ~= current.canvas then
    graphics.finish()
    default.batch:clear()
    gl.Viewport(0, 0, canvas.width, canvas.height)
    gl.BindFramebuffer(gl.FRAMEBUFFER, canvas.framebuffer)
    current.canvas = canvas

    current.projection:Ortho(0, canvas.width, canvas.height, 0, -1, 1)
    current.shader:send("u_MVP", current.projection)
  end
end

function graphics.clear(r, g, b, a)
  r = (r or 0) / 255
  g = (g or 0) / 255
  b = (b or 0) / 255
  a = (a or 255) / 255
  gl.ClearColor(r, g, b, a)
  gl.Clear(gl.COLOR_BUFFER_BIT)
end

function graphics.set_color(r, g, b, a)
  current.draw_color[1] = (r or 255) / 255
  current.draw_color[2] = (g or 255) / 255
  current.draw_color[3] = (b or 255) / 255
  current.draw_color[4] = (a or 255) / 255
end

function graphics.begin()
  default.batch:clear()
  graphics.set_shader(default.shader)
  set_image()
  set_draw_mode('triangles')
  graphics.set_font()
  graphics.set_canvas(default.canvas)
  gl.Enable(gl.BLEND)
  gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
  default.canvas.width, default.canvas.height = graphics.window:GetSize()
  graphics.set_color()
end

function graphics.finish()
  if not default.batch:flush() then return end
  local count = default.batch:count()
  local draw_mode = draw_modes[current.mode]

  gl.BindVertexArray(default.vao)
  gl.DrawArrays(draw_mode, 0, count)
  gl.BindVertexArray()
end

function graphics.swap()
  graphics.window:Swap()
end

function graphics.draw_point(x, y)
  set_draw_mode('points')
  set_image()
  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  default.batch:push(x, y, r, g, b, a, 0, 0)
end

function graphics.draw_line(x0, y0, x1, y1)
  set_image()
  set_draw_mode('lines')

  local r,g,b,a = table.unpack(current.draw_color)
  default.batch:push(x0, y0, r, g, b, a, 0.0, 0.0)
  default.batch:push(x1, y1, r, g, b, a, 0.0, 0.0)
end

function graphics.draw_rectangle(x, y, width, height)
  set_image()
  set_draw_mode('lines')

  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  default.batch:push(x, y, r, g, b, a, 0.0, 0.0)
  default.batch:push(x+width, y, r, g, b, a, 0.0, 0.0)

  default.batch:push(x+width, y, r, g, b, a, 0.0, 0.0)
  default.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)

  default.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)
  default.batch:push(x, y+height, r, g, b, a, 0.0, 0.0)

  default.batch:push(x, y+height, r, g, b, a, 0.0, 0.0)
  default.batch:push(x, y, r, g, b, a, 0.0, 0.0)
end

function graphics.fill_rectangle(x, y, width, height)
  set_image()
  set_draw_mode('triangles')

  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  default.batch:push(x, y, r, g, b, a, 0.0, 0.0)
  default.batch:push(x+width, y, r, g, b, a, 0.0, 0.0)
  default.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)

  default.batch:push(x, y, r, g, b, a, 0.0, 0.0)
  default.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)
  default.batch:push(x, y+height, r, g, b, a, 0.0, 0.0)
end

function graphics.draw_circle(x, y, radius, sides)
  set_image()
  set_draw_mode('lines')
  sides = sides or 32
  local bsize = 2 * sides
  local pi2 = math.pi * 2
  local vertex_data = default.batch.data
  local r,g,b,a = table.unpack(current.draw_color)
  local u,v = 0, 0
  for i=1,sides do
    local tetha = ((i-1) * pi2) / sides

    local xx = x + (math.cos(tetha) * radius)
    local yy = y + (math.sin(tetha) * radius)
    default.batch:push(xx, yy, r, g, b, a, u, v)

    tetha = (i * pi2) / sides
    xx = x + (math.cos(tetha) * radius)
    yy = y + (math.sin(tetha) * radius)
    default.batch:push(xx, yy, r, g, b, a, u, v)
  end
end

function graphics.fill_circle(x, y, radius, sides)
  set_image()
  set_draw_mode('triangles')
  local sides = sides or 32
  local bsize = 3 * sides
  local pi2 = math.pi * 2
  local a = { x = 0, y = 0, r = 0, g = 0, b = 0, a = 0, u = 0, v = 0 }
  local vertex_data = default.batch.data
  local r,g,b,a = table.unpack(current.draw_color)
  local u, v = 0, 0
  for i=1,sides do
    default.batch:push(x, y, r, g, b, a, u, v)

    local tetha = ((i-1) * pi2) / sides
    local xx = x + (math.cos(tetha) * radius)
    local yy = y + (math.sin(tetha) * radius)
    default.batch:push(xx, yy, r, g, b, a, u, v)

    tetha = (i * pi2) / sides
    xx = x + (math.cos(tetha) * radius)
    yy = y + (math.sin(tetha) * radius)
    default.batch:push(xx, yy, r, g, b, a, u, v)
  end
end

function graphics.draw_triangle(x0, y0, x1, y1, x2, y2)
  set_image()
  set_draw_mode('lines')

  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  default.batch:push(x0, y0, r, g, b, a, 0.0, 0.0)
  default.batch:push(x1, y1, r, g, b, a, 0.0, 0.0)

  default.batch:push(x1, y1, r, g, b, a, 0.0, 0.0)
  default.batch:push(x2, y2, r, g, b, a, 0.0, 0.0)

  default.batch:push(x2, y2, r, g, b, a, 0.0, 0.0)
  default.batch:push(x0, y0, r, g, b, a, 0.0, 0.0)
end

function graphics.fill_triangle(x0, y0, x1, y1, x2, y2)
  set_image()
  set_draw_mode('triangles')
  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  default.batch:push(x0, y0, r, g, b, a, 0.0, 0.0)
  default.batch:push(x1, y1, r, g, b, a, 0.0, 0.0)
  default.batch:push(x2, y2, r, g, b, a, 0.0, 0.0)
end

function graphics.draw(obj, rect, x, y)
  set_image(obj)
  set_draw_mode('triangles')

  x = x or 0
  y = y or 0

  local rect = rect or { x = 0, y = 0, w = obj.width, h = obj.height }
  local uv = obj:get_uv(rect)

  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  default.batch:push(x, y, r, g, b, a, uv[1], uv[2])
  default.batch:push(x+rect.w, y, r, g, b, a, uv[3], uv[2])
  default.batch:push(x+rect.w, y+rect.h, r, g, b, a, uv[3], uv[4])

  default.batch:push(x, y, r, g, b, a, uv[1], uv[2])
  default.batch:push(x+rect.w, y+rect.h, r, g, b, a, uv[3], uv[4])
  default.batch:push(x, y+rect.h, r, g, b, a, uv[1], uv[4])
end

function graphics.print(text, x, y)
  set_image(current.font.image)
  set_draw_mode('triangles')
  x = x or 0
  y = y or 0

  local ox = x or 0
  local oy = y or 0

  local font = current.font
  local image = font.image
  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  for i=1,#text do
    local c = text:sub(i, i)
    local codepoint = selene.utils.UTF8Codepoint(c)
    local rect = font.rects[codepoint]

    if c == '\n' then
      ox = x
      oy = oy + image.height
    elseif c == '\t' then
      ox = ox + rect.bw * 2
    else
      local xx = ox + rect.bl
      local yy = oy + rect.bt

      local uv = {}
      uv[1] = rect.tx / image.width
      uv[2] = 0
      uv[3] = uv[1] + (rect.bw / image.width)
      uv[4] = uv[2] + (rect.bh / image.height)

      default.batch:push(xx, yy, r, g, b, a, uv[1], uv[2])
      default.batch:push(xx+rect.bw, yy, r, g, b, a, uv[3], uv[2])
      default.batch:push(xx+rect.bw, yy+rect.bh, r, g, b, a, uv[3], uv[4])

      default.batch:push(xx, yy, r, g, b, a, uv[1], uv[2])
      default.batch:push(xx+rect.bw, yy+rect.bh, r, g, b, a, uv[3], uv[4])
      default.batch:push(xx, yy+rect.bh, r, g, b, a, uv[1], uv[4])
    end

    ox = ox + rect.ax
    oy = oy + rect.ay
  end
end

return graphics
