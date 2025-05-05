local Font = require('plugins.RenderBatch2D.Font')
local RenderBatch2D = {}
RenderBatch2D.__index = RenderBatch2D

function RenderBatch2D.create(win, backend)
    local r = {}
    r.window = win
    r.width, r.height = win:get_size()
    backend = backend or 'opengl'
    r.handle = selene.create_renderer(win, backend)
    local renderer = r.handle
    r.backend = r.handle:get_backend()
    r.batch = selene.renderer.VertexBatch(9*4, 1024)
    print(renderer)
    r.vertex = renderer:create_buffer('vertex', 9*4*256)
    --renderer:send_triangle(r.vertex)
    r.index = nil
    r.uniform = renderer:create_buffer('uniform', 4*16)
    r.pipeline = nil
    r.target = nil
    r.draw_list = {top = 0}
    r.last_offset = 0
    local font_8x8 = font.create8x8()
    r.font = Font.create8x8(r)
    print(r.font_texture, r.font_glyphs)

    if backend == 'opengl' then
        r.shader_list = require('plugins.RenderBatch2D.gl_shaders').compile_all(renderer)
    elseif backend == 'dx11' then
        r.shader_list = require('plugins.RenderBatch2D.dx_shaders').compile_all(renderer)
    end
    local shaders = r.shader_list

    r.pipelines = {
        ['SPRITE2D'] = renderer:create_pipeline{
            layout = {
                stride = 9*4,
                {name = 'a_position', offset = 0, size = 3, type = 'float'},
                {name = 'a_color', offset = 12, size = 4, type = 'float'},
                {name = 'a_texcoord', offset = 28, size = 2, type = 'float'},
            },
            vs = shaders['SPRITE2D'].vertex,
            ps = shaders['SPRITE2D'].pixel,
            blend = {enabled = true, func = 'alpha'},
            scissor = {enabled = true}
        },
        ['PRIMITIVE2D'] = renderer:create_pipeline{
            layout = {
                stride = 9*4,
                {name = 'a_position', offset = 0, size = 3, type = 'float'},
                {name = 'a_color', offset = 12, size = 4, type = 'float'},
            },
            vs = shaders['PRIMITIVE2D'].vertex,
            ps = shaders['PRIMITIVE2D'].pixel
        },
        ['PRIMITIVE3D'] = renderer:create_pipeline{
            layout = {
                stride = 9*4,
                {name = 'a_position', offset = 0, size = 3, type = 'float'},
                {name = 'a_color', offset = 12, size = 4, type = 'float'},
            },
            vs = shaders['PRIMITIVE2D'].vertex,
            ps = shaders['PRIMITIVE2D'].pixel,
            depth = {enabled = true, func = 'less'}
        }
    }
    --[[
    renderer:destroy_shader(vertex_shader)
    renderer:destroy_shader(pixel_shader)
    renderer:destroy_shader(primitive_shaders[1])
    renderer:destroy_shader(primitive_shaders[2])
    ]]
    return setmetatable(r, RenderBatch2D)
end

function RenderBatch2D:destroy()
    print('destroying RenderBatch2D')
    for k,pipeline in pairs(self.pipelines) do
        print(pipeline)
        self.handle:destroy_pipeline(pipeline)
    end
    print("Destroyed Pipelines")
    for i,shader in pairs(self.shader_list) do
        print(shader.vertex, shader.pixel)
        self.handle:destroy_shader(shader.vertex)
        self.handle:destroy_shader(shader.pixel)
    end
    if self.vertex then self.handle:destroy_buffer(self.vertex) end
    if self.index then self.handle:destroy_buffer(self.index) end
    if self.uniform then self.handle:destroy_buffer(self.uniform) end
    if self.batch then self.batch:destroy() end
    print("Cleaned data")
end

function RenderBatch2D:on_resize(w, h)
    self.width = w
    self.height = h
    self.handle:set_viewport(0, 0, w, h)
    self:set_clip_rect(0, 0, w, h)
    self.handle:send_buffer_ortho(self.uniform, 0, 0, w, h, 0, -1000, 1000)
end

function RenderBatch2D:begin_frame()
    self.draw_list.top = 0
    self.last_offset = 0
    self.batch:set_color(1, 1, 1, 1)
    self.batch:set_z(0)
    self.handle:set_vertex_buffer(self.vertex)
    self.handle:set_uniform_buffer(self.uniform)
    self:on_resize(self.width, self.height)
    self.pipeline = nil
    self.texture = nil
    self.target = nil
    self.handle:set_render_target(nil, false)
    self.batch:reset()
end

function RenderBatch2D:end_frame()
    self:push_draw('triangles')
    
    self.handle:send_buffer_data(self.vertex, self.batch:get_offset()*self.batch:get_stride(), self.batch:get_data())
    -- print('Pushing')
    self.handle:present()
end

function RenderBatch2D:push_draw(mode)
    local offset = self.batch:get_offset()
    if self.last_offset ~= offset then
        -- self:push_draw('triangles', self.last_offset, offset-self.last_offset)
        self.handle:draw(mode, self.last_offset, offset-self.last_offset)
        self.last_offset = offset
    end
end

function RenderBatch2D:set_canvas(canvas)
    if canvas ~= self.canvas then
        self:push_draw('triangles')
        if canvas then self.handle:set_render_target(canvas.target)
        else self.handle:set_render_target(nil) end
        self.canvas = canvas
    end
end

