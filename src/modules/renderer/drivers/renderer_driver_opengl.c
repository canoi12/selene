#include "modules/renderer.h"
#include "modules/window.h"
#ifndef SELENE_NO_OPENGL
extern char font8x8_basic[128][8];
extern char font8x8_control[32][8];
extern char font8x8_ext_latin[96][8];

static int aux_source_size = 0;
static char* aux_source = NULL;

/* On GLES2 SDKs that only expose VAO through GL_OES_vertex_array_object,
 * alias the core entry-point names to the OES ones (detected at build
 * time by the CMake probes). */
#ifdef SELENE_GLES_VAO_OES
#define glGenVertexArrays glGenVertexArraysOES
#define glBindVertexArray glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#endif
/* On GLES2 SDKs instancing only exists through GL_ANGLE_instanced_arrays */
#ifdef SELENE_GLES_INSTANCING_OES
#define glDrawArraysInstanced glDrawArraysInstancedANGLE
#define glDrawElementsInstanced glDrawElementsInstancedANGLE
#endif
/* GLES2 core only has glClearDepthf (glClearDepth is desktop GL) */
#ifdef SELENE_GLES_GLCLEARDEPTH_F
#define glClearDepth glClearDepthf
#endif

const int gl_clear_masks_values[] = {GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT };

const int gl_enable_attribs_values[] = { GL_BLEND, GL_DEPTH_TEST, GL_SCISSOR_TEST, GL_CULL_FACE };

/* Formats that depend on the context version are resolved at runtime
 * by gl_pixel_format(); only universally-defined tokens go here. */
const int gl_pixel_formats_values[] = { GL_RGB, GL_RGBA, 0, 0 };

const int gl_texture_filters_values[] = { GL_NEAREST, GL_LINEAR };
const int gl_texture_targets_values[] = { GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };

const int gl_draw_modes_values[] = {GL_POINTS, GL_LINES, GL_TRIANGLES};

/* GL_UNIFORM_BUFFER / GL_DOUBLE only exist in GL 3.1+ / GLES 3 headers;
 * runtime capability checks gate their use. */
