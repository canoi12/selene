local gl = selene.gl
local sdl = selene.sdl2

local Color = require 'core.graphics.Color'

local Batch = require 'core.graphics.Batch'
local Canvas = require 'core.graphics.Canvas'
local Effect = require 'core.graphics.Effect'
local Font = require 'core.graphics.Font'
local Image = require 'core.graphics.Image'

local Vec2 = require('core.math.Vec2')
local Mat4 = require('core.math.Mat4')

local Rect = require 'core.Rect'

--- @class RenderCommand
--- @field clearColor Color
--- @field clearFlags integer[]
--- @field clipRect Rect
--- @field effect Effect
--- @field canvas Canvas
--- @field drawable Drawable
--- @field translation number[]
--- @field scale number[]
--- @field rotation number
--- @field drawInfo {mode:integer,first:integer,count:integer}
--- @field call function
local RenderCommand = {}

local blitCount = 0

local pass = function(cmd, r) end

function RenderCommand.create(func)
    local cmd = {}
    cmd.drawInfo = {}
    cmd.call = func or pass

    return cmd
end

--- @param cmd RenderCommand
--- @param r core.Renderer
local clearRender = function (cmd, r)
    gl.clearColor(cmd.clearColor:toFloat())
    gl.clear(table.unpack(cmd.clearFlags))
end

--- @param cmd RenderCommand
--- @param r core.Renderer
local blitRender = function(cmd, r)
    blitCount = blitCount + 1
    local info = cmd.drawInfo
    if r.cameraUpdated and r.currentEffect then
        r.currentEffect:send("u_View", r.modelview)
        r.cameraUpdated = false
    end
    gl.drawArrays(info.mode, info.first, info.count)
end

local backToDefaultCanvas = RenderCommand.create(
    --- @param cmd RenderCommand
    --- @param r core.Renderer
    function (cmd, r)
        gl.Framebuffer.bind(gl.FRAMEBUFFER)
        local canvas = r.defaultCanvas
        local effect = r.currentEffect
        gl.viewport(0, 0, canvas.width, canvas.height)
        r.projection:ortho(0, canvas.width, canvas.height, 0, -1, 1)
        effect:send("u_MVP", r.projection)
        effect:send("u_View", r.modelview)
    end
)

local backToDefaultEffect = RenderCommand.create(
    --- @param cmd RenderCommand
    --- @param r core.Renderer
    function(cmd, r)
        local effect = r.defaultEffect
        gl.Program.use(effect.program)
        effect:send("u_MVP", r.projection)
        effect:send("u_View", r.modelview)
    end
)

--- @param cmd RenderCommand
--- @param r core.Renderer
local backToWhiteImage = RenderCommand.create(
    function(cmd, r)
        local d = r.whiteImage
        gl.Texture.bind(gl.TEXTURE_2D, d.texture)
    end
)

--- @param cmd RenderCommand
--- @param r core.Renderer
local setClipRect = function (cmd, r)
    local rect = cmd.clipRect
    local size = r.size
    gl.enable(gl.SCISSOR_TEST)
    gl.scissor(rect.x, size[2] - rect.h - rect.y, rect.w, rect.h)
end

local disableScissor = RenderCommand.create(
    --- @param cmd RenderCommand
    --- @param r core.Renderer
    function(cmd, r)
        gl.disable(gl.SCISSOR_TEST)
    end
)

local resetModelView = RenderCommand.create(
    --- @param cmd RenderCommand
    --- @param r core.Renderer
    function(cmd, r)
        r.modelview:identity()
        r.cameraUpdated = true
    end
)

--- @param cmd RenderCommand
--- @param r core.Renderer
local translateModelView = function(cmd, r)
    r.modelview:translate(cmd.translation[1], cmd.translation[2])
    r.cameraUpdated = true
end

--- @param cmd RenderCommand
--- @param r core.Renderer
local scaleModelView = function(cmd, r)
    r.modelview:scale(cmd.scale[1], cmd.scale[2])
    r.cameraUpdated = true
end

