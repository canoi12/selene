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

local Render = {}

function Render.create(win)
    local render = {}
    render.glContext = sdl.GLContext.create(win.handle)
    sdl.glMakeCurrent(win.handle, render.glContext)
    gl.loadGlad(sdl.glGetProcAddress())

    render.window = win
    render.state = {
        drawMode = "triangles",
        drawColor = Color.white,
        framebuffer = {},
        projection = Mat4.create()
    }


    for i,v in pairs(Color.white) do
      print(i, v)
    end
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
    canvas.width = width
    canvas.height = height

    render.state.projection:ortho(0, width, height, 0, -1, 1)

    local font = Font.default()

    render.whiteImage = white
    render.defaultCanvas = canvas
    render.defaultEffect = effect
    render.defaultFont = font

    sdl.glSetSwapInterval(true)

    return setmetatable(render, {
        __index = Render
    })
end

function Render:destroy()
    self.glContext:destroy()
end

function Render:clearColor(color)
    local cc  = {0, 0, 0, 1}
    for i,c in ipairs(color) do
        cc[i] = c / 255
    end
    gl.clearColor(table.unpack(cc))
end

local drawModes = {
    points = gl.POINTS,
    lines = gl.LINES,
    triangles = gl.TRIANGLES
}
local function setFramebuffer(r, t)
    if t ~= r.state.texture then
        r:finish()
        r.batch:clear()
        r.state.texture = t
        gl.Texture.bind(gl.TEXTURE_2D, t)
    end
end

local function setImage(r, t)
    t = t or r.whiteImage.handle
    if t ~= r.state.texture then
        r:finish()
        r.batch:clear()
        r.state.texture = t
        gl.Texture.bind(gl.TEXTURE_2D, t)
    end
end

local function setDrawMode(r, mode)
    if mode ~= r.state.drawMode then
        r:finish()
        r.batch:clear()
        r.state.drawMode = mode
    end
end

function Render:setEffect(effect)
    effect = effect or self.defaultEffect
    if effect.program ~= self.state.program then
        self:finish()
        self.batch:clear()
        self.state.program = effect.program
        gl.Program.use(effect.program)
        effect:send("u_MVP", self.state.projection)
    end
end

function Render:setFont(font)
    font = font or self.defaultFont
    self.state.font = font
end

function Render:setCanvas(canvas)
    canvas = canvas or self.defaultCanvas
    if canvas.handle ~= self.state.framebuffer then
        self:finish()
        self.batch:clear()
        gl.viewport(0, 0, canvas.width, canvas.height)
        gl.Framebuffer.bind(gl.FRAMEBUFFER, canvas.handle)
        self.state.framebuffer = canvas.handle
        
        self.state.projection:ortho(0, canvas.width, canvas.height, 0, -1, 1)
        local loc = self.state.program:getUniformLocation("u_MVP")
        gl.uniformMatrix4fv(loc, 1, false, self.state.projection)
    end
end

function Render:setDrawColor(c)
    self.state.drawColor = c
end

function Render:setClearColor(c)
    self.state.clearColor = c
    gl.clearColor(c:toFloat())
end

function Render:clear()
    gl.clear(gl.COLOR_BUFFER_BIT)
end

function Render:begin()
    self.batch:clear()

    self:setEffect()
    self:setCanvas()
    self:setFont()
    setDrawMode(self, "triangles")

    gl.enable(gl.BLEND)
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
    self.defaultCanvas.width, self.defaultCanvas.height = self.window:getSize()
    self:setDrawColor(Color.white)
end

function Render:finish()
    local mode = drawModes[self.state.drawMode]
    assert(mode, "Invalid draw mode")
    -- print(self.state.drawMode, self.batch:count())
    if not self.batch:flush() then return end
    local count = self.batch:count()
    self.vao:bind()
    gl.drawArrays(mode, 0, count)
    gl.VertexArray.unbind()
end

function Render:drawPoint(x, y)
    setImage(self)
    setDrawMode(self, 'points')
    local r,g,b,a = table.unpack(self.state.drawColor)
    self.batch:push(x, y, r, g, b, a, 0, 0)
end

function Render:drawLine(x0, y0, x1, y1)
    setImage(self)
    setDrawMode(self, 'lines')
    local r,g,b,a = table.unpack(self.state.drawColor)
    self.batch:push(x0, y0, r, g, b, a, 0.0, 0.0)
    self.batch:push(x1, y1, r, g, b, a, 0.0, 0.0)
end

function Render:drawRectangle(x, y, width, height)
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

function Render:fillRectangle(x, y, width, height)
    setImage(self)
    setDrawMode(self, 'triangles')
    local r,g,b,a = table.unpack(self.state.drawColor)
    self.batch:push(x, y, r, g, b, a, 0.0, 0.0)
    self.batch:push(x+width, y, r, g, b, a, 0.0, 0.0)
    self.batch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)

    self.batch:push(x, y, r, g, b, a, 0, 0)
    self.batch:push(x+width, y+height, r, g, b, a, 0, 0)
    self.batch:push(x, y+height, r, g, b, a, 0, 0)
end

function Render:drawCircle(x, y, radius, sides)
    setImage(self)
    setDrawMode(self, 'lines')
    sides = sides or 32.0
    local pi2 = math.pi * 2
    local r,g,b,a = table.unpack(self.state.drawColor)
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

function Render:fillCircle(x, y, radius, sides)
    setImage(self)
    setDrawMode(self, 'triangles')
    sides = sides or 32.0
    local pi2 = math.pi * 2
    local r,g,b,a = table.unpack(self.state.drawColor)
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

function Render:drawTriangle(p0, p1, p2)
    setImage(self)
    setDrawMode(self, 'lines')
    local r,g,b,a = table.unpack(self.state.drawColor)
    self.batch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p1[1], p1[2], r, g, b, a, 0.0, 0.0)

    self.batch:push(p1[2], p1[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)

    self.batch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
end

function Render:fillTriangle(p0, p1, p2)
    setImage(self)
    setDrawMode(self, 'triangles')
    local r,g,b,a = table.unpack(self.state.drawColor)
    self.batch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p1[1], p1[2], r, g, b, a, 0.0, 0.0)
    self.batch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)
end

function Render:copy(drawable, src, dest)
    setImage(self)
    setDrawMode(self, "triangles")
    local o = drawable
    
    local r,g,b,a = table.unpack(self.state.drawColor)
    local s = src or Rect.create(0, 0, o:getWidth(), o:getHeight())
    local d = dest or {x = 0, y = 0, w = s.w, h = s.h}
    -- local s = src
    -- local d = dest
    local uv = o:getUV(s)
    
    self.batch:push(d.x, d.y, r, g, b, a, uv[1], uv[2])
    self.batch:push(d.x+d.w, d.y, r, g, b, a, uv[3], uv[2])
    self.batch:push(d.x+d.w, d.y+d.h, r, g, b, a, uv[3], uv[4])
    
    self.batch:push(d.x, d.y, r, g, b, a, uv[1], uv[2])
    self.batch:push(d.x+d.w, d.y+d.h, r, g, b, a, uv[3], uv[4])
    self.batch:push(d.x, d.y+d.h, r, g, b, a, uv[1], uv[4])
end

function Render:print(text, x, y)
    setDrawMode(self, "triangles")
    
    local ox = x or 0
    local oy = y or 0

    local font = self.state.font
    local image = font.texture
    setImage(self, image)
    local r,g,b,a = table.unpack(self.state.drawColor)
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

return Render
