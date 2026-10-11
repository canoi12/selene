local
Font = require('plugins.RenderBatch2D.Font')
local RenderBatch2D = {}
RenderBatch2D.__index = RenderBatch2D

local function make_render_key(r, layer, pipeline, texture)
    local shader_id = r:get_object_id(pipeline)
    local texture_id = r:get_object_id(texture)
    return (layer * 1000000) + (shader_id * 10000) + texture_id
end

function RenderBatch2D.create(win, backend)
    local r = {}
    r.window = win
    r.width, r.height = win:get_size()
    backend = backend or 'opengl'
    r.handle = selene.create_renderer(win, backend)
    -- local renderer = r.handle
    r.backend = r.handle:get_backend()
    r.batch = renderer.VertexBatch(9 * 4, 1024)
    print(renderer)
    r.vertex = r.handle:create_buffer('vertex', 9 * 4 * 256)
    --r.handle:send_triangle(r.vertex)
    r.index = nil
    r.uniform = r.handle:create_buffer('uniform', 4*16)
    r.frame_data = r.handle:create_buffer('uniform', 128)
    r.object_data = r.handle:create_buffer('uniform', 128)

    r.camera = {
        x = 0, y = 0, zoom = 1,
        width = 0, height = 0,
    }

    r.draw_queue = {}

    r.pipeline = nil
    r.target = nil
    r.draw_list = { top = 0 }
    r.last_offset = 0
    -- local font_8x8 = font.create8x8()
    r.font = Font.create8x8(r)
    print(r.font_texture, r.font_glyphs)

    if backend == 'opengl' then
        r.shader_list = require('plugins.RenderBatch2D.gl_shaders').compile_all(r.handle)
    elseif backend == 'd3d11' then
        r.shader_list = require('plugins.RenderBatch2D.dx_shaders').compile_all(r.handle)
    elseif backend == 'vulkan' then
        r.shader_list = require('plugins.RenderBatch2D.spirv_shaders').compile_all(r.handle)
    end
    local shaders = r.shader_list

    r.pipelines = {
        ['SPRITE2D'] = r.handle:create_pipeline {
            layout = {
                stride = 9 * 4,
                { name = 'a_position', offset = 0,  size = 3, type = 'float' },
                { name = 'a_color',    offset = 12, size = 4, type = 'float' },
                { name = 'a_texcoord', offset = 28, size = 2, type = 'float' },
            },
            vs = shaders['SPRITE2D'].vertex,
            ps = shaders['SPRITE2D'].pixel,
            blend = {enabled = true, func = 'alpha'},
            scissor = {enabled = true},
            bindings = {
                {slot = 0, type = 'uniform', stage = {'vertex', 'pixel'}},
                {slot = 0, type = 'texture', stage = 'pixel'},
            }
        },
        ['PRIMITIVE2D'] = r.handle:create_pipeline {
            layout = {
                stride = 9 * 4,
                { name = 'a_position', offset = 0,  size = 3, type = 'float' },
                { name = 'a_color',    offset = 12, size = 4, type = 'float' },
            },
            vs = shaders['PRIMITIVE2D'].vertex,
            ps = shaders['PRIMITIVE2D'].pixel,
            bindings = {
                {slot = 0, type = 'uniform', stage = {'vertex', 'pixel'}},
            }
        },
        ['PRIMITIVE3D'] = r.handle:create_pipeline {
            layout = {
                stride = 9 * 4,
                { name = 'a_position', offset = 0,  size = 3, type = 'float' },
                { name = 'a_color',    offset = 12, size = 4, type = 'float' },
            },
            vs = shaders['PRIMITIVE2D'].vertex,
            ps = shaders['PRIMITIVE2D'].pixel,
            depth = {enabled = true, func = 'less'},
            bindings = {
                {slot = 0, type = 'uniform', stage = {'vertex', 'pixel'}},
            }
        }
    }
    --[[
    r.handle:destroy_shader(vertex_shader)
    r.handle:destroy_shader(pixel_shader)
    r.handle:destroy_shader(primitive_shaders[1])
    r.handle:destroy_shader(primitive_shaders[2])
    ]]
    return setmetatable(r, RenderBatch2D)
end

function RenderBatch2D:destroy()
    print('destroying RenderBatch2D')
    for k, pipeline in pairs(self.pipelines) do
        print(pipeline)
        self.handle:destroy_pipeline(pipeline)
    end
    print("Destroyed Pipelines")
    for i, shader in pairs(self.shader_list) do
        print(shader.vertex, shader.pixel)
        self.handle:destroy_shader(shader.vertex)
        self.handle:destroy_shader(shader.pixel)
    end
    if self.font then self.font:destroy(self) end
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

