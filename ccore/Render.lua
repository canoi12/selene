local gl = selene.gl
local sdl = selene.sdl2
local Window = require('core.Window')
local Render = {}

function Render.create(win)
  local render = {}
  render.glContext = sdl.GLContext.create(win.handle)
  sdl.glMakeCurrent(win.handle, render.glContext)
  gl.loadGlad(sdl.glGetProcAddress())

  render.state = {}

  render.state.mode = "triangles"
  local vao = gl.VertexArray.create()
  local batch = Batch.create(1000)

  render.vao = vao
  render.batch = batch

  vao:bind()
  gl.Buffer.bind(gl.ARRAY_BUFFER, batch.buffer)

  gl.VertexArray.enable(0)
  gl.VertexArray.enable(1)
  gl.VertexArray.enable(2)

  local effect = Effect.create()
  local p = effect.program
  gl.VertexArray.attribPointer(p:getAttribLocation("a_Position"), 2, gl.FLOAT, false, 32, 0)
  gl.VertexArray.attribPointer(p:getAttribLocation("a_Color"), 4, gl.FLOAT, false, 32, 8)
  gl.VertexArray.attribPointer(p:getAttribLocation("a_Texcoord"), 2, gl.FLOAT, false, 32, 24)

  gl.VertexArray.unbind()
  gl.Buffer.bind(gl.ARRAY_BUFFER)

  local imageData = selene.Data.create(4)
  imageData:writeBytes(0, 255, 255, 255, 255)
  local white = Image.create(1, 1, 4, imageData)
  imageData:free()

  local width, height = win:getSize()
  local canvas = setmetatable({}, { __index = Canvas })

  return render
end

return Render
