local shaders = {}

local vs_sources = {}
local ps_sources = {}

vs_sources['SPRITE2D'] = [[
#version 330 core
attribute vec3 a_position;
attribute vec4 a_color;
attribute vec2 a_texcoord;

layout (std140) uniform Matrices {
    mat4 world;
};


varying vec4 v_color;
varying vec2 v_texcoord;

void main() {
    gl_Position = world * vec4(a_position, 1.0);
    v_color = a_color;
    v_texcoord = a_texcoord;
}
]]

ps_sources['SPRITE2D'] =
[[
#version 120
varying vec4 v_color;
varying vec2 v_texcoord;

uniform sampler2D u_texture;

void main() {
    gl_FragColor = v_color * texture2D(u_texture, v_texcoord);
}
]]

vs_sources['PRIMITIVE2D'] = [[
#version 330 core
in vec3 a_position;
in vec4 a_color;

layout (std140) uniform Matrices {
    mat4 world;
};

out vec4 v_color;

void main() {
    gl_Position = world * vec4(a_position, 1.0);
    v_color = a_color;
}
]]

ps_sources['PRIMITIVE2D'] =
[[
#version 120

varying vec4 v_color;

void main() {
    gl_FragColor = v_color;
}
]]

function shaders.compile_all(renderer)
    local list = {
        ['SPRITE2D'] = {
            vertex = renderer:create_shader('vertex', vs_sources['SPRITE2D']),
            pixel = renderer:create_shader('pixel', ps_sources['SPRITE2D'])
        },
        ['PRIMITIVE2D'] = {
            vertex = renderer:create_shader('vertex', vs_sources['PRIMITIVE2D']),
            pixel = renderer:create_shader('pixel', ps_sources['PRIMITIVE2D'])
        }
    }

    return list
end

return shaders