function RenderBatch2D:begin_pass(data)
    data = data or {}
    self.draw_list.top = 0
    self.last_offset = 0
    self.batch:reset()
    self.batch:set_color(1, 1, 1, 1)
    self.batch:set_z(0)

    self.handle:update_frame_data(
        self.frame_data,
    )

    if data.target then
        self.handle:set_render_target(data.target)
        self.target = data.target
    else
        self.handle:set_render_target(nil, false)
        self.target = nil
    end
    if data.clear_color then
        self.handle:clear_color(data.clear_color[1], data.clear_color[2], data.clear_color[3], data.clear_color[4])
    end
    if data.clear_depth then
        self.handle:clear_depth(data.clear_depth)
    end

    local target_w = data.target and data.target.width or self.width
    local target_h = data.target and data.target.height or self.height
    
    if data.viewport then
        self.handle:set_viewport(data.viewport[1], data.viewport[2], data.viewport[3], data.viewport[4])
    else
        self.handle:set_viewport(0, 0, target_w, target_h)
    end
    if data.scissor then
        self.handle:set_scissor(data.scissor[1], data.scissor[2], data.scissor[3], data.scissor[4])
    else
        self.handle:set_scissor(0, 0, target_w, target_h)
    end


    self.handle:set_vertex_buffer(self.vertex)
    local uniforms = data.uniforms or {}
    if not uniforms[0] then
        uniforms[0] = self.frame_data
    end
    for i, buf in pairs(uniforms) do
        self.handle:set_uniform_buffer(i, buf)
    end
end

function RenderBatch2D:end_pass()
    if #self.draw_queue == 0 then return end
    table.sort(self.draw_queue, function(a, b) return a.key < b.key end)

    local current_pipeline = nil
    local current_texture = nil

    for _, cmd in ipairs(self.draw_queue) do
        if cmd.pipeline ~= current_pipeline or cmd.texture ~= current_texture then
            self:push_draw('triangles')
            if cmd.pipeline ~= current_pipeline then
                self.handle:set_pipeline(cmd.pipeline)
                current_pipeline = cmd.pipeline
            end
            if cmd.texture ~= current_texture then
                self.handle:set_texture(cmd.texture)
                current_texture = cmd.texture
            end
        end
        if cmd.type == 'sprite' then
            self.batch:push_sprite(cmd.texture, unpack(cmd.args))
        elseif cmd.type == 'triangle' then
            self.batch:push_fill_triangle(unpack(cmd.args))
        elseif cmd.type == 'rect' then
            self.batch:push_fill_rect(unpack(cmd.args))
        elseif cmd.type == 'circle' then
            self.batch:push_fill_circle(unpack(cmd.args))
        elseif cmd.type == 'cube' then
            self.batch:push_fill_cube(unpack(cmd.args))
        elseif cmd.type == 'sphere' then
            self.batch:push_fill_sphere(unpack(cmd.args))
        elseif cmd.type == 'text' then
            self.font:draw(cmd.texture, unpack(cmd.args))
        elseif cmd.type == 'draw' then
            cmd.drawable:draw(self, unpack(cmd.args))
        end
    end

   self:push_draw('triangles')
    local vertex_size = self.batch:get_offset() * self.batch:get_stride()
    if vertex_size > 0 then
        self.handle:send_buffer_data(self.vertex, vertex_size, self.batch:get_data())
    end
    self.handle:flush()
    self.draw_queue = {}
end

function RenderBatch2D:present()
    self.handle:present()
end

function RenderBatch2D:begin_frame()
    self.draw_list.top = 0
    self.last_offset = 0
    self.batch:reset()
    self.batch:set_color(1, 1, 1, 1)
    self.batch:set_z(0)
    self.handle:set_vertex_buffer(self.vertex)
    self.handle:set_uniform_buffer(self.uniform)
    self:on_resize(self.width, self.height)
    self.pipeline = nil
    self.texture = nil
    self.target = nil
    self.handle:set_render_target(nil, false)
    --print('Begin')
end

function RenderBatch2D:end_frame()
    -- print('End')
    self:push_draw('triangles')

    self.handle:send_buffer_data(self.vertex, self.batch:get_offset() * self.batch:get_stride(), self.batch:get_data())
    -- print('Pushing')
    self.handle:present()
end

function RenderBatch2D:push_draw(mode)
    local offset = self.batch:get_offset()
    if self.last_offset ~= offset then
        --print('drawing')
        -- self:push_draw('triangles', self.last_offset, offset-self.last_offset)
        self.handle:draw(mode, self.last_offset, offset - self.last_offset)
        self.last_offset = offset
    end
end

function RenderBatch2D:set_canvas(canvas)
    if canvas ~= self.canvas then
        self:push_draw('triangles')
        if canvas then
            self.handle:set_render_target(canvas.target)
        else
            self.handle:set_render_target(nil)
        end
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
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE2D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'triangle', pipeline = pipeline, texture = texture, args = {x0, y0, x1, y1, x2, y2}}
    table.insert(self.draw_queue, cmd)
    -- if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
    --     self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
    --     self.pipeline = self.pipelines['PRIMITIVE2D']
    --     self:push_draw('triangles')
    -- end
    -- -- self.batch:push_fill_triangle(x0, y0, x1, y1, x2, y2)
    -- self.batch:push_vertex2d(x0, y0, 0, 1, 0, 0, 1, 0, 0)
    -- self.batch:push_vertex2d(x1, y1, 0, 0, 1, 0, 1, 0, 0)
    -- self.batch:push_vertex2d(x2, y2, 0, 0, 0, 1, 1, 0, 0)