--- @param cmd RenderCommand
--- @param r core.Renderer
local rotateModelView = function(cmd, r)
    r.modelview:rotate(cmd.rotation)
    r.cameraUpdated = true
end

--- @param cmd RenderCommand
--- @param r core.Renderer
local setCanvas = function(cmd, r)
    gl.Framebuffer.bind(gl.FRAMEBUFFER, cmd.canvas.handle)
    local canvas = cmd.canvas
    local effect = cmd.effect
    gl.viewport(0, 0, canvas.width, canvas.height)
    r.projection:ortho(0, canvas.width, canvas.height, 0, -1, 1)
    effect:send("u_MVP", r.projection)
    -- effect:send("u_View", r.modelview)
end

--- @param cmd RenderCommand
--- @param r core.Renderer
local setEffect = function(cmd, r)
    local effect = cmd.effect
    effect.program:use()
    effect:send("u_MVP", r.projection)
    effect:send("u_View", r.modelview)
    r.cameraUpdated = false
end

--- @param cmd RenderCommand
--- @param r core.Renderer
local setDrawable = function(cmd, r)
    local d = cmd.drawable
    gl.Texture.bind(gl.TEXTURE_2D, d.texture)
end

--- @class core.Renderer
--- @field glContext selene.sdl2.GLContext
--- @field window Window
--- @field clearColor Color
--- @field drawColor Color
--- @field vao selene.gl.VertexArray
--- @field currentBatchOffset integer
--- @field clearFlags integer[]
--- @field drawMode integer
--- @field whiteImage Image
--- @field defaultCanvas Canvas
--- @field defaultEffect Effect
--- @field defaultFont Font
--- @field defaultBatch Batch
--- @field size Vec2
--- @field currentDrawable Drawable
--- @field currentEffect Effect
--- @field currentFont Font
--- @field projection selene.linmath.Mat4
--- @field modelview selene.linmath.Mat4
--- @field cameraUpdated boolean
--- @field commandPool {top:integer,commands:RenderCommand[]}
--- @field drawCommands RenderCommand[]
local Renderer = {}

--- Creates a new renderer
--- @param win Window
--- @return core.Renderer
function Renderer.create(win)
    -- local win = app.window
    local render = {}

    render.glContext = sdl.GLContext.create(win.handle)
    sdl.glMakeCurrent(win.handle, render.glContext)
    gl.loadGlad(sdl.glGetProcAddress())

    render.window = win

    render.vao = gl.VertexArray.create()
    render.defaultBatch = Batch.create(1000)

    local batch = render.defaultBatch

    render.vao:bind()
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

    local font = Font.default()

    render.whiteImage = white
    render.defaultCanvas = canvas
    render.defaultEffect = effect
    render.defaultFont = font

    render.currentFont = font
    render.currentCanvas = canvas
    render.currentEffect = effect
    render.currentDrawable = white

    render.clearFlags = {gl.COLOR_BUFFER_BIT}

    render.size = Vec2.create(win.width, win.height)

    render.projection = Mat4.create()
    render.projection:ortho(0, render.size[1], render.size[2], 0, -1, 1)
    render.modelview = Mat4.create()

    render.clearColor = Color.black
    render.drawColor = Color.white

    render.drawMode = gl.LINES

    render.cameraUpdated = false

    render.commandPool = {
        top = 1,
        commands = {}
    }
    for i=1,512 do
        render.commandPool.commands[i] = RenderCommand.create()
    end

    render.drawCommands = {}

    return setmetatable (
        render,
        { __index = Renderer }
    )
end

function Renderer:destroy()
    self.glContext:destroy()
end

function Renderer:onResize(w, h)
    gl.viewport(0, 0, w, h)
    self.size[1] = w
    self.size[2] = h
    self.defaultCanvas.width = w
    self.defaultCanvas.height = h
    print('Resizing: ', w, h)
end

