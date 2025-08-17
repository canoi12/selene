#include "modules/renderer.h"
#include "modules/window.h"
#ifndef SELENE_NO_OPENGL
extern char font8x8_basic[128][8];
extern char font8x8_control[32][8];
extern char font8x8_ext_latin[96][8];

static int aux_source_size = 0;
static char* aux_source = NULL;

const int gl_clear_masks_values[] = {GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT };

const int gl_enable_attribs_values[] = { GL_BLEND, GL_DEPTH_TEST, GL_SCISSOR_TEST, GL_CULL_FACE };

const int gl_pixel_formats_values[] = { GL_RGB, GL_RGBA,
#if defined(USE_GLES2)
    GL_DEPTH_COMPONENT16
#else
    GL_DEPTH_COMPONENT32
#endif
};

const int gl_texture_filters_values[] = { GL_NEAREST, GL_LINEAR };
const int gl_texture_targets_values[] = { GL_TEXTURE_2D, GL_TEXTURE_3D };

const int gl_draw_modes_values[] = {GL_POINTS, GL_LINES, GL_TRIANGLES};

#if defined(USE_GLES2)
const int gl_buffer_target_types_values[] = {GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER};
#else
const int gl_buffer_target_types_values[] = {GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_UNIFORM_BUFFER};
#endif
const int gl_shader_types_values[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

const int gl_front_face_values[] = {GL_CW, GL_CCW};
const int gl_cull_face_values[] = {GL_NONE, GL_FRONT, GL_BACK, GL_FRONT_AND_BACK};

const int gl_type_values[] = {GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT
#if !defined(USE_GLES2)
    , GL_DOUBLE
#endif
};

const int gl_comparison_funcs[] = {GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};

extern int l_RenderCommandList_create(lua_State* L);

static inline void gl_push_commands(selene_Renderer* r, int count, struct RenderCommand* cmd) {
    if (r->command_offset+count > r->command_count) {
        r->command_count *= 2;
        r->command_pool = (struct RenderCommand*)realloc(r->command_pool, sizeof(*cmd)*r->command_count);
    }
    memcpy(r->command_pool+r->command_offset, cmd, sizeof(*cmd)*count);
    r->command_offset += count;
}
#define gl_push_command(r, cmd) gl_push_commands(r, 1, cmd)

static inline void gl_enable_layout(selene_Renderer* r, struct selene_InputLayout* layout) {
    for (int j = 0; j < layout->count; j++) {
        int binding = layout->attributes[j].binding;
        if (binding == -1) continue;
        int offset = layout->attributes[j].offset;
        int size = layout->attributes[j].size;
        int type = layout->attributes[j].type;
        int stride = (int)layout->vertex_stride;
        glEnableVertexAttribArray(binding);
        glVertexAttribPointer(binding, size, type, GL_FALSE, stride, (const void*)offset);
    }
}

int l_GL_Renderer__destroy(lua_State* L) {
    CHECK_META(selene_Renderer);
    luaL_unref(L, LUA_REGISTRYINDEX, self->l_window_ref);
    if (aux_source) free(aux_source);
    free(self->command_pool);
    SDL_GL_DeleteContext(self->gl);
    return 0;
}


/**
 * Pipeline
 */
int l_GL_Renderer__create_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (!lua_istable(L, arg)) return luaL_argerror(L, arg, "must be a table");
    int top = arg;

    lua_getfield(L, arg, "vs");
    selene_Shader* vertex = (selene_Shader*)luaL_checkudata(L, -1, selene_Shader_METANAME);
    lua_getfield(L, arg, "ps");
    selene_Shader* pixel = (selene_Shader*)luaL_checkudata(L, -1, selene_Shader_METANAME);
    lua_pop(L, 2);

    GLuint vao = 0;
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    glGenVertexArrays(1, &vao);
#endif

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex->gl.handle);
    glAttachShader(program, pixel->gl.handle);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, log, NULL);
        glDeleteProgram(program);
        return luaL_error(L, "failed to link program: %s", log);
    }

    if (lua_getfield(L, arg, "layout") != LUA_TTABLE) return luaL_error(L, "invalid layout field");
    NEW_UDATA(selene_RenderPipeline, pipe);
    if (lua_getfield(L, -2, "stride") == LUA_TNUMBER) pipe->layout.vertex_stride = lua_tointeger(L, -1);
    lua_pop(L, 1);
    int len = lua_rawlen(L, -2);
    pipe->layout.count = len;
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -2, i+1);
        lua_getfield(L, -1, "name");
        const char* name = luaL_checkstring(L, -1);
        pipe->layout.attributes[i].binding = glGetAttribLocation(program, name);

        lua_pop(L, 1);
        lua_getfield(L, -1, "offset");
        pipe->layout.attributes[i].offset = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "size");
        const int size = lua_tointeger(L, -1);
        pipe->layout.attributes[i].size = size;
        lua_pop(L, 1);

        lua_getfield(L, -1, "type");
        int opt = luaL_checkoption(L, -1, NULL, type_name_options);
        pipe->layout.attributes[i].type = gl_type_values[opt];
        lua_pop(L, 1);
        fprintf(stdout, "name: %s, offset: %d, size: %d, type: %d\n", name, pipe->layout.attributes[i].offset, size, pipe->layout.attributes[i].type);
        switch (opt) {
            case 0:
            case 1:
                pipe->layout.attributes[i].stride = size;
                break;
            case 2:
            case 3:
                pipe->layout.attributes[i].stride = size * 2;
                break;
            default:
                pipe->layout.attributes[i].stride = size * 4;
        }
        lua_pop(L, 1);
    }
    fprintf(stdout, "registered layout\n");

    pipe->gl.program = program;
    pipe->gl.vao = vao;
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    glBindVertexArray(vao);
    for (int i = 0; i < len; i++) {
        int binding = pipe->layout.attributes[i].binding;
        int offset = pipe->layout.attributes[i].offset;
        int size = pipe->layout.attributes[i].size;
        int type = pipe->layout.attributes[i].type;
        int stride = pipe->layout.vertex_stride;
        glEnableVertexAttribArray(binding);
        glVertexAttribPointer(binding, size, type, GL_FALSE, stride, (const void*)offset);
    }
    glBindVertexArray(0);
