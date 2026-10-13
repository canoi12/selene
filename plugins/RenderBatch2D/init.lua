local Font = require('plugins.RenderBatch2D.Font')
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
    r.info = r.handle:get_info()
    r.backend = r.info.backend

    -- Vertex batch for building geometry
    r.batch = renderer.VertexBatch(9 * 4, 1024)
    r.vertex = r.handle:create_buffer('vertex', 9 * 4 * 256)
    r.index = nil
    r.uniform = r.handle:create_buffer('uniform', 4*16)
    -- FrameData struct: 2 mat4 + 4 floats = 144 bytes
    r.frame_data = r.handle:create_frame_data()
    r.frame_buffer = r.handle:create_buffer('uniform', 2 * 64 + 16)
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
    r.font = Font.create8x8(r)
    r.white_texture = r.handle:create_texture2d(1, 1, 'rgba', '\255\255\255\255')

    -- Shaders come from plugins.Shaders. The helper picks the right backend
    -- variant (glsl / hlsl / spirv) for the active renderer, so no per-backend
    -- branching is needed here.
    local shader_lib = require('plugins.Shaders')
    local shaders = {
        ['SPRITE2D'] = {
            vertex = shader_lib.get(r.handle, 'sprite2d', 'vertex'),
            pixel = shader_lib.get(r.handle, 'sprite2d', 'pixel'),
        },
        ['PRIMITIVE2D'] = {
            vertex = shader_lib.get(r.handle, 'primitive2d', 'vertex'),
            pixel = shader_lib.get(r.handle, 'primitive2d', 'pixel'),
        },
    }
    r.shader_list = shaders

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
            blend = {enabled = true, func = 'alpha'},
            scissor = {enabled = true},
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
            blend = {enabled = true, func = 'alpha'},
            scissor = {enabled = true},
            bindings = {
                {slot = 0, type = 'uniform', stage = {'vertex', 'pixel'}},
            }
        }
    }
    print('Renderer created with backend: ' .. r.backend)
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
    if self.white_texture then self.handle:destroy_texture(self.white_texture) end
    if self.vertex then self.handle:destroy_buffer(self.vertex) end
    if self.index then self.handle:destroy_buffer(self.index) end
    if self.uniform then self.handle:destroy_buffer(self.uniform) end
    if self.frame_buffer then self.handle:destroy_buffer(self.frame_buffer) end
    if self.batch then self.batch:destroy() end
    print("Cleaned data")
end

function RenderBatch2D:on_resize(w, h)
    self.width = w
    self.height = h
    self.handle:set_viewport(0, 0, w, h)
    self:set_clip_rect(0, 0, w, h)
    self.handle:send_buffer_ortho(self.uniform, 0, 0, w, h, 0, -1000, 1000)
    self.frame_data:screen_width(w)
    self.frame_data:screen_height(h)
    self.frame_data:ortho(0, w, h, 0, -1000, 1000)
    self.fd_width, self.fd_height = w, h
end

function RenderBatch2D:begin_pass(data)
    data = data or {}
    self.draw_list.top = 0
    self.last_offset = 0
    self.batch:reset()
    self.batch:set_color(1, 1, 1, 1)
    self.batch:set_z(0)

    self.handle:update_frame_data(
        self.frame_buffer, self.frame_data
    )

    if data.target then
        self.handle:set_render_target(data.target)
        self.target = data.target
    else
        self.handle:set_render_target(nil, false)
        self.target = nil
    end
    if data.clear_color then
        self.handle:clear_color(data.clear_color[1], data.clear_color[2], data.clear_color[3], data.clear_color[4] or 1.0)
    end
    if data.clear_depth then
        self.handle:clear_depth(data.clear_depth)
    end

    local target_w = data.target and data.target.width or self.width
    local target_h = data.target and data.target.height or self.height

    -- the frame data projection must match the current target size
    if target_w ~= self.fd_width or target_h ~= self.fd_height then
        self.frame_data:ortho(0, target_w, target_h, 0, -1000, 1000)
        self.frame_data:screen_width(target_w)
        self.frame_data:screen_height(target_h)
        self.fd_width, self.fd_height = target_w, target_h
        self.handle:update_frame_data(self.frame_buffer, self.frame_data)
    end

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
        uniforms[0] = self.frame_buffer
    end
    for i, buf in pairs(uniforms) do
        self.handle:set_uniform_buffer(i, buf)
    end