function Renderer:begin()
    blitCount = 0
    for i,cmd in ipairs(self.drawCommands) do
        self.drawCommands[i] = nil
    end
    self.currentBatchOffset = 0
    self.defaultBatch:clear()
    self.commandPool.top = 1

    gl.enable(gl.BLEND)
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

    local cmd = self:newCommand(setCanvas)
    cmd.canvas = self.defaultCanvas
    cmd.effect = self.defaultEffect
    table.insert(self.drawCommands, cmd)
    table.insert(self.drawCommands, backToDefaultEffect)
    table.insert(self.drawCommands, backToWhiteImage)
    table.insert(self.drawCommands, resetModelView)

    self.currentCanvas = self.defaultCanvas
    self.currentEffect = self.defaultEffect
    self.currentDrawable = self.whiteImage
end


function Renderer:finish()
    self:pushBlitCommand()
    self.defaultBatch:flush()
    self.vao:bind()
    for i,cmd in ipairs(self.drawCommands) do
        cmd:call(self)
    end
    gl.VertexArray.unbind()
end

function Renderer:setClearColor(col)
    self.clearColor = col or Color.black
end

function Renderer:setDrawColor(col)
    self.drawColor = col or Color.white
end

function Renderer:newCommand(call)
    local pool = self.commandPool
    local cmd = pool.commands[pool.top]
    cmd.call = call
    if pool.top == #pool.commands then
        local size = pool.top * 2
        for i=pool.top+1,size do
            pool.commands[i] = RenderCommand.create()
        end
    end
    pool.top = pool.top + 1
    return cmd
end

function Renderer:pushBlitCommand()
    if self.currentBatchOffset == self.defaultBatch:getCount() then return end
    local cmd = self:newCommand(blitRender)
    cmd.drawInfo.mode = self.drawMode
    cmd.drawInfo.first = self.currentBatchOffset
    cmd.drawInfo.count = self.defaultBatch:getCount() - self.currentBatchOffset
    self.currentBatchOffset = self.defaultBatch:getCount()
    table.insert(self.drawCommands, cmd)
end

function Renderer:clear()
    local cmd = self:newCommand(clearRender)
    cmd.clearColor = self.clearColor
    cmd.clearFlags = {}
    for i,flag in ipairs(self.clearFlags) do
        cmd.clearFlags[i] = flag
    end
    table.insert(self.drawCommands, cmd)
end

--- @param r core.Renderer
--- @param m integer
local function checkDrawMode(r, m)
    if r.drawMode ~= m then
        r:pushBlitCommand()
        r.drawMode = m
    end
end

--- @param r core.Renderer
--- @param d Drawable
local function checkDrawable(r, d)
    if r.currentDrawable ~= d then
        r:pushBlitCommand()
        local cmd = r:newCommand(setDrawable)
        cmd.drawable = d
        table.insert(r.drawCommands, cmd)
        r.currentDrawable = d
    end
end

--- @param r core.Renderer
local function checkWhiteImage(r)
    if r.currentDrawable ~= r.whiteImage then
        r:pushBlitCommand()
        table.insert(r.drawCommands, backToWhiteImage)
        r.currentDrawable = r.whiteImage
    end
end

function Renderer:identity()
    self:pushBlitCommand()
    table.insert(self.drawCommands, resetModelView)
end

--- Translate the camera matrix
--- @param x number
--- @param y number
function Renderer:translate(x, y)
    self:pushBlitCommand()
    local cmd = self:newCommand(translateModelView)
    cmd.translation = {x, y}
    table.insert(self.drawCommands, cmd)
end

--- Scale the camera matrix
--- @param x number
--- @param y number
function Renderer:scale(x, y)
    self:pushBlitCommand()
    local cmd = self:newCommand(scaleModelView)
    cmd.scale = {x, y}
    table.insert(self.drawCommands, cmd)
end

--- Rotate the camera matrix
--- @param angle number
function Renderer:rotate(angle)
    self:pushBlitCommand()
    local cmd = self:newCommand(rotateModelView)
    cmd.rotation = angle
    table.insert(self.drawCommands, cmd)
end