#endif
    pipe->blend_state.enabled = 0;
    if (lua_getfield(L, arg, "blend") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) pipe->blend_state.enabled = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "func") == LUA_TSTRING) {
            const char* blendmode = luaL_optstring(L, -1, "alpha");
            if (strcmp(blendmode, "alpha") == 0) {
                pipe->blend_state.src = GL_SRC_ALPHA;
                pipe->blend_state.dst = GL_ONE_MINUS_SRC_ALPHA;
                pipe->blend_state.equation = GL_FUNC_ADD;
            }
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    pipe->depth_stencil_state.depth_enabled = 0;
    if (lua_getfield(L, arg, "depth") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) pipe->depth_stencil_state.depth_enabled = lua_toboolean(L, -1);
        //fprintf(stdout, "set depth: %d\n", pipe->gl.depth_stencil_state.depth_enabled);
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "func") == LUA_TSTRING) {
            int opt = luaL_checkoption(L, -1, NULL, comparison_func_options);
            pipe->depth_stencil_state.depth_func = gl_comparison_funcs[opt];
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    pipe->raster_state.cull_enabled = 0;
    if (lua_getfield(L, arg, "cull") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) pipe->raster_state.cull_enabled = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "front_face") == LUA_TSTRING) {
            int opt = luaL_checkoption(L, -1, "cw", front_face_options);
            pipe->raster_state.cull_face = gl_front_face_values[opt];
        }
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "cull_face") == LUA_TSTRING) {
            int opt = luaL_checkoption(L, -1, "back", cull_face_options);
            pipe->raster_state.cull_face = gl_cull_face_values[opt];
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    return 1;
}

int l_GL_Renderer__destroy_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderPipeline, pipe);
    glDeleteProgram(pipe->gl.program);
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    glDeleteVertexArrays(1, &(pipe->gl.vao));
#endif
    return 0;
}

/**
 * Buffers
 */

// create vertex, index or uniform buffer
int l_GL_Renderer__create_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    Uint32 handle;
    int opt = luaL_checkoption(L, arg++, "vertex", buffer_target_options);
    int size = (int)luaL_checkinteger(L, arg++);
    int target = gl_buffer_target_types_values[opt];
    glGenBuffers(1, &handle);
    glBindBuffer(target, handle);
    glBufferData(target, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(target, 0);
    NEW_UDATA(selene_GpuBuffer, buf);
    buf->type = opt;
    buf->size = size;
    buf->gl.handle = handle;
    buf->gl.type = target;
    return 1;
}

