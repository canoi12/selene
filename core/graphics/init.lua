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
  projection = selene.linmath.Mat4.New(),
  modelview = selene.linmath.Mat4.New(),
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
  local flags = {
    sdl.WINDOW_SHOWN,
    sdl.WINDOW_OPENGL
  }

  if config.window.resizable then
    table.insert(flags, sdl.WINDOW_RESIZABLE)
  end

  if config.window.borderless then
    table.insert(flags, sdl.WINDOW_BORDERLESS)
  end

  if config.window.fullscreen then
    table.insert(flags, sdl.WINDOW_FULLSCREEN_DESKTOP)
  end

  if config.window.always_on_top then
    table.insert(flags, sdl.WINDOW_ALWAYS_ON_TOP)
  end

  local window = sdl.Window.Create(
    config.window.title,
    sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED,
    config.window.width, config.window.height,
    flags
  )
  local ctx = sdl.GLContext.Create(window)
  sdl.GL_MakeCurrent(window, ctx)

  gl.LoadGlad(sdl.GL_GetProcAddress())
  -- gl.PrintVersion()

  graphics.window = window
  graphics.gl_context = ctx

  default.shader = Shader:new()

  default.vao = gl.VertexArray.New()
  default.batch = Batch:new(1000)

  default.vao:Bind()
  gl.Buffer.Bind(gl.ARRAY_BUFFER, default.batch.vbo)

  gl.VertexArray.Enable(0)
  gl.VertexArray.Enable(1)
  gl.VertexArray.Enable(2)

  local program = default.shader.program
  gl.VertexArray.AttribPointer(program:GetAttribLocation("a_Position"), 2, gl.FLOAT, false, 32, 0)
  gl.VertexArray.AttribPointer(program:GetAttribLocation("a_Color"), 4, gl.FLOAT, false, 32, 8)
  gl.VertexArray.AttribPointer(program:GetAttribLocation("a_Texcoord"), 2, gl.FLOAT, false, 32, 24)

  gl.VertexArray.Unbind()
  gl.Buffer.Bind(gl.ARRAY_BUFFER)

  local image_data = selene.Data.New(4)
  image_data:WriteByte(0, 255, 255, 255, 255)
  default.image = setmetatable({}, { __index = Image })
  default.image.target = gl.TEXTURE_2D
  default.image.texture = gl.Texture.New()
  gl.Texture.Bind(gl.TEXTURE_2D, default.image.texture)
  gl.Texture.Image2D(gl.TEXTURE_2D, gl.RGBA, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, image_data)
  gl.Texture.Bind(gl.TEXTURE_2D)
  default.image.width = 1
  default.image.height = 1
  default.image.data = image_data

  local width, height = graphics.window:GetSize()

  default.canvas = setmetatable({}, { __index = Canvas })
  default.canvas.target = gl.FRAMEBUFFER
  default.canvas.width = width
  default.canvas.height = height
  default.canvas.framebuffer = nil

  local data, w, h, glyphs = selene.font.GetDefault()
  default.font = setmetatable({}, { __index = Font })
  default.font.image = Image(w, h, data)
  default.font.rects = glyphs
  default.font.size = 8

  sdl.GL_SetSwapInterval(true)
end

function graphics.deinit()
  graphics.gl_context:Delete()
  graphics.window:Destroy()
end

local function set_image(image)
  image = image or default.image
  if image ~= current.image then
    graphics.finish()
    default.batch:clear()
    current.image = image
    gl.Texture.Bind(gl.TEXTURE_2D, image.texture)
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
    gl.Program.Use(shader.program)
    shader:send("u_MVP", current.projection)
  end
end

function graphics.set_canvas(canvas)
  canvas = canvas or default.canvas
  if canvas ~= current.canvas then
    graphics.finish()
    default.batch:clear()
    gl.Viewport(0, 0, canvas.width, canvas.height)
    gl.Framebuffer.Bind(gl.FRAMEBUFFER, canvas.framebuffer)
    current.canvas = canvas

    current.projection:Ortho(0, canvas.width, canvas.height, 0, -1, 1)
    current.shader:send("u_MVP", current.projection)
  end
end

function graphics.update_size(w, h)
  gl.Viewport(0, 0, w, h)
  current.projection:Ortho(0, w, h, 0, -1, 1)
  current.shader:send("u_MVP", current.projection)
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

  default.vao:Bind()
  gl.DrawArrays(draw_mode, 0, count)
  gl.VertexArray.Unbind()
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

function graphics.draw(obj, rect, x, y, sx, sy)
  sx = sx or 1
  sy = sy or 1
  set_image(obj)
  set_draw_mode('triangles')

  x = x or 0
  y = y or 0

  local rect = rect or { x = 0, y = 0, w = obj.width, h = obj.height }
  local uv = obj:get_uv(rect)

  local r,g,b,a = table.unpack(current.draw_color)
  local vertex_data = default.batch.data
  local w = rect.w * sx
  local h = rect.h * sy
  default.batch:push(x, y, r, g, b, a, uv[1], uv[2])
  default.batch:push(x+w, y, r, g, b, a, uv[3], uv[2])
  default.batch:push(x+w, y+h, r, g, b, a, uv[3], uv[4])

  default.batch:push(x, y, r, g, b, a, uv[1], uv[2])
  default.batch:push(x+w, y+h, r, g, b, a, uv[3], uv[4])
  default.batch:push(x, y+h, r, g, b, a, uv[1], uv[4])
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
  local i = 1
  while i <= #text do
    local c = text:byte(i)
    local codepoint, bytes = selene.UTF8Codepoint(text, i)
    i = i + bytes
    local rect = font.rects[codepoint]

    if c == string.byte('\n') then
      ox = x
      oy = oy + image.height
    elseif c == string.byte('\t') then
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

      ox = ox + rect.ax
      oy = oy + rect.ay
    end
  end
end

function graphics.print_wrap(text, x, y, width)
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
  local i = 1
  while i <= #text do
    local c = text:byte(i)
    local codepoint, bytes = selene.UTF8Codepoint(text, i)
    i = i + bytes
    local rect = font.rects[codepoint]

    if c == string.byte('\n') or ox >= width then
      ox = x
      oy = oy + image.height
    elseif c == string.byte('\t') then
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

      ox = ox + rect.ax
      oy = oy + rect.ay
    end
  end
end

return graphics