const int gl_buffer_target_types_values[] = {
    GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
#if defined(GL_UNIFORM_BUFFER)
    GL_UNIFORM_BUFFER
#else
    0
#endif
};
const int gl_shader_types_values[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

const int gl_front_face_values[] = {GL_CW, GL_CCW};
const int gl_cull_face_values[] = {GL_NONE, GL_FRONT, GL_BACK, GL_FRONT_AND_BACK};

const int gl_type_values[] = {GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT
#if defined(GL_DOUBLE)
    , GL_DOUBLE
#endif
};

const int gl_comparison_funcs[] = {GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};

static inline void gl_enable_layout(selene_Renderer* r, struct selene_InputLayout* layout) {
    for (int j = 0; j < layout->count; j++) {
        int binding = layout->attributes[j].binding;
        if (binding == -1) continue;
        int offset = layout->attributes[j].offset;
        int size = layout->attributes[j].size;
        int type = layout->attributes[j].type;
        int stride = (int)layout->vertex_stride;
        glEnableVertexAttribArray(binding);
        glVertexAttribPointer(binding, size, gl_type_values[type], GL_FALSE, stride, (const void*)(uintptr_t)offset);
    }
}

int l_GL_Renderer__destroy(lua_State* L) {
    CHECK_META(selene_Renderer);
    luaL_unref(L, LUA_REGISTRYINDEX, self->l_window_ref);
    if (aux_source) free(aux_source);
    free(self->command_pool);
    SDL_GL_DeleteContext(self->gl.ctx);
    return 0;
}


static void gl_apply_blend_mode(selene_Renderer* r, int mode) {
    switch (mode) {
        case SELENE_BLEND_ADDITIVE:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            if (r->gl.caps.has_blend_equation) glBlendEquation(GL_FUNC_ADD);
            break;
        case SELENE_BLEND_SUBTRACT:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            if (r->gl.caps.has_blend_equation) glBlendEquation(GL_FUNC_SUBTRACT);
            break;
        case SELENE_BLEND_MULTIPLY:
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            if (r->gl.caps.has_blend_equation) glBlendEquation(GL_FUNC_ADD);
            break;
        default:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if (r->gl.caps.has_blend_equation) glBlendEquation(GL_FUNC_ADD);
            break;
    }
}

static void gl_apply_viewport(selene_Renderer* r, selene_RenderTarget* target, int x, int y, int w, int h) {
    int tw, th;
    renderer_get_target_size(r, target, &tw, &th);
    glViewport(x, th - y - h, w, h);
}

/* Resolve a SELENE_FORMAT_* pixel format for the current context.
 * Returns 0 when the format is not supported. */
static int gl_pixel_format(selene_Renderer* r, int opt) {
    struct selene_GLCapabilities* caps = &r->gl.caps;
    switch (opt) {
        case SELENE_FORMAT_DEPTH:
            if (!caps->has_depth_texture) return 0;
            if (caps->is_gles) {
#if defined(GL_DEPTH_COMPONENT24)
                if (caps->major >= 3) return GL_DEPTH_COMPONENT24;
#endif
                return GL_DEPTH_COMPONENT;
            }
#if defined(GL_DEPTH_COMPONENT32)
            return GL_DEPTH_COMPONENT32;
#else
            return GL_DEPTH_COMPONENT;
#endif
        case SELENE_FORMAT_BGRA:
#if defined(GL_BGRA)
            if (caps->has_bgra) return GL_BGRA;
#endif
            return 0;
        default:
            return gl_pixel_formats_values[opt];
    }
}

/**
 * Pipeline
 */
int l_GL_Renderer__create_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct selene_PipelineDesc desc;
    renderer_parse_pipeline_desc(L, arg, &desc);

    GLuint vao = 0;
#ifndef SELENE_NO_VAO
    if (self->gl.caps.has_vao)
        glGenVertexArrays(1, &vao);
#endif

    GLuint program = glCreateProgram();
    glAttachShader(program, desc.vs->gl.handle);
    glAttachShader(program, desc.ps->gl.handle);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        glDeleteProgram(program);
        return luaL_error(L, "failed to link program: %s", log);
    }

    NEW_UDATA(selene_RenderPipeline, pipe);
    memset(pipe, 0, sizeof(*pipe));
    renderer_apply_pipeline_desc(pipe, &desc);
    for (int i = 0; i < pipe->layout.count; i++) {
        if (pipe->layout.attributes[i].type == SELENE_DOUBLE && self->gl.caps.is_gles)
            return luaL_error(L, "double vertex attributes are not supported on GLES");
        pipe->layout.attributes[i].binding = glGetAttribLocation(program, pipe->layout.attributes[i].name);
    }

    pipe->gl.program = program;
    pipe->gl.vao = vao;
#ifndef SELENE_NO_VAO
    if (self->gl.caps.has_vao) {
        glBindVertexArray(vao);
        for (int i = 0; i < pipe->layout.count; i++) {
            int binding = pipe->layout.attributes[i].binding;
            if (binding < 0) continue;
            int offset = pipe->layout.attributes[i].offset;
            int size = pipe->layout.attributes[i].size;
            int type = pipe->layout.attributes[i].type;
            int stride = (int)pipe->layout.vertex_stride;
            glEnableVertexAttribArray(binding);
            glVertexAttribPointer(binding, size, gl_type_values[type], GL_FALSE, stride, (const void*)(uintptr_t)offset);
        }
        glBindVertexArray(0);
    }
#endif
    pipe->id = next_pipeline_id++;
    return 1;
}

int l_GL_Renderer__destroy_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderPipeline, pipe);
    glDeleteProgram(pipe->gl.program);
