#include "renderer.h"

const char* c_vertex_header =
#if defined(USE_GLES2)
"#version 100\n"
"attribute vec3 a_position;\n"
"attribute vec4 a_color;\n"
"attribute vec2 a_texcoord;\n"
"varying vec4 v_color;\n"
"varying vec2 v_texcoord;\n"
#else
"#version 150\n"
"in vec3 a_position;\n"
"in vec4 a_color;\n"
"in vec2 a_texcoord;\n"
"out vec4 v_color;\n"
"out vec2 v_texcoord;\n"
#endif
"uniform mat4 u_model_view;\n"
"uniform mat4 u_projection;";

const char* c_vertex_position_default =
"vec4 position(vec4 pos, mat4 view, mat4 projection) {\n"
"   return pos;\n"
"}\n";

const char* c_vertex_main =
"void main() {\n"
"   gl_Position = position(vec4(a_position, 1), u_model_view, u_projection);\n"
"   v_color = a_color;\n"
"   v_texcoord = a_texcoord;\n"
"}\n";

const char* c_fragment_header =
#if defined(USE_GLES2)
"#version 100\n"
"precision mediump float;\n"
"varying vec4 v_color;\n"
"varying vec2 v_texcoord;\n"
"#define fragColor gl_FragColor\n"
#else
"#version 150\n"
"in vec4 v_color;\n"
"in vec2 v_texcoord;\n"
"out vec4 fragColor;\n"
#endif
"uniform sampler2D u_texture;\n";

const char* c_fragment_pixel_default =
"vec4 pixel(vec4 color, sampler2D tex, vec2 texcoord) {\n"
"   return color * texture2D(tex, texcoord);\n"
"}\n";

const char* c_fragment_main =
"void main() {\n"
"   fragColor = pixel(v_color, u_texture, v_texcoord);\n"
"}\n";

int l_Effect2D__call(lua_State* L) {
    const char* position = luaL_optstring(L, 1, c_vertex_position_default);
    const char* pixel = luaL_optstring(L, 2, c_fragment_pixel_default);

    Uint32 vertex_shader, fragment_shader;
    int success = 0;

    /* Vertex shader*/
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_source[] = { c_vertex_header, position, c_vertex_main };
    glShaderSource(vertex_shader, 3, vertex_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        fprintf(stderr, "Failed to compile vertex shader: %s\n", log);
        return luaL_error(L, "Failed to compile vertex shader: %s", log);
    }

    /* Fragment shader */
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_source[] = { c_fragment_header, pixel, c_fragment_main };
    glShaderSource(fragment_shader, 3, fragment_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        fprintf(stderr, "Failed to compile fragment shader: %s\n", log);
        return luaL_error(L, "Failed to compile fragment shader: %s", log);
    }
    /* Program */
    Uint32 program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        fprintf(stderr, "Failed to link shader program: %s\n", log);
        return luaL_error(L, "Failed to link shader program: %s", log);
    }
    Effect2D* effect = (Effect2D*)lua_newuserdata(L, sizeof(Effect2D));
    luaL_setmetatable(L, "Effect2D");
    effect->handle = program;
    effect->position_location = glGetAttribLocation(program, "a_position");
    effect->color_location = glGetAttribLocation(program, "a_color");
    effect->texcoord_location = glGetAttribLocation(program, "a_texcoord");
    effect->model_view_location = glGetUniformLocation(program, "u_model_view");
    effect->projection_location = glGetUniformLocation(program, "u_projection");
    glUseProgram(program);
    float m[] = {
                    1.f, 0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f
                };
    glUniformMatrix4fv(effect->model_view_location, 1, GL_FALSE, m);
    glUniformMatrix4fv(effect->projection_location, 1, GL_FALSE, m);
    glUseProgram(0);
    return 1;
}

int l_Effect2D_destroy(lua_State* L) {
    Effect2D* effect = (Effect2D*)lua_touserdata(L, 1);
    glDeleteProgram(effect->handle);
    return 0;
}

int l_Effect2D_open_meta(lua_State* L) {
    luaL_newmetatable(L, "Effect2D");
    lua_pushcfunction(L, l_Effect2D__call);
    lua_setfield(L, -2, "__call");
    lua_pushcfunction(L, l_Effect2D_destroy);
    lua_setfield(L, -2, "destroy");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 0;
}