end

function RenderBatch2D:end_pass()
    if #self.draw_queue > 0 then
    table.sort(self.draw_queue, function(a, b) return a.key < b.key end)

    local current_pipeline = nil
    local current_texture = nil

    for _, cmd in ipairs(self.draw_queue) do
        if cmd.color then
            self.batch:set_color(table.unpack(cmd.color))
        end
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
            self.batch:push_sprite(cmd.texture, table.unpack(cmd.args))
        elseif cmd.type == 'triangle' then
            self.batch:push_fill_triangle(table.unpack(cmd.args))
        elseif cmd.type == 'rect' then
            self.batch:push_fill_rect(table.unpack(cmd.args))
        elseif cmd.type == 'circle' then
            self.batch:push_fill_circle(table.unpack(cmd.args))
        elseif cmd.type == 'cube' then
            self.batch:push_fill_cube(table.unpack(cmd.args))
        elseif cmd.type == 'sphere' then
            self.batch:push_fill_sphere(table.unpack(cmd.args))
        elseif cmd.type == 'text' then
            self.font:draw(cmd.texture, table.unpack(cmd.args))
        elseif cmd.type == 'draw' then
            cmd.drawable:draw(self, table.unpack(cmd.args))
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
    self.handle:end_pass()
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
    -- present is explicit: call render:present() after end_frame
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
            self.handle:set_render_target(canvas.target, true)
        else
            self.handle:set_render_target(nil, false)
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

function RenderBatch2D:set_color(r, g, b, a)
    self.batch:set_color(r, g, b, a or 1)
end

function RenderBatch2D:set_texture(tex)
    -- accepts a raw texture or a Canvas (unwraps its texture)
    if type(tex) == 'table' and tex.texture then tex = tex.texture end
    if tex ~= self.texture then
        self:push_draw('triangles')
        self.handle:set_texture(tex)
        self.texture = tex
    end
end

function RenderBatch2D:enable_3d(enable)
    if enable then
        if self.pipelines['PRIMITIVE3D'] ~= self.pipeline then
            self:push_draw('triangles')
            self.handle:set_pipeline(self.pipelines['PRIMITIVE3D'])
            self.pipeline = self.pipelines['PRIMITIVE3D']
        end
        if self.canvas then
            self.handle:set_render_target(self.canvas.target, true)
        end
        self:clear_depth(1)
    else
        self:push_draw('triangles')
        if self.canvas then
            self.handle:set_render_target(self.canvas.target, false)
        end
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
    local cmd = {key = render_key, type = 'triangle', pipeline = pipeline, texture = texture, color = {self.batch:get_color()}, args = {x0, y0, x1, y1, x2, y2}}
    table.insert(self.draw_queue, cmd)
end

function RenderBatch2D:draw_rectangle(x, y, w, h)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE2D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'rect', pipeline = pipeline, texture = texture, color = {self.batch:get_color()}, args = {x, y, w, h}}
    table.insert(self.draw_queue, cmd)
end

function RenderBatch2D:draw_circle(x, y, radius)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE2D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'circle', pipeline = pipeline, texture = texture, color = {self.batch:get_color()}, args = {x, y, radius}}
    table.insert(self.draw_queue, cmd)
end

function RenderBatch2D:draw_cube(pos, rot, scale, textured)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE3D']
    local texture = textured and self.white_texture or self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'cube', pipeline = pipeline, texture = texture, color = {self.batch:get_color()}, args = {pos, rot, scale, textured}}
    table.insert(self.draw_queue, cmd)
end

function RenderBatch2D:draw_sphere(x, y, r)
    local layer = self.batch:get_z()
    local pipeline = self.pipelines['PRIMITIVE3D']
    local texture = self.white_texture
    local render_key = make_render_key(self.handle, layer, pipeline, texture)
    local cmd = {key = render_key, type = 'sphere', pipeline = pipeline, texture = texture, color = {self.batch:get_color()}, args = {x, y, r}}
    table.insert(self.draw_queue, cmd)
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
        texture = drawable.texture,
        pipeline = pipeline,
        args = args
    })
end

function RenderBatch2D:clear_color(r, g, b, a)
    self.handle:clear_color(r, g, b, a or 1)
end

function RenderBatch2D:clear_depth(depth)
    self.handle:clear_depth(depth)
end

function RenderBatch2D:get_white_texture()
    return self.white_texture
end

return RenderBatch2D