#ifndef SELENE_NO_VAO
    if (self->gl.caps.has_vao)
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
    if (opt == SELENE_BUFFER_STORAGE)
        return luaL_error(L, "storage buffers are not implemented on OpenGL yet");
    if (opt == SELENE_BUFFER_UNIFORM && !self->gl.caps.has_ubo)
        return luaL_error(L, "uniform buffers are not supported by this context");
    int size = (int)luaL_checkinteger(L, arg++);
    int target = gl_buffer_target_types_values[opt];
    glGenBuffers(1, &handle);
    glBindBuffer(target, handle);
    glBufferData(target, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(target, 0);
    NEW_UDATA(selene_GpuBuffer, buf);
    buf->type = opt;
    buf->size = size;
    buf->index_type = (opt == SELENE_BUFFER_INDEX) ? SELENE_INDEX_UINT32 : 0;
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
    renderer_clip_ortho(self, left, right, bottom, top, n, f, matrix);
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
#if 0
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
#endif

int l_GL_Renderer__create_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int opt = luaL_checkoption(L, arg++, "rgb", pixel_formats);
    int pixel_format = gl_pixel_format(self, opt);
    if (!pixel_format)
        return luaL_error(L, "pixel format '%s' is not supported by this context", pixel_formats[opt]);
    void* data = NULL;
    if (lua_type(L, arg) == LUA_TSTRING) data = (void*)lua_tostring(L, arg++);
    else if (lua_isuserdata(L, arg)) data = lua_touserdata(L, arg++);

    Uint32 handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    NEW_UDATA(selene_Texture2D, tex);
    tex->id = next_texture_id;
    next_texture_id++;
    tex->width = width;
    tex->height = height;
    tex->gl.handle = handle;
    return 1;
}


int l_GL_Renderer__create_texture2d(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int opt = luaL_checkoption(L, arg++, "rgba", pixel_formats);
    void* data = NULL;
    if (lua_isuserdata(L, arg)) data = lua_touserdata(L, arg++);
    int pixel_format = gl_pixel_format(self, opt);
    if (!pixel_format)
        return luaL_error(L, "pixel format '%s' is not supported by this context", pixel_formats[opt]);
    Uint32 handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
    tex->id = next_texture_id;
    next_texture_id++;
    return 1;
}

int l_GL_Renderer__create_depth_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int depth_format = gl_pixel_format(self, SELENE_FORMAT_DEPTH);
    if (!depth_format)
        return luaL_error(L, "depth textures are not supported by this context");
    GLenum type = self->gl.caps.is_gles ? GL_UNSIGNED_INT : GL_FLOAT;
    Uint32 handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, depth_format, width, height, 0, depth_format, type, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    NEW_UDATA(selene_Texture2D, tex);
    tex->gl.handle = handle;
    tex->width = width;
    tex->height = height;
    tex->format = 2;
    tex->id = next_texture_id;
    next_texture_id++;
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
    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->gl.handle, 0);
    if (depth) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->gl.handle, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &handle);
        return luaL_error(L, "Framebuffer incomplete: %d", status);
    }
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
    if (opt == SELENE_SHADER_COMPUTE)
        return luaL_error(L, "compute shaders are not implemented on OpenGL yet");
    const char* source = NULL;
    if (lua_type(L, arg) == LUA_TSTRING) {
        source = luaL_checkstring(L, arg++);
    } else if (lua_isinteger(L, arg)) {
        int size = lua_tointeger(L, arg++);
        if (size > aux_source_size) {
            aux_source = realloc(aux_source, size + 1);
            aux_source_size = size + 1;
            DEBUG_LOG("alloc memory for aux_source: %d\n", size+1);
            if (!aux_source)
                return luaL_error(L, "failed to alloc memory for shader compiler auxiliar source");
        }
        void* data = lua_touserdata(L, arg++);
        memcpy(aux_source, data, size);
        aux_source[size] = '\0';
        source = aux_source;
    }
    /* Desktop GLSL version directives are invalid on OpenGL ES (and
     * crash some drivers' linkers) — fail fast with a clear error. */
    if (self->gl.caps.is_gles && source) {
        const char* ver = strstr(source, "#version");
        if (ver) {
            const char* p = ver + 8;
            while (*p == ' ' || *p == '\t') p++;
            int num = (int)strtol(p, (char**)&p, 10);
            int es_suffix = (strncmp(p, " es", 3) == 0);
            if (!(num == 100 || (num >= 300 && es_suffix)))
                return luaL_error(L,
                    "shader uses desktop GLSL '#version %d' which is invalid "
                    "on OpenGL ES; use a GLSL ES source (e.g. '#version 100')",
                    num);
        }
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
    shader->id = next_shader_id;
    next_shader_id++;
    return 1;
}