function RenderBatch2D:set_viewport(x, y, w, h)
    self.handle:set_viewport(x, y, w, h)
end

function RenderBatch2D:set_clip_rect(x, y, w, h)
    self:push_draw('triangles')
    x = x or 0
    y = y or 0
    w = w or self.width
    h = h or self.height
    self.handle:set_scissor(x, y, w, h)
end

function RenderBatch2D:set_color(x, y, w, h)
    self.batch:set_color(x, y, w, h)
end

function RenderBatch2D:set_texture(drawable)
    if not drawable then self.handle:set_texture()
    else
        self.handle:set_texture(drawable.texture)
    end
end

function RenderBatch2D:clear(r, g, b, a)
    self.handle:clear_color(r, g, b, a)
end

function RenderBatch2D:clear_depth(depth)
    self.handle:clear_depth(depth)
end

function RenderBatch2D:enable_3d(enable)
    if enable then
        if self.pipelines['PRIMITIVE3D'] ~= self.pipeline then
            self:push_draw('triangles')
            self.handle:set_pipeline(self.pipelines['PRIMITIVE3D'])
            self.pipeline = self.pipelines['PRIMITIVE3D']
        end
        self.handle:set_render_target(self.canvas.target, true)
        self:clear_depth(1)
    else
        self:push_draw('triangles')
        self.handle:set_render_target(self.canvas.target, false)
    end
end

function RenderBatch2D:draw_point(x, y)
    if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
        self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
        self.pipeline = self.pipelines['PRIMITIVE2D']
    end
end

function RenderBatch2D:draw_line(x0, y0, x1, y1)
    if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
        self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
        self.pipeline = self.pipelines['PRIMITIVE2D']
    end
end

function RenderBatch2D:draw_triangle(x0, y0, x1, y1, x2, y2)
    if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
        self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
        self.pipeline = self.pipelines['PRIMITIVE2D']
        self:push_draw('triangles')
    end
    -- self.batch:push_fill_triangle(x0, y0, x1, y1, x2, y2)
    self.batch:push_vertex2d(x0, y0, 0, 1, 0, 0, 1, 0, 0)
    self.batch:push_vertex2d(x1, y1, 0, 0, 1, 0, 1, 0, 0)
    self.batch:push_vertex2d(x2, y2, 0, 0, 0, 1, 1, 0, 0)
end

function RenderBatch2D:draw_rectangle(x, y, w, h)
    -- print('drawing rectangle')
    if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
        self:push_draw('triangles')
        self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
        self.pipeline = self.pipelines['PRIMITIVE2D']
    end
    self.batch:push_fill_rect(x, y, w, h)
end

function RenderBatch2D:draw_circle(x, y, radius)
    if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
        self:push_draw('triangles')
        self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
        self.pipeline = self.pipelines['PRIMITIVE2D']
    end
    self.batch:set_color(1, 0, 1, 1)
    self.batch:set_z(32)
    self.batch:push_fill_circle(x, y, radius)
    self.batch:set_z(0)
    self.batch:set_color(1, 1, 1, 1)
end

function RenderBatch2D:draw_cube(x, y, sx, sy)
    if self.pipelines['PRIMITIVE3D'] ~= self.pipeline then
        self:push_draw('triangles')
        self.handle:set_pipeline(self.pipelines['PRIMITIVE3D'])
        self.pipeline = self.pipelines['PRIMITIVE3D']
    end
    self.batch:push_fill_cube({x, y, -320}, {math.rad(30)*self.time, math.rad(15)*self.time, 0}, {32, 32, 32})
end

function RenderBatch2D:draw_sphere(x, y, r)
    if self.pipelines['PRIMITIVE3D'] ~= self.pipeline then
        self:push_draw('triangles')
        self.handle:set_pipeline(self.pipelines['PRIMITIVE3D'])
        self.pipeline = self.pipelines['PRIMITIVE3D']
    end
    self.batch:push_fill_sphere(x, y, 16, r)
end

function RenderBatch2D:draw_sprite(texture, ...)
    if self.pipelines['SPRITE2D'] ~= self.pipeline then
        self:push_draw('triangles')
        self.handle:set_pipeline(self.pipelines['SPRITE2D'])
        self.pipeline = self.pipelines['SPRITE2D']
    end
    if self.texture ~= texture then
        self:push_draw('triangles')
        self.handle:set_texture(texture)
        self.texture = texture
    end
    self.batch:push_sprite(texture, ...)
end

function RenderBatch2D:draw_text(font, text, x, y)
    if self.pipelines['SPRITE2D'] ~= self.pipeline then
        self:push_draw('triangles')
        -- print(self.texture)
        self.handle:set_pipeline(self.pipelines['SPRITE2D'])
        self.pipeline = self.pipelines['SPRITE2D']
    end
    if self.texture ~= font.texture then
        self:push_draw('triangles')
        self.handle:set_texture(font.texture)
        self.texture = font.texture
    end
    self.batch:push_text(font.texture, font.glyphs, text, x, y)
end

function RenderBatch2D:draw(drawable, ...)
    if self.pipelines['SPRITE2D'] ~= self.pipeline then
        self:push_draw('triangles')
        self.handle:set_pipeline(self.pipelines['SPRITE2D'])
        self.pipeline = self.pipelines['SPRITE2D']
    end
    if self.texture ~= drawable.texture then
        self:push_draw('triangles')
        self.handle:set_texture(drawable.texture)
        self.texture = drawable.texture
    end
    drawable:draw(self, ...)
end

return RenderBatch2D
