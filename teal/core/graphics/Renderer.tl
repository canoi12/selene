local gl = selene.gl
local sdl = selene.sdl2
local graphics = require('core.graphics.init')
local Mat4 = require 'core.math.Mat4'
local Color = require 'core.graphics.Color'
local Window = require 'core.Window'

local Rect = require('core.Rect')

local Drawable = require('core.graphics.Drawable')

local Batch = require 'core.graphics.Batch'
local Canvas = require 'core.graphics.Canvas'
local Effect = require 'core.graphics.Effect'
local Font = require 'core.graphics.Font'
local Image = require 'core.graphics.Image'

global enum DrawMode
    "points"
    "lines"
    "triangles"
end

local record DrawState
    vao: gl.VertexArray
    drawMode: graphics.DrawMode
    drawColor: Color
    clearColor: Color
end

local record RendererState
    vao: gl.VertexArray

    drawMode: graphics.DrawMode
    drawColor: Color
    clearColor: Color

    canvas: Canvas | nil
    
    font: Font

    texture: gl.Texture
    framebuffer: gl.Framebuffer
    program: gl.Program

    projection: Mat4
    modelview: Mat4
end

local record Renderer
    state: RendererState

    glContext: sdl.GLContext

    vao: gl.VertexArray
    batch: Batch

    whiteImage: Image
    defaultCanvas: Canvas
    defaultEffect: Effect
    defaultFont: Font

    window : Window

    create: function(Window) : Renderer
    destroy: function(Renderer)

    begin: function(Renderer)
    finish: function(Renderer)

    clearColor: function(Renderer, Color)
    clear: function(Renderer)

    setClearColor: function(Renderer, Color)
    setDrawColor: function(Renderer, Color)

    setEffect: function(Renderer, Effect | nil)
    setCanvas: function(Renderer, Canvas | nil)
    setFont: function(Renderer, Font | nil)

    setScissor: function(Renderer)

    drawPoint: function(Renderer, number, number)
    drawLine: function(Renderer, number, number, number, number)
    -- Rect
    drawRectangle: function(Renderer, number, number, number, number)
    fillRectangle: function(Renderer, number, number, number, number)
    -- Circle
    drawCircle: function(Renderer, number, number, number, number | nil)
    fillCircle: function(Renderer, number, number, number, number | nil)
    -- Triangle
    drawTriangle: function(Renderer, {number}, {number}, {number})
    fillTriangle: function(Renderer, {number}, {number}, {number})

    -- Quad
    drawQuad: function(Renderer, {number}, {number}, {number}, {number})
    fillQuad: function(Renderer, {number}, {number}, {number}, {number})
    
    copy: function(Renderer, any, Rect, Rect)
    print: function(Renderer, string, number, number)
end

function Renderer.create(win: Window) : Renderer
    local render : Renderer = {}
    render.glContext = sdl.GLContext.create(win.handle)
    sdl.glMakeCurrent(win.handle, render.glContext)
    gl.loadGlad(sdl.glGetProcAddress())

    render.window = win
    render.state = {}

    render.state.drawMode = "triangles"
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
    local white : Image = Image.create(1, 1, 4, imageData)
    imageData:free()

    local width, height = win:getSize()
    local canvas : Canvas = setmetatable({}, { __index = Canvas })
    canvas.width = width
    canvas.height = height

    local font = Font.default()

    render.whiteImage = white
    render.defaultCanvas = canvas
    render.defaultEffect = effect
    render.defaultFont = font

    sdl.glSetSwapInterval(true)

    return setmetatable(render, {
        __index = Renderer
    })
end

function Renderer:destroy()
    self.glContext:destroy()
end

function Renderer:clearColor(color: Color)
    local cc : {number} = {0, 0, 0, 1}
    for i,c in ipairs(color) do
        cc[i] = c / 255
    end
    gl.clearColor(table.unpack(cc))
end

local drawModes : {DrawMode:integer} = {
    points = gl.POINTS,
    lines = gl.LINES,
    triangles = gl.TRIANGLES
}
local function setFramebuffer(r: Renderer, t: gl.Texture | nil)
    if t ~= r.state.texture then
        r:finish()
        r.batch:clear()
        r.state.texture = t
        gl.Texture.bind(gl.TEXTURE_2D, t)
    end
end

local function setImage(r: Renderer, t: gl.Texture | nil)
    t = t or r.whiteImage.handle
    if t ~= r.state.texture then
        r:finish()
        r.batch:clear()
        r.state.texture = t
        gl.Texture.bind(gl.TEXTURE_2D, t)
    end