int l_GL_Renderer__destroy_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    glDeleteBuffers(1, &(buffer->gl.handle));
    return 0;
}

int l_GL_Renderer__send_buffer_data(lua_State* L) {
    INIT_ARG();
    selene_Renderer* self = (selene_Renderer*)lua_touserdata(L, arg++);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(size);
    if (!lua_isuserdata(L, arg)) return luaL_argerror(L, arg, "userdata or lightuserdata expected");
    void* data = lua_touserdata(L, arg++);
    int target = gl_buffer_target_types_values[buffer->type];
    // fprintf(stdout, "send buffer: %d %s\n", buffer->type, buffer_target_options[buffer->type]);
    glBindBuffer(target, buffer->gl.handle);
    if (size > buffer->size) {
        buffer->size = size;
        glBufferData(target, buffer->size, data, GL_DYNAMIC_DRAW);
    } else glBufferSubData(target, 0, size, data);
    glBindBuffer(target, 0);
    return 0;
}

int l_GL_Renderer__send_buffer_ortho(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(offset);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, n);
    CHECK_NUMBER(float, f);
    mat4 matrix;
    glm_mat4_identity(matrix);
    glm_ortho(left, right, bottom, top, n, f, matrix);
    int target = gl_buffer_target_types_values[buffer->type];
    // fprintf(stdout, "send buffer: %d %s\n", buffer->type, buffer_target_options[buffer->type]);
    int size = sizeof(matrix);
    glBindBuffer(target, buffer->gl.handle);
    if (size > buffer->size) {
        buffer->size = size;
        glBufferData(target, buffer->size, matrix, GL_DYNAMIC_DRAW);
    }
    else glBufferSubData(target, offset, size, matrix);
    glBindBuffer(target, 0);
    return 0;
}