int l_GL_Renderer__destroy_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Shader, shd);
    glDeleteShader(shd->gl.handle);
    return 0;
}

static void gl_apply_scissor(selene_Renderer* r, selene_RenderTarget* target, int enabled, int x, int y, int w, int h) {
    if (!enabled) {
        glDisable(GL_SCISSOR_TEST);
        return;
    }
    int tw, th;
    renderer_get_target_size(r, target, &tw, &th);
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, th - y - h, w, h);
}

static GLenum gl_index_type(selene_GpuBuffer* index) {
    if (index && index->index_type == SELENE_INDEX_UINT16) return GL_UNSIGNED_SHORT;
    return GL_UNSIGNED_INT;
}

static void gl_draw_arrays_cmd(selene_Renderer* r, const struct RenderCommand* rc) {
    GLenum mode = gl_draw_modes_values[rc->draw.topology];
    int instances = rc->draw.instance_count < 1 ? 1 : rc->draw.instance_count;
    if (instances > 1 && r->gl.caps.has_instancing) {
#ifndef SELENE_NO_INSTANCING
        glDrawArraysInstanced(mode, rc->draw.start, rc->draw.count, instances);
#endif
    } else {
        for (int i = 0; i < instances; i++)
            glDrawArrays(mode, rc->draw.start, rc->draw.count);
    }
}