end

local function setDrawMode(r: Renderer, mode: DrawMode)
    if mode ~= r.state.drawMode then
        r:finish()
        r.batch:clear()
        r.state.drawMode = mode
    end
end

function Renderer:setEffect(effect: Effect | nil)
    effect = effect or self.defaultEffect
    if effect.program ~= self.state.program then
        self:finish()
        self.batch:clear()
        self.state.program = effect.program
        gl.Program.use(effect.program)
        effect:send("u_MVP", self.state.projection)
    end
end

function Renderer:setCanvas(canvas: Canvas | nil)
    canvas = canvas or self.defaultCanvas
    if canvas.handle ~= self.state.framebuffer then
        self:finish()
        self.batch:clear()
        gl.viewport(0, 0, canvas.width, canvas.height)
        gl.Framebuffer.bind(gl.FRAMEBUFFER, canvas.handle)
        self.state.framebuffer = canvas.handle
        
        self.state.projection:ortho(0, canvas.width, canvas.height, 0, -1, 1)
        local loc = gl.Program.getUniformLocation(self.state.program, "u_MVP")
        gl.uniformMatrix4fv(loc, 1, false, self.state.projection)
    end
end

function Renderer:setDrawColor(c: Color)
    self.state.drawColor = c
end

function Renderer:setClearColor(c: Color)
    self.state.clearColor = c
    gl.clearColor(c:toFloat())
end

function Renderer:clear()
    gl.clear(gl.COLOR_BUFFER_BIT)
end

function Renderer:begin()
    if not self.batch:flush() then return end
    local count = self.batch:count()
    local drawMode = drawModes[self.state.drawMode]

    self:setEffect()
    self:setFont()
    self:setCanvas()

    gl.enable(gl.BLEND)
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
    self.defaultCanvas.width, self.defaultCanvas.height = self.window:getSize()
    self:setDrawColor()
end

function Renderer:finish()
    local mode = drawModes[self.state.drawMode]
    assert(mode, "Invalid draw mode")
    if not self.batch:flush() then return end
    local count = self.batch:count()
    self.vao:bind()
    gl.drawArrays(mode)
    gl.VertexArray.unbind()
end

function Renderer:drawPoint(x : number, y : number)
    setImage(self)
    setDrawMode(self, 'points')
    local r,g,b,a = self.state.drawColor:toFloat()
    self.batch:push(x, y, r, g, b, a, 0, 0)
end

function Renderer:drawLine(x0 : number, y0 : number, x1 : number, y1 : number)
    setImage(self)
    setDrawMode(self, 'lines')
    local r,g,b,a = self.state.drawColor:toFloat()
    self.batch:push(x0, y0, r, g, b, a, 0.0, 0.0)
    self.batch:push(x1, y1, r, g, b, a, 0.0, 0.0)
end

function Renderer:drawRectangle(x : number, y : number, width : number, height : number)
    setImage(self)
    setDrawMode(self, 'lines')
    local r,g,b,a = self.state.drawColor:toFloat()
    self.batch:push(x, y, r, g, b, a, 0.0, 0.0)
    self.batch:push(x+width, y, r, g, b, a, 0.0, 0.0)

    self.batch:push(x+width, y, r, g, b, a, 0.0, 0.0)
    self.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)

    self.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)
    self.batch:push(x, y+height, r, g, b, a, 0.0, 0.0)

    self.batch:push(x, y+height, r, g, b, a, 0.0, 0.0)
    self.batch:push(x, y, r, g, b, a, 0.0, 0.0)
end

function Renderer:fillRectangle(x: number, y: number, width: number, height: number)
    setImage(self)
    setDrawMode(self, 'triangles')
    local r,g,b,a = self.state.drawColor:toFloat()
    self.batch:push(x, y, r, g, b, a, 0.0, 0.0)
    self.batch:push(x+width, y, r, g, b, a, 0.0, 0.0)
    self.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)

    self.batch:push(x, y, r, g, b, a, 0, 0)
    self.batch:push(x+width, y+height, r, g, b, a, 0, 0)
    self.batch:push(x, y+height, r, g, b, a, 0, 0)
end

function Renderer:drawCircle(x: number, y: number, radius: number, sides: number | nil)
    setImage(self)
    setDrawMode(self, 'lines')
    sides = sides or 32.0
    local pi2 = math.pi * 2
    local r,g,b,a = self.state.drawColor:toFloat()
    local u,v = 0, 0
    for i=1,sides do
    local tetha = ((i-1) * pi2) / sides

    local xx = x + (math.cos(tetha) * radius)
    local yy = y + (math.sin(tetha) * radius)
    self.batch:push(xx, yy, r, g, b, a, u, v)

    tetha = (i * pi2) / sides
    xx = x + (math.cos(tetha) * radius)
    yy = y + (math.sin(tetha) * radius)
    self.batch:push(xx, yy, r, g, b, a, u, v)
    end