int l_GL_Renderer__send_triangle(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    const float vertices[] = {
            0.0f, 0.5f, 0.f, 1.f, 0.f, 1.f, 1.f,
            0.5f, -0.5f, 0.f, 1.f, 1.f, 0.f, 1.f,
            -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f
    };
    glBindBuffer(GL_ARRAY_BUFFER, buffer->gl.handle);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

int l_GL_Renderer__send_rectangle(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    const float vertices[] = {
            -0.5f, 0.5f, 0.f, 1.f, 0.f, 1.f, 1.f,
            0.5f, 0.5f, 0.f, 1.f, 1.f, 0.f, 1.f,
            0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f,

            -0.5f, 0.5f, 0.f, 1.f, 0.f, 1.f, 1.f,
            0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f,
            -0.5f, -0.5f, 0.f, 1.f, 1.f, 0.f, 1.f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, buffer->gl.handle);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

/**
 * Textures
 */

int l_GL_Renderer__create_font(lua_State* L) {
    CHECK_META(selene_Renderer);
    int w = 2048;
    int h = 8;
    uint8_t* bitmap = (uint8_t*)malloc(w * h * 4);
    int ox = 0;
    int stride = w * sizeof(int);
    for (int i = 0; i < 128; i++) {
        for (int y = 0; y < 8; y++) {
            uint8_t l = font8x8_basic[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    ox = 128 * 8 * sizeof(int);
    for (int i = 0; i < 32; i++) {
        for (int y = 0; y < 8; y++) {
            uint8_t l = font8x8_control[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    ox = 160 * 8 * sizeof(int);
    for (int i = 0; i < 96; i++) {
        for (int y = 0; y < 8; y++) {
            uint8_t l = font8x8_ext_latin[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    NEW_UDATA(selene_Font, font);
    font->texture.width = w;
    font->texture.height = h;
    size_t size = w * h * 4;
#if 1
    glGenTextures(1, &(font->texture.gl.handle));
    glBindTexture(GL_TEXTURE_2D, font->texture.gl.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(bitmap);
#endif

    selene_FontGlyph* glyphs = font->glyphs;
    for (int i = 0; i < 256; i++) {
        glyphs[i].ax = 8 / w;
        glyphs[i].ay = 0;
        glyphs[i].bl = 0;
        glyphs[i].bt = 0;
        glyphs[i].bw = 8;
        glyphs[i].bh = 8;
        glyphs[i].tx = 8 * i;
    }
    return 1;
}

int l_GL_Renderer__create_texture2d(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int opt = luaL_checkoption(L, arg++, "rgba", pixel_formats);
    void* data = NULL;
    if (lua_isuserdata(L, arg)) data = lua_touserdata(L, arg++);
    int pixel_format = gl_pixel_formats_values[opt];
    Uint32 handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    NEW_UDATA(selene_Texture2D, tex);
    tex->gl.handle = handle;
    tex->width = width;
    tex->height = height;
    tex->format = opt;
    return 1;
}

int l_GL_Renderer__create_depth_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    Uint32 handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
    NEW_UDATA(selene_Texture2D, tex);
    tex->gl.handle = handle;
    tex->width = width;
    tex->height = height;
    tex->format = 2;
    return 1;
}

int l_GL_Renderer__destroy_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Texture2D, tex);
    glDeleteTextures(1, &(tex->gl.handle));
    return 0;
}

/**
 * Render Targets
 */
int l_GL_Renderer__create_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Texture2D, color);
    TEST_UDATA(selene_Texture2D, depth);

    GLuint handle;
    GLuint tex_color, tex_depth;
    glGenFramebuffers(1, &handle);
#if 0
    glGenTextures(1, &tex_color);
    if (depth) {
        glGenTextures(1, &tex_depth);
        glBindTexture(GL_TEXTURE_2D, tex_depth);
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
#endif
        glBindTexture(GL_TEXTURE_2D, 0);
    } else tex_depth = 0;

    glBindTexture(GL_TEXTURE_2D, tex_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->gl.handle, 0);
    if (depth) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->gl.handle, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    NEW_UDATA(selene_RenderTarget, target);
    target->gl.fbo = handle;
    target->color = color;
    target->depth = depth;
    target->width = color->width;
    target->height = color->height;
    return 1;
}

int l_GL_Renderer__destroy_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderTarget, target);
    glDeleteFramebuffers(1, &(target->gl.fbo));
    return 0;
}

/**
 * Shaders
 */

int l_GL_Renderer__create_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "vertex", shader_type_options);
    const char* source = NULL;
    if (lua_type(L, arg) == LUA_TSTRING) {
        source = luaL_checkstring(L, arg++);
    } else if (lua_isinteger(L, arg)) {
        int size = lua_tointeger(L, arg++);
        if (size > aux_source_size) {
            aux_source = realloc(aux_source, size + 1);
            aux_source_size = size + 1;
            fprintf(stdout, "alloc memory for aux_source: %d\n", size + 1);
            if (!aux_source)
                return luaL_error(L, "failed to alloc memory for shader compiler auxiliar source");
        }
        void* data = lua_touserdata(L, arg++);
        memcpy(aux_source, data, size);
        aux_source[size] = '\0';
        source = aux_source;
    }
    Uint32 handle = glCreateShader(gl_shader_types_values[opt]);
    glShaderSource(handle, 1, &source, NULL);
    int success;
    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(handle, 512, NULL, log);
        glDeleteShader(handle);
        return luaL_error(L, "failed to compile %s shader: %s", shader_type_options[opt], log);
    }
    NEW_UDATA(selene_Shader, shader);
    shader->type = opt;
    shader->gl.handle = handle;
    return 1;
}

int l_GL_Renderer__destroy_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Shader, shd);
    glDeleteShader(shd->gl.handle);
    return 0;
}


/**
 * Textures
 */
int l_GL_Renderer__create_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int opt = luaL_checkoption(L, arg++, "rgb", pixel_formats);
    void* data = lua_touserdata(L, arg++);

    Uint32 handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, gl_pixel_formats_values[opt], width, height, 0, gl_pixel_formats_values[opt], GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    NEW_UDATA(selene_Texture2D, tex);
    tex->width = width;
    tex->height = height;
    tex->gl.handle = handle;
    return 1;
}


/**
 * Draw and flush
 */
int l_GL_Renderer__draw(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "triangles", draw_modes);
    int mode = gl_draw_modes_values[opt];
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_DRAW_VERTEX;
    cmd.draw.mode = mode;
    cmd.draw.start = (int)luaL_checkinteger(L, arg++);
    cmd.draw.count = (int)luaL_checkinteger(L, arg++);
    gl_push_command(self, &cmd);
    return 0;
}