static void gl_draw_indexed_cmd(selene_Renderer* r, selene_GpuBuffer* index, const struct RenderCommand* rc) {
    GLenum mode = gl_draw_modes_values[rc->draw.topology];
    GLenum itype = gl_index_type(index);
    int instances = rc->draw.instance_count < 1 ? 1 : rc->draw.instance_count;
    const void* offset = (const void*)(uintptr_t)(rc->draw.start * (itype == GL_UNSIGNED_SHORT ? 2 : 4));
    if (instances > 1 && r->gl.caps.has_instancing) {
#ifndef SELENE_NO_INSTANCING
        glDrawElementsInstanced(mode, rc->draw.count, itype, offset, instances);
#endif
    } else {
        for (int i = 0; i < instances; i++)
            glDrawElements(mode, rc->draw.count, itype, offset);
    }
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
            case RENDER_COMMAND_BEGIN_PASS: {
                selene_RenderTarget* t = rc->pass.target;
                int width, height;
                renderer_get_target_size(self, t, &width, &height);
                glBindFramebuffer(GL_FRAMEBUFFER, t ? t->gl.fbo : 0);
                state.target = t;
                if (rc->pass.has_viewport) {
                    gl_apply_viewport(self, t,
                                      rc->pass.viewport.x, rc->pass.viewport.y,
                                      rc->pass.viewport.width, rc->pass.viewport.height);
                } else {
                    glViewport(0, 0, width, height);
                }
                if (rc->pass.has_scissor) {
                    gl_apply_scissor(self, t, rc->pass.scissor.enabled,
                                     rc->pass.scissor.x, rc->pass.scissor.y,
                                     rc->pass.scissor.width, rc->pass.scissor.height);
                } else {
                    glDisable(GL_SCISSOR_TEST);
                }
                GLbitfield mask = 0;
                if (rc->pass.clear_color) {
                    float* c = rc->pass.color;
                    glClearColor(c[0], c[1], c[2], c[3]);
                    mask |= GL_COLOR_BUFFER_BIT;
                }
                if (rc->pass.clear_depth) {
                    if (self->gl.caps.is_gles)
                        glClearDepthf(rc->pass.depth);
                    else
                        glClearDepth(rc->pass.depth);
                    mask |= GL_DEPTH_BUFFER_BIT;
                }
                if (mask) glClear(mask);
            }
                break;
            case RENDER_COMMAND_END_PASS:
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDisable(GL_SCISSOR_TEST);
                state.target = NULL;
                break;
            case RENDER_COMMAND_CLEAR_COLOR: {
                float* c = rc->clear.color;
                glClearColor(c[0], c[1], c[2], c[3]);
                glClear(GL_COLOR_BUFFER_BIT);
            }
                break;
            case RENDER_COMMAND_CLEAR_DEPTH: {
                if (self->gl.caps.is_gles)
                    glClearDepthf(rc->clear.depth);
                else
                    glClearDepth(rc->clear.depth);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
                break;
            case RENDER_COMMAND_SET_VIEWPORT:
                gl_apply_viewport(self, state.target,
                                  rc->viewport.x, rc->viewport.y,
                                  rc->viewport.width, rc->viewport.height);
                break;
            case RENDER_COMMAND_SET_PIPELINE: {
                selene_RenderPipeline* rp = rc->pipeline;
                if (rp == state.pipe) break;
                if (rp == NULL) {
                    glUseProgram(0);
#ifndef SELENE_NO_VAO
                    if (self->gl.caps.has_vao)
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
#ifndef SELENE_NO_UBO
                    if (self->gl.caps.has_ubo && state.uniform)
                        glUniformBlockBinding(rp->gl.program, 0, 0);
#endif
                }

                if (rp->blend_state.enabled != state.pipe->blend_state.enabled) {
                    if (rp->blend_state.enabled) glEnable(GL_BLEND);
                    else glDisable(GL_BLEND);
                }
                if (rp->blend_state.enabled &&
                    (rp->blend_state.mode != state.pipe->blend_state.mode ||
                     !state.pipe->blend_state.enabled))
                    gl_apply_blend_mode(self, rp->blend_state.mode);

                if (rp->depth_stencil_state.depth_enabled != state.pipe->depth_stencil_state.depth_enabled) {
                    if (rp->depth_stencil_state.depth_enabled) glEnable(GL_DEPTH_TEST);
                    else glDisable(GL_DEPTH_TEST);
                }
                if (rp->depth_stencil_state.depth_func != state.pipe->depth_stencil_state.depth_func)
                    glDepthFunc(gl_comparison_funcs[rp->depth_stencil_state.depth_func]);

                if (rp->raster_state.cull_enabled != state.pipe->raster_state.cull_enabled) {
                    if (rp->raster_state.cull_enabled) glEnable(GL_CULL_FACE);
                    else glDisable(GL_CULL_FACE);
                }
                if (rp->raster_state.cull_face != state.pipe->raster_state.cull_face)
                    glCullFace(gl_cull_face_values[rp->raster_state.cull_face]);
                if (rp->raster_state.front_face != state.pipe->raster_state.front_face)
                    glFrontFace(gl_front_face_values[rp->raster_state.front_face]);

#ifndef SELENE_NO_VAO
                if (self->gl.caps.has_vao) {
                    if (rp->gl.vao != state.pipe->gl.vao) {
                        glBindVertexArray(rp->gl.vao);
                        if (state.vertex)
                            gl_enable_layout(self, &(rp->layout));
                    }
                } else
#endif
                {
                    for (int j = 0; j < 8; j++) glDisableVertexAttribArray(j);
                    gl_enable_layout(self, &(rp->layout));
                }
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
            case RENDER_COMMAND_UPDATE_UNIFORM_BUFFER: {
                selene_GpuBuffer* b = rc->uniform.buffer;
                if (b) {
                    GLenum target = gl_buffer_target_types_values[b->type];
                    glBindBuffer(target, b->gl.handle);
                    if (rc->uniform.offset + rc->uniform.size > b->size) {
                        b->size = rc->uniform.offset + rc->uniform.size;
                        glBufferData(target, b->size, &rc->uniform.frame, GL_DYNAMIC_DRAW);
                    } else {
                        glBufferSubData(target, rc->uniform.offset, rc->uniform.size, &rc->uniform.frame);
                    }
                    glBindBuffer(target, 0);
                }
            }
                break;
            case RENDER_COMMAND_SET_UNIFORM_BUFFER: {
                selene_GpuBuffer* b = rc->buffer.ptr;
#ifndef SELENE_NO_UBO
                if (self->gl.caps.has_ubo) {
                    glBindBuffer(GL_UNIFORM_BUFFER, b ? b->gl.handle : 0);
                    glBindBufferBase(GL_UNIFORM_BUFFER, rc->buffer.slot, b ? b->gl.handle : 0);
                    if (b && state.pipe) {
                        int index = glGetUniformBlockIndex(state.pipe->gl.program, "Matrices");
                        if (index != (int)((GLuint)-1))
                            glUniformBlockBinding(state.pipe->gl.program, index, rc->buffer.slot);
                    }
                }
#endif
                state.uniform = b;
            }
                break;
            case RENDER_COMMAND_SET_TEXTURE: {
                selene_Texture2D* t = (selene_Texture2D*)rc->texture.ptr;
                int tex_target = gl_texture_targets_values[rc->texture.target];
                int unit = renderer_pipeline_binding_index(state.pipe, SELENE_BINDING_TEXTURE,
                                                           rc->texture.slot);
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(tex_target, t ? t->gl.handle : 0);
            }
                break;
            case RENDER_COMMAND_SET_SHADER:
                glUseProgram(rc->program.handle);
                break;
            case RENDER_COMMAND_SET_RENDER_TARGET: {
                selene_RenderTarget* t = rc->target.ptr;
                if (t != state.target || t == NULL) {
                    /* state resets every flush but the GL FBO binding
                     * persists, so always restore the default target */
                    int width, height;
                    renderer_get_target_size(self, t, &width, &height);
                    glBindFramebuffer(GL_FRAMEBUFFER, t ? t->gl.fbo : 0);
                    #if 0
                    glViewport(0, 0, width, height);
                    if (state.uniform) {
                    #if !defined(USE_GLES2)
                        glBindBuffer(GL_UNIFORM_BUFFER, state.uniform->gl.handle);
                    #endif
                        mat4 matrix;
                        glm_mat4_identity(matrix);
                        glm_ortho(0, (float)width, (float)height, 0, -1000, 1000, matrix);
                    #if !defined(USE_GLES2)
                        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matrix), matrix);
                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    #endif
                    }
                    #endif
                }
                state.target = t;
            }
                break;
            case RENDER_COMMAND_SET_SCISSOR:
                gl_apply_scissor(self, state.target, rc->scissor.enabled,
                                 rc->scissor.x, rc->scissor.y,
                                 rc->scissor.width, rc->scissor.height);
                break;
            case RENDER_COMMAND_FLOAT_UNIFORM:
                glUniform1f(rc->uniform.location, rc->uniform.f);
                break;
            case RENDER_COMMAND_SET_PROJECTION:
            case RENDER_COMMAND_SET_VIEW:
            case RENDER_COMMAND_MATRIX_UNIFORM:
                glUniformMatrix4fv(rc->uniform.location, 1, GL_FALSE, rc->uniform.m[0]);
                break;
            case RENDER_COMMAND_DRAW:
                gl_draw_arrays_cmd(self, rc);
                break;
            case RENDER_COMMAND_DRAW_INDEXED:
                gl_draw_indexed_cmd(self, state.index, rc);
                break;
            default:
                break;
        }
    }
    self->command_offset = 0;
    glUseProgram(0);