--- @param canvas Canvas | nil
function Renderer:setCanvas(canvas)
    canvas = canvas or self.defaultCanvas
    if canvas ~= self.currentCanvas then
        self:pushBlitCommand()
        local cmd = self:newCommand(setCanvas)
        cmd.effect = self.currentEffect
        cmd.canvas = canvas
        table.insert(self.drawCommands, cmd)
        self.currentCanvas = canvas
    end
end

--- @param effect Effect | nil
function Renderer:setEffect(effect)
    effect = effect or self.defaultEffect
    if effect ~= self.currentEffect then
        self:pushBlitCommand()
        local cmd = self:newCommand(setEffect)
        cmd.effect = effect
        table.insert(self.drawCommands, cmd)
        self.currentEffect = effect
    end
end

--- @param rect Rect | nil
function Renderer:setClipRect(rect)
    self:pushBlitCommand()
    if rect then
        local cmd = self:newCommand(setClipRect)
        cmd.clipRect = rect
        table.insert(self.drawCommands, cmd)
    else
        table.insert(self.drawCommands, disableScissor)
    end
end

function Renderer:drawPoint(x, y)
    checkWhiteImage(self)
    checkDrawMode(self, gl.POINTS)
    local r,g,b,a = self.drawColor:toFloat()
    self.defaultBatch:push(x, y, r, g, b, a, 0, 0)
end