int l_GL_Renderer__flush(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct {
        selene_RenderPipeline* pipe;
        selene_GpuBuffer* vertex;
        selene_GpuBuffer* index;
        selene_GpuBuffer* uniform;
        selene_Texture2D* texture;
        selene_RenderTarget* target;
    } state;
    memset(&state, 0, sizeof(state));
    for (int i = 0; i < self->command_offset; i++) {
        struct RenderCommand* rc = self->command_pool + i;
        switch (rc->type) {
            case RENDER_COMMAND_CLEAR_COLOR: {
                float* c = rc->clear.color;
                glClearColor(c[0], c[1], c[2], c[3]);
                glClear(GL_COLOR_BUFFER_BIT);
            }
                break;
            case RENDER_COMMAND_CLEAR_DEPTH: {
            #if defined(USE_GLES2)
                glClearDepthf(rc->clear.depth);
            #else
                glClearDepth(rc->clear.depth);
            #endif
                glClear(GL_DEPTH_BUFFER_BIT);
            }
                break;
            case RENDER_COMMAND_SET_VIEWPORT:
                glViewport(rc->viewport.x, rc->viewport.y, rc->viewport.width, rc->viewport.height);
                break;
            case RENDER_COMMAND_SET_PIPELINE: {
                selene_RenderPipeline* rp = rc->pipeline;
                if (rp == state.pipe) break;
                if (rp == NULL) {
                    glUseProgram(0);
#if  !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
                    glBindVertexArray(0);
#endif
                    glDisable(GL_BLEND);
                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    state.pipe = rp;
                    break;
                }
                selene_RenderPipeline pipe;
                memset(&pipe, 0, sizeof(pipe));
                if (!state.pipe) state.pipe = &pipe;

                if (state.pipe->gl.program != rp->gl.program) {
                    glUseProgram(rp->gl.program);
        #if !defined(USE_GLES2)
                    if (state.uniform) glUniformBlockBinding(rp->gl.program, 0, 0);
        #endif
                }

                int diff_func0 = rp->blend_state.src != state.pipe->blend_state.src;
                int diff_func1 = rp->blend_state.dst != state.pipe->blend_state.dst;
                int diff_equation = rp->blend_state.equation != state.pipe->blend_state.equation;
                if (rp->blend_state.enabled != state.pipe->blend_state.enabled) {
                    if (rp->blend_state.enabled) glEnable(GL_BLEND);
                    else glDisable(GL_BLEND);
                }
                if (diff_func0 || diff_func1) glBlendFunc(rp->blend_state.src, rp->blend_state.dst);
                if (diff_equation) glBlendEquation(rp->blend_state.equation);

                if (rp->depth_stencil_state.depth_enabled != state.pipe->depth_stencil_state.depth_enabled) {
                    if (rp->depth_stencil_state.depth_enabled) glEnable(GL_DEPTH_TEST);
                    else glDisable(GL_DEPTH_TEST);
                }
                if (rp->depth_stencil_state.depth_func != state.pipe->depth_stencil_state.depth_func)
                    glDepthFunc(rp->depth_stencil_state.depth_func);

                if (rp->raster_state.cull_enabled != state.pipe->raster_state.cull_enabled) {
                    if (rp->raster_state.cull_enabled) glEnable(GL_CULL_FACE);
                    else glDisable(GL_CULL_FACE);
                }
                if (rp->raster_state.cull_face != state.pipe->raster_state.cull_face)
                    glCullFace(rp->raster_state.cull_face);
                if (rp->raster_state.front_face != state.pipe->raster_state.front_face)
                    glFrontFace(rp->raster_state.front_face);

#if  !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
                if (rp->gl.vao != state.pipe->gl.vao) {
                    glBindVertexArray(rp->gl.vao);
                    if (state.vertex)
                        gl_enable_layout(self, &(rp->layout));
                }
#else
                for (int j = 0; j < 8; j++) glDisableVertexAttribArray(j);
                gl_enable_layout(self, &(rp->layout));
#if 0
                for (int j = 0; j < rp->layout.count; j++) {
                    int binding = rp->layout.attributes[j].binding;
                    if (binding == -1) continue;
                    int offset = rp->layout.attributes[j].offset;
                    int size = rp->layout.attributes[j].size;
                    int type = rp->layout.attributes[j].type;
                    int stride = (int)rp->layout.vertex_stride;
                    glEnableVertexAttribArray(binding);
                    glVertexAttribPointer(binding, size, type, GL_FALSE, stride, (const void*)offset);
                }
#endif
#endif
                state.pipe = rp;
            }
                break;
            case RENDER_COMMAND_SET_VERTEX_BUFFER: {
                selene_GpuBuffer* b = rc->buffer.ptr;
                glBindBuffer(GL_ARRAY_BUFFER, b ? b->gl.handle : 0);
                selene_RenderPipeline* rp = state.pipe;
                if (b && rp) {
                    gl_enable_layout(self, &(rp->layout));
                }
                state.vertex = b;
            }
                break;
            case RENDER_COMMAND_SET_INDEX_BUFFER: {
                selene_GpuBuffer* b = rc->buffer.ptr;
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b ? b->gl.handle : 0);
                state.index = b;
            }
                break;
            case RENDER_COMMAND_SET_UNIFORM_BUFFER: {
                selene_GpuBuffer* b = rc->buffer.ptr;
#if defined(USE_GLES2)
                if (state.pipe->gl.program) {
                    GLuint program = state.pipe->gl.program;
                    GLint count;
                    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
                    for (GLint i = 0; i < count; ++i) {
                        char name[128];          // Buffer for uniform name
                        GLsizei length;          // Actual length of name
                        GLint size;              // Array size (usually 1 unless it's an array)
                        GLenum type;             // Uniform type (GL_FLOAT, GL_FLOAT_VEC3, etc.)

                        glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);
                        GLint location = glGetUniformLocation(program, name);

                        printf("Uniform #%d: name = %s, type = 0x%X, size = %d, location = %d\n",
                            i, name, type, size, location);
                    }
                }
#else
                glBindBuffer(GL_UNIFORM_BUFFER, b ? b->gl.handle : 0);
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, b->gl.handle);
                if (b && state.pipe) {
                    int index = glGetUniformBlockIndex(state.pipe->gl.program, "Matrices");
                    glUniformBlockBinding(state.pipe->gl.program, index, 0);
                }
#endif
                state.uniform = b;
            }
                break;
            case RENDER_COMMAND_SET_TEXTURE: {
                selene_Texture2D* t = (selene_Texture2D*)rc->texture.ptr;
                glActiveTexture(GL_TEXTURE0+rc->texture.slot);
                glBindTexture(rc->texture.target, t ? t->gl.handle : 0);
            }
                break;
            case RENDER_COMMAND_SET_SHADER:
                glUseProgram(rc->program.handle);
                break;
            case RENDER_COMMAND_SET_RENDER_TARGET: {
                selene_RenderTarget* t = rc->target.ptr;
                if (t != state.target) {
                    int width = t ? t->color->width : 640;
                    int height = t ? t->color->height : 380;
                    glBindFramebuffer(GL_FRAMEBUFFER, t ? t->gl.fbo : 0);
                    glViewport(0, 0, width, height);
                    if (state.uniform) {
                    #if !defined(USE_GLES2)
                        glBindBuffer(GL_UNIFORM_BUFFER, state.uniform->gl.handle);
                    #endif
                        mat4 matrix;
                        glm_mat4_identity(matrix);
                        glm_ortho(0, width, height, 0, -1000, 1000, matrix);
                    #if !defined(USE_GLES2)
                        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matrix), matrix);
                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    #endif
                    }
                }
                state.target = t;
            }
                break;
            case RENDER_COMMAND_SET_SCISSOR: {
                if (rc->scissor.enabled) glEnable(GL_SCISSOR_TEST);
                else glDisable(GL_SCISSOR_TEST);
                glScissor(rc->scissor.x, rc->scissor.y, rc->scissor.width, rc->scissor.height);
            }
                break;