#ifndef SELENE_NO_VAO
    if (self->gl.caps.has_vao)
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
    if (self->command_offset > 0) l_GL_Renderer__flush(L);
    SDL_GL_SwapWindow(self->window_ptr->handle);
    return 0;
}

/**
 * Runtime capability detection — replaces platform #ifdefs so any
 * context (desktop GL or GLES, any version) is probed dynamically.
 */
static int gl_supports_extension(selene_Renderer* r, const char* name) {
    const struct selene_GLCapabilities* caps = &r->gl.caps;
#if defined(GLAD_GL_VERSION_3_0) || defined(GLAD_GL_ES_VERSION_3_0)
    if (caps->major >= 3 && glad_glGetStringi) {
        GLint num_extensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
        for (GLint i = 0; i < num_extensions; i++) {
            const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
            if (ext && strcmp(ext, name) == 0) return 1;
        }
        return 0;
    }
#endif
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (!extensions) return 0;
    size_t name_len = strlen(name);
    const char* p = extensions;
    while ((p = strstr(p, name)) != NULL) {
        if ((p == extensions || p[-1] == ' ') &&
            (p[name_len] == ' ' || p[name_len] == '\0'))
            return 1;
        p += name_len;
    }
    return 0;
}

static void gl_detect_capabilities(selene_Renderer* r) {
    struct selene_GLCapabilities* caps = &r->gl.caps;
    memset(caps, 0, sizeof(*caps));

    const char* version = (const char*)glGetString(GL_VERSION);
    if (!version) return;

    const char* p = version;
    if (strncmp(p, "OpenGL ES", 9) == 0) {
        caps->is_gles = 1;
        p += 9;
    } else if (strncmp(p, "OpenGL", 6) == 0) {
        p += 6;
    }
    while (*p == ' ') p++;
    caps->major = (int)strtol(p, (char**)&p, 10);
    if (*p == '.') {
        p++;
        caps->minor = (int)strtol(p, (char**)&p, 10);
    }

    /* VAO: core in GL 3.0+ / GLES 3.0+, extension otherwise */
    caps->has_vao = caps->major >= 3 ||
        gl_supports_extension(r, caps->is_gles ? "GL_OES_vertex_array_object"
                                                : "GL_ARB_vertex_array_object");
    /* Instancing: core in GL 3.0+ / GLES 3.0+ */
    caps->has_instancing = caps->major >= 3 ||
        gl_supports_extension(r, "GL_ANGLE_instanced_arrays") ||
        gl_supports_extension(r, "GL_EXT_draw_instanced") ||
        gl_supports_extension(r, "GL_ARB_draw_instanced");
#ifdef SELENE_NO_INSTANCING
    caps->has_instancing = 0; /* not declared in this SDK's headers */
#endif
    /* UBO: core in GL 3.1+ / GLES 3.0+ */
    caps->has_ubo = (caps->is_gles && caps->major >= 3) ||
        (!caps->is_gles && (caps->major > 3 ||
                            (caps->major == 3 && caps->minor >= 1))) ||
        gl_supports_extension(r, "GL_ARB_uniform_buffer_object");
#ifdef SELENE_NO_UBO
    caps->has_ubo = 0; /* not declared in this SDK's headers */
#endif
    /* glBlendEquation: core in GL 1.4+ / GLES 3.0+ */
    caps->has_blend_equation = !caps->is_gles || caps->major >= 3 ||
        gl_supports_extension(r, "GL_EXT_blend_subtract") ||
        gl_supports_extension(r, "GL_EXT_blend_minmax");
    /* Depth textures */
    caps->has_depth_texture = caps->major >= 3 ||
        gl_supports_extension(r, caps->is_gles ? "GL_OES_depth_texture"
                                                : "GL_ARB_depth_texture");
    /* BGRA format: desktop core / GLES extension */
    caps->has_bgra = !caps->is_gles ||
        gl_supports_extension(r, "GL_EXT_texture_format_BGRA8888");
    /* GL_UNPACK_FLIP_Y: desktop extension, not available on GLES */
    caps->has_unpack_flip_y = !caps->is_gles &&
        gl_supports_extension(r, "GL_EXT_unpack_subimage");

    DEBUG_LOG("[renderer] GL capabilities: %s %d.%d vao=%d instancing=%d "
              "ubo=%d blend_eq=%d depth_tex=%d bgra=%d flip_y=%d\n",
              caps->is_gles ? "GLES" : "GL", caps->major, caps->minor,
              caps->has_vao, caps->has_instancing, caps->has_ubo,
              caps->has_blend_equation, caps->has_depth_texture,
              caps->has_bgra, caps->has_unpack_flip_y);
}