end

function RenderBatch2D:draw_rectangle(x, y, w, h)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE2D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'rectangle', pipeline = pipeline, texture = texture, args = {x, y, w, h}}
    table.insert(self.draw_queue, cmd)
    
    -- print('drawing rectangle')
    -- if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
    --     self:push_draw('triangles')
    --     self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
    --     self.pipeline = self.pipelines['PRIMITIVE2D']
    -- end
    -- self.batch:push_fill_rect(x, y, w, h)
end

function RenderBatch2D:draw_circle(x, y, radius)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE2D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'circle', pipeline = pipeline, texture = texture, args = {x, y, radius}}
    table.insert(self.draw_queue, cmd)
    -- if self.pipelines['PRIMITIVE2D'] ~= self.pipeline then
    --     self:push_draw('triangles')
    --     self.handle:set_pipeline(self.pipelines['PRIMITIVE2D'])
    --     self.pipeline = self.pipelines['PRIMITIVE2D']
    -- end
    -- self.batch:set_color(1, 0, 1, 1)
    -- self.batch:set_z(32)
    -- self.batch:push_fill_circle(x, y, radius)
    -- self.batch:set_z(0)
    -- self.batch:set_color(1, 1, 1, 1)
end

function RenderBatch2D:draw_cube(x, y, sx, sy)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE3D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'cube', pipeline = pipeline, texture = texture, args = {x, y, sx, sy}}
    table.insert(self.draw_queue, cmd)

    -- if self.pipelines['PRIMITIVE3D'] ~= self.pipeline then
    --     self:push_draw('triangles')
    --     self.handle:set_pipeline(self.pipelines['PRIMITIVE3D'])
    --     self.pipeline = self.pipelines['PRIMITIVE3D']
    -- end
    -- self.batch:push_fill_cube({x, y, -320}, {math.rad(30)*self.time, math.rad(15)*self.time, 0}, {32, 32, 32})
end

function RenderBatch2D:draw_sphere(x, y, r)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE3D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'sphere', pipeline = pipeline, texture = texture, args = {x, y, r}}
    table.insert(self.draw_queue, cmd)
    -- if self.pipelines['PRIMITIVE3D'] ~= self.pipeline then
    --     self:push_draw('triangles')
    --     self.handle:set_pipeline(self.pipelines['PRIMITIVE3D'])
    --     self.pipeline = self.pipelines['PRIMITIVE3D']
    -- end
    -- self.batch:push_fill_sphere(x, y, 16, r)
end

function RenderBatch2D:draw_sprite(texture, ...)
    local args = {...}

    local layer = self.batch:get_z()
    local pipeline = self.pipelines['SPRITE2D']
    
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    table.insert(self.draw_queue, {
        key = render_key,
        type = 'sprite',
        texture = texture,
        pipeline = pipeline,
        args = args   
    })

    -- if self.pipelines['SPRITE2D'] ~= self.pipeline then
    --     self:push_draw('triangles')
    --     self.handle:set_pipeline(self.pipelines['SPRITE2D'])
    --     self.pipeline = self.pipelines['SPRITE2D']
    -- end
    -- if self.texture ~= texture then
    --     self:push_draw('triangles')
    --     self.handle:set_texture(texture)
    --     self.texture = texture
    -- end
    -- self.batch:push_sprite(texture, ...)
end

function RenderBatch2D:draw_text(font, text, x, y)
    local args = {text, x, y}
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['SPRITE2D']
    local render_key = make_render_key(self.handle, layer, pipeline, font.texture)
    table.insert(self.draw_queue, {
        key = render_key,
        type = 'text',
        font = font,
        pipeline = pipeline,
        args = args   
    })

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
    local args = {...}
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['SPRITE2D']
    local render_key = make_render_key(self.handle, layer, pipeline, drawable.texture)
    table.insert(self.draw_queue, {
        key = render_key,
        type = 'draw',
        drawable = drawable,
        pipeline = pipeline,
        args = args   
    })

    -- if self.pipelines['SPRITE2D'] ~= self.pipeline then
    --     self:push_draw('triangles')
    --     self.handle:set_pipeline(self.pipelines['SPRITE2D'])
    --     self.pipeline = self.pipelines['SPRITE2D']
    -- end
    -- if self.texture ~= drawable.texture then
    --     self:push_draw('triangles')
    --     self.handle:set_texture(drawable.texture)
    --     self.texture = drawable.texture
    -- end
    -- drawable:draw(self, ...)
end

return RenderBatch2D