#if 0
            case RENDER_COMMAND_SET_BLEND_MODE:
                glBlendFunc(rc->blend.func0, rc->blend.func1);
                glBlendEquation(rc->blend.equation);
                break;
#endif
            case RENDER_COMMAND_FLOAT_UNIFORM:
                glUniform1f(rc->uniform.location, rc->uniform.f);
                break;
            case RENDER_COMMAND_SET_PROJECTION:
            case RENDER_COMMAND_SET_VIEW:
            case RENDER_COMMAND_MATRIX_UNIFORM:
                glUniformMatrix4fv(rc->uniform.location, 1, GL_FALSE, rc->uniform.m[0]);
                break;
            case RENDER_COMMAND_DRAW_VERTEX:
                // fprintf(stdout, "draw_vertex: %d %d %d\n", rc->draw.mode, rc->draw.start, rc->draw.count);
                glDrawArrays(gl_draw_modes_values[rc->draw.mode], rc->draw.start, rc->draw.count);
                break;
            case RENDER_COMMAND_DRAW_INDEX:
                glDrawElements(gl_draw_modes_values[rc->draw.mode], rc->draw.count, GL_UNSIGNED_INT, NULL);
                break;
            case RENDER_COMMAND_DRAW_VERTEX_INSTANCED:
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glDrawArraysInstanced(gl_draw_modes_values[rc->draw.mode], rc->instanced.start, rc->instanced.count, rc->instanced.instance_count);
#endif
                break;
            case RENDER_COMMAND_DRAW_INDEX_INSTANCED:
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glDrawElementsInstanced(gl_draw_modes_values[rc->draw.mode], rc->instanced.count, GL_UNSIGNED_INT, NULL, rc->instanced.instance_count);
#endif
                break;
            default:
                break;
        }
    }
    self->command_offset = 0;
    glUseProgram(0);