function Renderer:drawLine(x0, y0, x1, y1)
    checkWhiteImage(self)
    checkDrawMode(self, gl.LINES)
    local r,g,b,a = self.drawColor:toFloat()
    self.defaultBatch:push(x0, y0, r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(x1, y1, r, g, b, a, 0.0, 0.0)
end

function Renderer:drawRectangle(x, y, width, height)
    checkWhiteImage(self)
    checkDrawMode(self, gl.LINES)
    local r,g,b,a = self.drawColor:toFloat()
    self.defaultBatch:push(x, y, r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(x+width, y, r, g, b, a, 0.0, 0.0)

    self.defaultBatch:push(x+width, y, r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)

    self.defaultBatch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(x, y+height, r, g, b, a, 0.0, 0.0)

    self.defaultBatch:push(x, y+height, r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(x, y, r, g, b, a, 0.0, 0.0)
end

function Renderer:fillRectangle(x, y, width, height)
    checkWhiteImage(self)
    checkDrawMode(self, gl.TRIANGLES)
    local r,g,b,a = self.drawColor:toFloat()
    self.defaultBatch:push(x, y, r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(x+width, y, r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(x+width, y+height, r, g, b, a, 0.0, 0.0)

    self.defaultBatch:push(x, y, r, g, b, a, 0, 0)
    self.defaultBatch:push(x+width, y+height, r, g, b, a, 0, 0)
    self.defaultBatch:push(x, y+height, r, g, b, a, 0, 0)
end

function Renderer:drawCircle(x, y, radius, side)
    checkWhiteImage(self)
    checkDrawMode(self, gl.LINES)
    local sides = sides or 32.0
    local pi2 = math.pi * 2
    local r,g,b,a = self.drawColor:toFloat()
    local u,v = 0, 0
    for i=1,sides do
    local tetha = ((i-1) * pi2) / sides

    local xx = x + (math.cos(tetha) * radius)
    local yy = y + (math.sin(tetha) * radius)
    self.defaultBatch:push(xx, yy, r, g, b, a, u, v)

    tetha = (i * pi2) / sides
    xx = x + (math.cos(tetha) * radius)
    yy = y + (math.sin(tetha) * radius)
    self.defaultBatch:push(xx, yy, r, g, b, a, u, v)
    end
end

function Renderer:fillCircle(x, y, radius, sides)
    checkWhiteImage(self)
    checkDrawMode(self, gl.TRIANGLES)
    sides = sides or 32.0
    local pi2 = math.pi * 2
    local r,g,b,a = self.drawColor:toFloat()
    local u, v = 0, 0
    for i=1,sides do
        self.defaultBatch:push(x, y, r, g, b, a, u, v)

        local tetha = ((i-1) * pi2) / sides
        local xx = x + (math.cos(tetha) * radius)
        local yy = y + (math.sin(tetha) * radius)
        self.defaultBatch:push(xx, yy, r, g, b, a, u, v)

        tetha = (i * pi2) / sides
        xx = x + (math.cos(tetha) * radius)
        yy = y + (math.sin(tetha) * radius)
        self.defaultBatch:push(xx, yy, r, g, b, a, u, v)
    end
end

function Renderer:drawTriangle(p0, p1, p2)
    checkWhiteImage(self)
    checkDrawMode(self, gl.LINES)
    local r,g,b,a = self.drawColor:toFloat()
    self.defaultBatch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(p1[1], p1[2], r, g, b, a, 0.0, 0.0)

    self.defaultBatch:push(p1[2], p1[2], r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)

    self.defaultBatch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
end

function Renderer:fillTriangle(p0, p1, p2)
    checkWhiteImage(self)
    checkDrawMode(self, gl.TRIANGLES)
    local r,g,b,a = self.drawColor:toFloat()
    self.defaultBatch:push(p0[1], p0[2], r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(p1[1], p1[2], r, g, b, a, 0.0, 0.0)
    self.defaultBatch:push(p2[1], p2[2], r, g, b, a, 0.0, 0.0)
end

--- @param drawable Drawable
---@param src Rect | nil
---@param dest Rect | nil
function Renderer:blit(drawable, src, dest)
    checkDrawable(self, drawable)
    checkDrawMode(self, gl.TRIANGLES)
    local o = drawable

    local r,g,b,a = self.drawColor:toFloat()
    local s = src or Rect.create(0, 0, o:getWidth(), o:getHeight())
    local d = dest or {x = 0, y = 0, w = s.w, h = s.h}
    local uv = o:getUV(s)
    -- local uv = {0, 0, 1, 1}

    self.defaultBatch:push(d.x, d.y, r, g, b, a, uv[1], uv[2])
    self.defaultBatch:push(d.x+d.w, d.y, r, g, b, a, uv[3], uv[2])
    self.defaultBatch:push(d.x+d.w, d.y+d.h, r, g, b, a, uv[3], uv[4])

    self.defaultBatch:push(d.x, d.y, r, g, b, a, uv[1], uv[2])
    self.defaultBatch:push(d.x+d.w, d.y+d.h, r, g, b, a, uv[3], uv[4])
    self.defaultBatch:push(d.x, d.y+d.h, r, g, b, a, uv[1], uv[4])
end

function Renderer:print(text, x, y)
    checkDrawMode(self, gl.TRIANGLES)
    local ox = x or 0
    local oy = y or 0
    local font = self.currentFont
    checkDrawable(self, font)
    local r,g,b,a = self.drawColor:toFloat()
    local i = 1
    while i <= #text do
        local c = text:byte(i)
        local codepoint, bytes = selene.UTF8Codepoint(text, i)
        i = i + bytes
        local rect = font.rects[codepoint]
        if c == string.byte('\n') then
            ox = x
            oy = oy + font.height
        elseif c == string.byte('\t') then
            ox = ox + (rect.bw * 2)
        else
            local xx = ox + rect.bl
            local yy = oy + rect.bt

            local uv = {}
            uv[1] = rect.tx / font.width
            uv[2] = 0
            uv[3] = uv[1] + (rect.bw / font.width)
            uv[4] = uv[2] + (rect.bh / font.height)
            self.defaultBatch:push(xx, yy, r, g, b, a, uv[1], uv[2])
            self.defaultBatch:push(xx+rect.bw, yy, r, g, b, a, uv[3], uv[2])
            self.defaultBatch:push(xx+rect.bw, yy+rect.bh, r, g, b, a, uv[3], uv[4])

            self.defaultBatch:push(xx, yy, r, g, b, a, uv[1], uv[2])
            self.defaultBatch:push(xx+rect.bw, yy+rect.bh, r, g, b, a, uv[3], uv[4])
            self.defaultBatch:push(xx, yy+rect.bh, r, g, b, a, uv[1], uv[4])

            ox = ox + rect.ax
            oy = oy + rect.ay
        end
    end
end

return Renderer