int l_GL_Renderer_create(lua_State* L) {
#ifndef NDEBUG
    fprintf(stdout, "[renderer] creating OpenGL Renderer\n");
#endif
    INIT_ARG();
    selene_Window* win = (selene_Window*)luaL_checkudata(L, arg++, selene_Window_METANAME);
    arg++; /* backend name (consumed by l_renderer_create) */
#ifndef NDEBUG
    fprintf(stdout, "[renderer] window: %p\n", win);
    fprintf(stdout, "[renderer] creating GL Context\n");
#endif
    /* Optional context options: {es=true, major=3, minor=0, debug=true} */
    int context_major = 0, context_minor = 0;
    int profile_mask = 0, context_flags = 0;
    if (lua_istable(L, arg)) {
        if (lua_getfield(L, arg, "es") == LUA_TBOOLEAN && lua_toboolean(L, -1))
            profile_mask = SDL_GL_CONTEXT_PROFILE_ES;
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "major") == LUA_TNUMBER)
            context_major = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "minor") == LUA_TNUMBER)
            context_minor = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "debug") == LUA_TBOOLEAN && lua_toboolean(L, -1))
            context_flags = SDL_GL_CONTEXT_DEBUG_FLAG;
        lua_pop(L, 1);
    }
    if (profile_mask)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile_mask);
    if (context_major)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, context_major);
    if (context_minor)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, context_minor);
    if (context_flags)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);

    SDL_GLContext ctx = SDL_GL_CreateContext(win->handle);
    if (!ctx) {
        return luaL_error(L, "failed to create SDL OpenGL context: %s", SDL_GetError());
    }
    /* Restore defaults so the next context is not affected */