#if  !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    glBindVertexArray(0);
#endif
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}

int l_GL_Renderer__present(lua_State* L) {
    CHECK_META(selene_Renderer);
    l_GL_Renderer__flush(L);
    SDL_GL_SwapWindow(self->window_ptr->handle);
    return 0;
}

int l_GL_Renderer_create(lua_State* L) {
#ifndef NDEBUG
    fprintf(stdout, "[selene] creating GL Renderer\n");
#endif
    INIT_ARG();
    selene_Window* win = (selene_Window*)luaL_checkudata(L, arg++, selene_Window_METANAME);
#ifndef NDEBUG
    fprintf(stdout, "[selene] window: %p\n", win);
    fprintf(stdout, "[selene] creating GL Context\n");
#endif
    SDL_GLContext ctx = SDL_GL_CreateContext(win->handle);
    if (!ctx) {
        return luaL_error(L, "failed to create SDL OpenGL context: %s", SDL_GetError());
    }
#ifndef NDEBUG
    fprintf(stdout, "[selene] GL Context created\n");
#endif
    NEW_UDATA(selene_Renderer, r);
    r->window_ptr = win;
    int width, height;
    SDL_GetWindowSize(win->handle, &width, &height);
    r->gl = ctx;
    SDL_GL_MakeCurrent(win->handle, ctx);
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
        return luaL_error(L, "Failed to init glad");
#endif
#ifndef NDEBUG
    fprintf(stdout, "[selene] modern OpenGL loaded\n");
#endif
    const char* version = glGetString(GL_VERSION);
    fprintf(stdout, "OpenGL Version: %s\n", version);
    r->command_offset = 0;
    r->command_count = 512;
    r->command_pool = (struct RenderCommand*)malloc(sizeof(struct RenderCommand) * r->command_count);

    lua_pushvalue(L, 1);
    r->l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    r->backend = SELENE_RENDERER_OPENGL;

    r->destroy = l_GL_Renderer__destroy;

    r->create_pipeline = l_GL_Renderer__create_pipeline;
    r->destroy_pipeline = l_GL_Renderer__destroy_pipeline;

    r->create_buffer = l_GL_Renderer__create_buffer;
    r->destroy_buffer = l_GL_Renderer__destroy_buffer;
    r->send_buffer_data = l_GL_Renderer__send_buffer_data;

    r->create_shader = l_GL_Renderer__create_shader;
    r->destroy_shader = l_GL_Renderer__destroy_shader;

    r->create_render_target = l_GL_Renderer__create_render_target;
    r->destroy_render_target = l_GL_Renderer__destroy_render_target;

    r->create_texture2d = l_GL_Renderer__create_texture2d;
    r->create_depth_texture = l_GL_Renderer__create_depth_texture;
    r->destroy_texture = l_GL_Renderer__destroy_texture;

    r->draw = l_GL_Renderer__draw;

    r->flush = l_GL_Renderer__flush;
    r->present = l_GL_Renderer__present;

    return 1;
}
#endif