end

function Renderer:fillCircle(x: number, y: number, radius: number, sides: number | nil)
    setImage(self)
    setDrawMode(self, 'triangles')
    sides = sides or 32.0
    local pi2 = math.pi * 2
    local r,g,b,a = self.state.drawColor:toFloat()
    local u, v = 0, 0
    for i=1,sides do
        self.batch:push(x, y, r, g, b, a, u, v)

        local tetha = ((i-1) * pi2) / sides
        local xx = x + (math.cos(tetha) * radius)
        local yy = y + (math.sin(tetha) * radius)
        self.batch:push(xx, yy, r, g, b, a, u, v)

        tetha = (i * pi2) / sides
        xx = x + (math.cos(tetha) * radius)
        yy = y + (math.sin(tetha) * radius)
        self.batch:push(xx, yy, r, g, b, a, u, v)
    end
end

function Renderer:drawTriangle(p0: {number}, p1: {number}, p2: {number})
    setImage(self)
    setDrawMode(self, 'lines')
    local r,g,b,a = self.state.drawColor:toFloat()
    self.batch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p1[1], p1[2], r, g, b, a, 0.0, 0.0)

    self.batch:push(p1[2], p1[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)

    self.batch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
end

function Renderer:fillTriangle(p0: {number}, p1: {number}, p2: {number})
    setImage(self)
    setDrawMode(self, 'triangles')
    local r,g,b,a = self.state.drawColor:toFloat()
    self.batch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p1[1], p1[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)
end

function Renderer:copy(drawable: any, src: Rect, dest: Rect)
    setImage(self)
    setDrawMode(self, "triangles")
    local o = drawable as Drawable
    
    local r,g,b,a = self.state.drawColor:toFloat()
    local s: Rect = src or Rect.create(0, 0, o:getWidth(), o:getHeight())
    local d: Rect = dest or {x = 0, y = 0, w = s.w, h = s.h}
    local uv = o:getUV(s)
    -- local uv = {0, 0, 1, 1}
    
    self.batch:push(d.x, d.y, r, g, b, a, uv[1], uv[2])
    self.batch:push(d.x+d.w, d.y, r, g, b, a, uv[3], uv[2])
    self.batch:push(d.x+d.w, d.y+d.h, r, g, b, a, uv[3], uv[4])
    
    self.batch:push(d.x, d.y, r, g, b, a, uv[1], uv[2])
    self.batch:push(d.x+d.w, d.y+d.h, r, g, b, a, uv[3], uv[4])
    self.batch:push(d.x, d.y+d.h, r, g, b, a, uv[1], uv[4])
end

function Renderer:print(text: string, x: number, y: number)
    setDrawMode(self, "triangles")
    
    local ox = x or 0
    local oy = y or 0

    local font = self.state.font
    local image = font.texture
    setImage(self, image)
    local r,g,b,a = self.state.drawColor:toFloat()
    local i = 1
    while i <= #text do
        local c = text:byte(i)
        local codepoint, bytes = selene.UTF8Codepoint(text, i)
        i = i + bytes
        local rect = font.rects[codepoint]
        if c == string.byte('\n') then
            ox = x
            oy = oy + font.height
        elseif c == string.byte('t') then
            ox = ox + (rect.bw * 2)
        else
            local xx = ox + rect.bl
            local yy = oy + rect.bt

            local uv = {}
            uv[1] = rect.tx / font.width
            uv[2] = 0
            uv[3] = uv[1] + (rect.bw / font.width)
            uv[4] = uv[2] + (rect.bh / font.height)
            self.batch:push(xx, yy, r, g, b, a, uv[1], uv[2])
            self.batch:push(xx+rect.bw, yy, r, g, b, a, uv[3], uv[2])
            self.batch:push(xx+rect.bw, yy+rect.bh, r, g, b, a, uv[3], uv[4])

            self.batch:push(xx, yy, r, g, b, a, uv[1], uv[2])
            self.batch:push(xx+rect.bw, yy+rect.bh, r, g, b, a, uv[3], uv[4])
            self.batch:push(xx, yy+rect.bh, r, g, b, a, uv[1], uv[4])

            ox = ox + rect.ax
            oy = oy + rect.ay
        end
    end
end

return Renderer