#if defined(SELENE_USE_SDL3)
    SDL_ResetAttributes();
#elif SDL_VERSION_ATLEAST(2, 24, 0)
    SDL_GL_ResetAttributes();
#endif
#ifndef NDEBUG
    fprintf(stdout, "[renderer] GL Context created\n");
#endif
    NEW_UDATA(selene_Renderer, r);
    r->window_ptr = win;
    int width, height;
    SDL_GetWindowSize(win->handle, &width, &height);
    r->gl.ctx = ctx;
    SDL_GL_MakeCurrent(win->handle, ctx);
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
        return luaL_error(L, "Failed to init glad");
#endif
#ifndef NDEBUG
    fprintf(stdout, "[renderer] modern OpenGL loaded\n");
#endif
    gl_detect_capabilities(r);

    r->width = width;
    r->height = height;
    r->in_frame = 0;
    r->in_pass = 0;
    if (!renderer_init_command_pool(r, 512))
        return luaL_error(L, "failed to alloc renderer command pool");

    lua_pushvalue(L, 1);
    r->l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    r->backend = SELENE_RENDERER_OPENGL;

    r->destroy = l_GL_Renderer__destroy;
    r->begin_frame = NULL;
    r->end_frame = NULL;
    r->begin_pass = NULL;
    r->end_pass = NULL;

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

    r->flush = l_GL_Renderer__flush;
    r->present = l_GL_Renderer__present;

    return 1;
}
#endif
