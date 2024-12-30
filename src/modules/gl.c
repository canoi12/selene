
#include "selene.h"
#include "lua_helper.h"

BEGIN_ENUM(gl_Enums)
    ENUM_FIELD(POINTS, GL_),
    ENUM_FIELD(LINES, GL_),
    ENUM_FIELD(LINE_LOOP, GL_),
    ENUM_FIELD(LINE_STRIP, GL_),
    ENUM_FIELD(TRIANGLES, GL_),
    ENUM_FIELD(TRIANGLE_STRIP, GL_),
    ENUM_FIELD(TRIANGLE_FAN, GL_),
    ENUM_FIELD(NEVER, GL_),
    ENUM_FIELD(LESS, GL_),
    ENUM_FIELD(EQUAL, GL_),
    ENUM_FIELD(LEQUAL, GL_),
    ENUM_FIELD(GREATER, GL_),
    ENUM_FIELD(NOTEQUAL, GL_),
    ENUM_FIELD(GEQUAL, GL_),
    ENUM_FIELD(ALWAYS, GL_),
    ENUM_FIELD(ZERO, GL_),
    ENUM_FIELD(ONE, GL_),
    ENUM_FIELD(SRC_COLOR, GL_),
    ENUM_FIELD(ONE_MINUS_SRC_COLOR, GL_),
    ENUM_FIELD(SRC_ALPHA, GL_),
    ENUM_FIELD(ONE_MINUS_SRC_ALPHA, GL_),
    ENUM_FIELD(DST_ALPHA, GL_),
    ENUM_FIELD(ONE_MINUS_DST_ALPHA, GL_),
    ENUM_FIELD(DST_COLOR, GL_),
    ENUM_FIELD(ONE_MINUS_DST_COLOR, GL_),
    ENUM_FIELD(SRC_ALPHA_SATURATE, GL_),
    ENUM_FIELD(NONE, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(FRONT_LEFT, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(FRONT_RIGHT, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(BACK_LEFT, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(BACK_RIGHT, GL_),
#endif
    ENUM_FIELD(FRONT, GL_),
    ENUM_FIELD(BACK, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(LEFT, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(RIGHT, GL_),
#endif
    ENUM_FIELD(FRONT_AND_BACK, GL_),
    ENUM_FIELD(NO_ERROR, GL_),
    ENUM_FIELD(INVALID_ENUM, GL_),
    ENUM_FIELD(INVALID_VALUE, GL_),
    ENUM_FIELD(INVALID_OPERATION, GL_),
    ENUM_FIELD(OUT_OF_MEMORY, GL_),
    ENUM_FIELD(CW, GL_),
    ENUM_FIELD(CCW, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(POINT_SIZE, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(POINT_SIZE_RANGE, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(POINT_SIZE_GRANULARITY, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(LINE_SMOOTH, GL_),
#endif
    ENUM_FIELD(LINE_WIDTH, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(LINE_WIDTH_RANGE, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(LINE_WIDTH_GRANULARITY, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(POLYGON_MODE, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(POLYGON_SMOOTH, GL_),
#endif
    ENUM_FIELD(CULL_FACE, GL_),
    ENUM_FIELD(CULL_FACE_MODE, GL_),
    ENUM_FIELD(FRONT_FACE, GL_),
    ENUM_FIELD(DEPTH_RANGE, GL_),
    ENUM_FIELD(DEPTH_TEST, GL_),
    ENUM_FIELD(DEPTH_WRITEMASK, GL_),
    ENUM_FIELD(DEPTH_CLEAR_VALUE, GL_),
    ENUM_FIELD(DEPTH_FUNC, GL_),
    ENUM_FIELD(STENCIL_TEST, GL_),
    ENUM_FIELD(STENCIL_CLEAR_VALUE, GL_),
    ENUM_FIELD(STENCIL_FUNC, GL_),
    ENUM_FIELD(STENCIL_VALUE_MASK, GL_),
    ENUM_FIELD(STENCIL_FAIL, GL_),
    ENUM_FIELD(STENCIL_PASS_DEPTH_FAIL, GL_),
    ENUM_FIELD(STENCIL_PASS_DEPTH_PASS, GL_),
    ENUM_FIELD(STENCIL_REF, GL_),
    ENUM_FIELD(STENCIL_WRITEMASK, GL_),
    ENUM_FIELD(VIEWPORT, GL_),
    ENUM_FIELD(DITHER, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(BLEND_DST, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(BLEND_SRC, GL_),
#endif
    ENUM_FIELD(BLEND, GL_),
    ENUM_FIELD(SHADING_LANGUAGE_VERSION, GL_),
    ENUM_FIELD(DEPTH_BUFFER_BIT, GL_),
    ENUM_FIELD(STENCIL_BUFFER_BIT, GL_),
    ENUM_FIELD(COLOR_BUFFER_BIT, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(TEXTURE_1D, GL_),
#endif
    ENUM_FIELD(TEXTURE_2D, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(TEXTURE_WIDTH, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(TEXTURE_HEIGHT, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(TEXTURE_BORDER_COLOR, GL_),
#endif
    ENUM_FIELD(DONT_CARE, GL_),
    ENUM_FIELD(FASTEST, GL_),
    ENUM_FIELD(NICEST, GL_),
    ENUM_FIELD(BYTE, GL_),
    ENUM_FIELD(UNSIGNED_BYTE, GL_),
    ENUM_FIELD(SHORT, GL_),
    ENUM_FIELD(UNSIGNED_SHORT, GL_),
    ENUM_FIELD(INT, GL_),
    ENUM_FIELD(UNSIGNED_INT, GL_),
    ENUM_FIELD(FLOAT, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(CLEAR, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(AND, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(AND_REVERSE, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(COPY, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(AND_INVERTED, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(NOOP, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(XOR, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(OR, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(NOR, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(EQUIV, GL_),
#endif
    ENUM_FIELD(INVERT, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(OR_REVERSE, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(COPY_INVERTED, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(OR_INVERTED, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(NAND, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(SET, GL_),
#endif
    ENUM_FIELD(TEXTURE, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(COLOR, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(DEPTH, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(STENCIL, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(STENCIL_INDEX, GL_),
#endif
    ENUM_FIELD(DEPTH_COMPONENT, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(RED, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(GREEN, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(BLUE, GL_),
#endif
    ENUM_FIELD(ALPHA, GL_),
    ENUM_FIELD(RGB, GL_),
    ENUM_FIELD(RGBA, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(POINT, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(LINE, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(FILL, GL_),
#endif
    ENUM_FIELD(KEEP, GL_),
    ENUM_FIELD(REPLACE, GL_),
    ENUM_FIELD(INCR, GL_),
    ENUM_FIELD(DECR, GL_),
    ENUM_FIELD(VENDOR, GL_),
    ENUM_FIELD(RENDERER, GL_),
    ENUM_FIELD(VERSION, GL_),
    ENUM_FIELD(EXTENSIONS, GL_),
    ENUM_FIELD(NEAREST, GL_),
    ENUM_FIELD(LINEAR, GL_),
    ENUM_FIELD(NEAREST_MIPMAP_NEAREST, GL_),
    ENUM_FIELD(LINEAR_MIPMAP_NEAREST, GL_),
    ENUM_FIELD(NEAREST_MIPMAP_LINEAR, GL_),
    ENUM_FIELD(LINEAR_MIPMAP_LINEAR, GL_),
    ENUM_FIELD(TEXTURE_MAG_FILTER, GL_),
    ENUM_FIELD(TEXTURE_MIN_FILTER, GL_),
    ENUM_FIELD(TEXTURE_WRAP_S, GL_),
    ENUM_FIELD(TEXTURE_WRAP_T, GL_),
    ENUM_FIELD(REPEAT, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(BGR, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(BGRA, GL_),
#endif
    ENUM_FIELD(CLAMP_TO_EDGE, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(CLAMP_TO_BORDER, GL_),
#endif
    ENUM_FIELD(BLEND_DST_RGB, GL_),
    ENUM_FIELD(BLEND_SRC_RGB, GL_),
    ENUM_FIELD(BLEND_DST_ALPHA, GL_),
    ENUM_FIELD(BLEND_SRC_ALPHA, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(POINT_FADE_THRESHOLD_SIZE, GL_),
#endif
    ENUM_FIELD(DEPTH_COMPONENT16, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(DEPTH_COMPONENT24, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(DEPTH_COMPONENT32, GL_),
#endif
    ENUM_FIELD(MIRRORED_REPEAT, GL_),
    ENUM_FIELD(BLEND_COLOR, GL_),
    ENUM_FIELD(BLEND_EQUATION, GL_),
    ENUM_FIELD(CONSTANT_COLOR, GL_),
    ENUM_FIELD(ONE_MINUS_CONSTANT_COLOR, GL_),
    ENUM_FIELD(CONSTANT_ALPHA, GL_),
    ENUM_FIELD(ONE_MINUS_CONSTANT_ALPHA, GL_),
    ENUM_FIELD(FUNC_ADD, GL_),
    ENUM_FIELD(FUNC_REVERSE_SUBTRACT, GL_),
    ENUM_FIELD(FUNC_SUBTRACT, GL_),
    ENUM_FIELD(MIN, GL_),
    ENUM_FIELD(MAX, GL_),
    ENUM_FIELD(ARRAY_BUFFER, GL_),
    ENUM_FIELD(ELEMENT_ARRAY_BUFFER, GL_),
    ENUM_FIELD(ARRAY_BUFFER_BINDING, GL_),
    ENUM_FIELD(ELEMENT_ARRAY_BUFFER_BINDING, GL_),
    ENUM_FIELD(STREAM_DRAW, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(STREAM_READ, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(STREAM_COPY, GL_),
#endif
    ENUM_FIELD(STATIC_DRAW, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(STATIC_READ, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(STATIC_COPY, GL_),
#endif
    ENUM_FIELD(DYNAMIC_DRAW, GL_),
#if !defined(USE_GLES2)
    ENUM_FIELD(DYNAMIC_READ, GL_),
#endif
#if !defined(USE_GLES2)
    ENUM_FIELD(DYNAMIC_COPY, GL_),
#endif
    ENUM_FIELD(FRAGMENT_SHADER, GL_),
    ENUM_FIELD(VERTEX_SHADER, GL_),
    ENUM_FIELD(COMPILE_STATUS, GL_),
    ENUM_FIELD(LINK_STATUS, GL_),
END_ENUM()
/************************
 #                      #
 #    Vertex Arrays     #
 #                      #
 ************************/

static MODULE_FUNCTION(gl, gen_vertex_arrays) {
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        glGenVertexArrays(1, &t);
        lua_pushinteger(L, t);
    }
    return n;
#else
    return 0;
#endif
}

static MODULE_FUNCTION(gl, delete_vertex_arrays) {
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(vao);
        glDeleteVertexArrays(1, &vao);
    }
#endif
    return 0;
}

static MODULE_FUNCTION(gl, enable_vertex_attrib_array) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glEnableVertexAttribArray(attrib);
    return 0;
}

static MODULE_FUNCTION(gl, disable_vertex_attrib_array) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glDisableVertexAttribArray(attrib);
    return 0;
}

static MODULE_FUNCTION(gl, vertex_attrib_pointer) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    CHECK_INTEGER(size);
    CHECK_INTEGER(type);
    GET_BOOLEAN(normalized);
    CHECK_INTEGER(stride);
    CHECK_INTEGER(offset);
    glVertexAttribPointer(attrib, size, type, normalized, stride, (void*)offset);
    return 0;
}

static MODULE_FUNCTION(gl, bind_vertex_array) {
    INIT_ARG();
    OPT_INTEGER(vao, 0);
    #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    glBindVertexArray(vao);
#endif
    return 0;
}

/************************
 #                      #
 #        Shader        #
 #                      #
 ************************/

static MODULE_FUNCTION(gl, create_shader) {
    INIT_ARG();
    CHECK_INTEGER(shader_type);
    PUSH_INTEGER(glCreateShader(shader_type));
    return 1;
}

static MODULE_FUNCTION(gl, delete_shader) {
    INIT_ARG();
    CHECK_INTEGER(shader);
    glDeleteShader(shader);
    return 0;
}

static MODULE_FUNCTION(gl, shader_source) {
    INIT_ARG();
    CHECK_INTEGER(shader);
    CHECK_STRING(source);
    glShaderSource(shader, 1, &source, NULL);
    return 0;
}

static MODULE_FUNCTION(gl, compile_shader) {
    INIT_ARG();
    CHECK_INTEGER(shader);
    glCompileShader(shader);
    return 0;
}

static MODULE_FUNCTION(gl, get_shaderiv) {
    INIT_ARG();
    CHECK_INTEGER(shader);
    CHECK_INTEGER(pname);
    int out;
    glGetShaderiv(shader, pname, &out);
    PUSH_INTEGER(out);
    return 1;
}

static MODULE_FUNCTION(gl, get_shader_info_log) {
    INIT_ARG();
    CHECK_INTEGER(shader);
        char log[1024];
    glGetShaderInfoLog(shader, 1024, NULL, log);
    PUSH_STRING(log);

    return 1;
}

static MODULE_FUNCTION(gl, uniform1f) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_NUMBER(float, v);
    glUniform1f(loc, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform2f) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_NUMBER(float, v0);
    CHECK_NUMBER(float, v1);
    glUniform2f(loc, v0, v1);
    return 0;
}

static MODULE_FUNCTION(gl, uniform3f) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_NUMBER(float, v0);
    CHECK_NUMBER(float, v1);
    CHECK_NUMBER(float, v2);
    glUniform3f(loc, v0, v1, v2);
    return 0;
}

static MODULE_FUNCTION(gl, uniform4f) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_NUMBER(float, v0);
    CHECK_NUMBER(float, v1);
    CHECK_NUMBER(float, v2);
    CHECK_NUMBER(float, v3);
    glUniform4f(loc, v0, v1, v2, v3);
    return 0;
}

static MODULE_FUNCTION(gl, uniform1fv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform1fv(loc, n, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform2fv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform2fv(loc, n, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform3fv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform3fv(loc, n, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform4fv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform4fv(loc, n, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform1i) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(v);
    glUniform1i(loc, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform2i) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(v0);
    CHECK_INTEGER(v1);
    glUniform2i(loc, v0, v1);
    return 0;
}

static MODULE_FUNCTION(gl, uniform3i) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(v0);
    CHECK_INTEGER(v1);
    CHECK_INTEGER(v2);
    glUniform3i(loc, v0, v1, v2);
    return 0;
}

static MODULE_FUNCTION(gl, uniform4i) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(v0);
    CHECK_INTEGER(v1);
    CHECK_INTEGER(v2);
    CHECK_INTEGER(v3);
    glUniform4i(loc, v0, v1, v2, v3);
    return 0;
}

static MODULE_FUNCTION(gl, uniform1iv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform1iv(loc, n, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform2iv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform2iv(loc, n, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform3iv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform3iv(loc, n, v);
    return 0;
}

static MODULE_FUNCTION(gl, uniform4iv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(void, v);
    glUniform4iv(loc, n, v);
    return 0;
}

/************************
 #                      #
 #       Program        #
 #                      #
 ************************/

static MODULE_FUNCTION(gl, create_program) {
    INIT_ARG();
    PUSH_INTEGER(glCreateProgram());
    return 1;
}

static MODULE_FUNCTION(gl, delete_program) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    glDeleteProgram(prog);
    return 0;
}

static MODULE_FUNCTION(gl, use_program) {
    INIT_ARG();
    OPT_INTEGER(prog, 0);
    glUseProgram(prog);
    return 0;
}

static MODULE_FUNCTION(gl, attach_shader) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_INTEGER(shader);
    glAttachShader(prog, shader);
    return 0;
}

static MODULE_FUNCTION(gl, link_program) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    glLinkProgram(prog);
    return 0;
}

static MODULE_FUNCTION(gl, get_programiv) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_INTEGER(pname);
        int out;
    glGetProgramiv(prog, pname, &out);
    PUSH_INTEGER(out);

    return 1;
}

static MODULE_FUNCTION(gl, get_program_info_log) {
    INIT_ARG();
    CHECK_INTEGER(prog);
        char log[1024];
    glGetProgramInfoLog(prog, 1024, NULL, log);
    PUSH_STRING(log);

    return 1;
}

static MODULE_FUNCTION(gl, get_uniform_location) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_STRING(name);
    PUSH_INTEGER(glGetUniformLocation(prog, name));
    return 1;
}

static MODULE_FUNCTION(gl, get_attrib_location) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_STRING(name);
    PUSH_INTEGER(glGetAttribLocation(prog, name));
    return 1;
}

    float aux[512];

static MODULE_FUNCTION(gl, uniform_matrix4fv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(count);
    CHECK_BOOLEAN(transpose);
    int args = lua_gettop(L);
    float* m = NULL;
    int type = lua_type(L, arg);
    if (type == LUA_TLIGHTUSERDATA || type == LUA_TUSERDATA) {
        m = lua_touserdata(L, arg);
    } else {
        for (int i = 0; i < count * 16; i++) {
            aux[i] = luaL_checknumber(L, arg + i);
        }
        m = aux;
    }
    glUniformMatrix4fv(loc, count, transpose, m);
    return 0;
}

/************************
 #                      #
 #    Draw Functions    #
 #                      #
 ************************/

static MODULE_FUNCTION(gl, clear) {
    INIT_ARG();
    CHECK_INTEGER(mask);
    glClear(mask);
    return 0;
}

static MODULE_FUNCTION(gl, clear_color) {
    INIT_ARG();
    CHECK_NUMBER(float, r);
    CHECK_NUMBER(float, g);
    CHECK_NUMBER(float, b);
    CHECK_NUMBER(float, a);
    glClearColor(r, g, b, a);
    return 0;
}

static MODULE_FUNCTION(gl, enable) {
    INIT_ARG();
    CHECK_INTEGER(val);
    glEnable(val);
    return 0;
}

static MODULE_FUNCTION(gl, disable) {
    INIT_ARG();
    CHECK_INTEGER(val);
    glDisable(val);
    return 0;
}

static MODULE_FUNCTION(gl, blend_func) {
    INIT_ARG();
    CHECK_INTEGER(sfactor);
    CHECK_INTEGER(dfactor);
    glBlendFunc(sfactor, dfactor);
    return 0;
}

static MODULE_FUNCTION(gl, blend_equation) {
    INIT_ARG();
    CHECK_INTEGER(eq);
    glBlendEquation(eq);
    return 0;
}

static MODULE_FUNCTION(gl, draw_arrays) {
    INIT_ARG();
    CHECK_INTEGER(mode);
    CHECK_INTEGER(start);
    CHECK_INTEGER(count);
    glDrawArrays(mode, start, count);
    return 0;
}

static MODULE_FUNCTION(gl, draw_elements) {
    INIT_ARG();
    CHECK_INTEGER(mode);
    CHECK_INTEGER(start);
    CHECK_INTEGER(type);
    CHECK_INTEGER(indices);
    glDrawElements(mode, start, type, (void*)indices);
    return 0;
}

/************************
 #                      #
 #     Framebuffers     #
 #                      #
 ************************/

static MODULE_FUNCTION(gl, gen_framebuffers) {
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        glGenFramebuffers(1, &t);
        PUSH_INTEGER(t);
    }
    return n;
}

static MODULE_FUNCTION(gl, delete_framebuffers) {
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        glDeleteFramebuffers(1, &b);
    }
    return 0;
}

static MODULE_FUNCTION(gl, bind_framebuffer) {
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(fbo, 0);
    glBindFramebuffer(target, fbo);
    return 0;
}

static MODULE_FUNCTION(gl, framebuffer_texture2d) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(attachment);
    CHECK_INTEGER(textarget);
    CHECK_INTEGER(texture);
    CHECK_INTEGER(level);
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
    return 0;
}

/************************
 #                      #
 #        Buffer        #
 #                      #
 ************************/

static MODULE_FUNCTION(gl, gen_buffers) {
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        glGenBuffers(1, &t);
        PUSH_INTEGER(t);
    }
    return n;}

static MODULE_FUNCTION(gl, delete_buffers) {
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        glDeleteBuffers(1, &b);
    }
    return 0;
}

static MODULE_FUNCTION(gl, bind_buffer) {
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(buf, 0);
    glBindBuffer(target, buf);
    return 0;
}

static MODULE_FUNCTION(gl, buffer_data) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(size);
    const char* data = lua_touserdata(L, arg++);
    CHECK_INTEGER(usage);
    glBufferData(target, size, data, usage);
    return 0;
}

static MODULE_FUNCTION(gl, buffer_sub_data) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(start);
    CHECK_INTEGER(size);
    CHECK_LUDATA(const void*, data);
    glBufferSubData(target, start, size, data);
    return 0;
}

/************************
 #                      #
 #       Texture        #
 #                      #
 ************************/

static MODULE_FUNCTION(gl, gen_textures) {
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        glGenTextures(1, &t);
        PUSH_INTEGER(t);
    }
    return n;
}

static MODULE_FUNCTION(gl, delete_textures) {
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        glDeleteTextures(1, &b);
    }
    return 0;
}

static MODULE_FUNCTION(gl, bind_texture) {
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(tex, 0);
    glBindTexture(target, tex);
    return 0;
}

static MODULE_FUNCTION(gl, tex_parameteri) {
    INIT_ARG();
    CHECK_INTEGER(tex);
    CHECK_INTEGER(pname);
    CHECK_INTEGER(param);
    glTexParameteri(tex, pname, param);
    return 0;
}

static MODULE_FUNCTION(gl, tex_parameterf) {
    INIT_ARG();
    CHECK_INTEGER(tex);
    CHECK_INTEGER(pname);
    CHECK_NUMBER(float, param);
    glTexParameterf(tex, pname, param);
    return 0;
}

static MODULE_FUNCTION(gl, tex_image2d) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(level);
    CHECK_INTEGER(internal);
    CHECK_INTEGER(w);
    CHECK_INTEGER(h);
    CHECK_INTEGER(border);
    CHECK_INTEGER(format);
    CHECK_INTEGER(type);
    const void* data = lua_touserdata(L, arg);
    glTexImage2D(target, level, internal, w, h, border, format, type, data);
    return 0;
}

static MODULE_FUNCTION(gl, setup) {
#ifndef USE_GLES2
    INIT_ARG();
    if (lua_type(L, arg) == LUA_TLIGHTUSERDATA) {
        gladLoadGLLoader(lua_touserdata(L, arg));
        return 0;
    } else gladLoadGL();
#endif
    return 0;
}

static MODULE_FUNCTION(gl, get_string) {
    INIT_ARG();
    CHECK_INTEGER(pname);
    PUSH_STRING((const char*)glGetString(pname));
    return 1;
}

int luaopen_gl(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(gl, gen_vertex_arrays),
        REG_FIELD(gl, delete_vertex_arrays),
        REG_FIELD(gl, enable_vertex_attrib_array),
        REG_FIELD(gl, disable_vertex_attrib_array),
        REG_FIELD(gl, vertex_attrib_pointer),
        REG_FIELD(gl, bind_vertex_array),
        REG_FIELD(gl, create_shader),
        REG_FIELD(gl, delete_shader),
        REG_FIELD(gl, shader_source),
        REG_FIELD(gl, compile_shader),
        REG_FIELD(gl, get_shaderiv),
        REG_FIELD(gl, get_shader_info_log),
        REG_FIELD(gl, uniform1f),
        REG_FIELD(gl, uniform2f),
        REG_FIELD(gl, uniform3f),
        REG_FIELD(gl, uniform4f),
        REG_FIELD(gl, uniform1fv),
        REG_FIELD(gl, uniform2fv),
        REG_FIELD(gl, uniform3fv),
        REG_FIELD(gl, uniform4fv),
        REG_FIELD(gl, uniform1i),
        REG_FIELD(gl, uniform2i),
        REG_FIELD(gl, uniform3i),
        REG_FIELD(gl, uniform4i),
        REG_FIELD(gl, uniform1iv),
        REG_FIELD(gl, uniform2iv),
        REG_FIELD(gl, uniform3iv),
        REG_FIELD(gl, uniform4iv),
        REG_FIELD(gl, create_program),
        REG_FIELD(gl, delete_program),
        REG_FIELD(gl, use_program),
        REG_FIELD(gl, attach_shader),
        REG_FIELD(gl, link_program),
        REG_FIELD(gl, get_programiv),
        REG_FIELD(gl, get_program_info_log),
        REG_FIELD(gl, get_uniform_location),
        REG_FIELD(gl, get_attrib_location),
        REG_FIELD(gl, uniform_matrix4fv),
        REG_FIELD(gl, clear),
        REG_FIELD(gl, clear_color),
        REG_FIELD(gl, enable),
        REG_FIELD(gl, disable),
        REG_FIELD(gl, blend_func),
        REG_FIELD(gl, blend_equation),
        REG_FIELD(gl, draw_arrays),
        REG_FIELD(gl, draw_elements),
        REG_FIELD(gl, gen_framebuffers),
        REG_FIELD(gl, delete_framebuffers),
        REG_FIELD(gl, bind_framebuffer),
        REG_FIELD(gl, framebuffer_texture2d),
        REG_FIELD(gl, gen_buffers),
        REG_FIELD(gl, delete_buffers),
        REG_FIELD(gl, bind_buffer),
        REG_FIELD(gl, buffer_data),
        REG_FIELD(gl, buffer_sub_data),
        REG_FIELD(gl, gen_textures),
        REG_FIELD(gl, delete_textures),
        REG_FIELD(gl, bind_texture),
        REG_FIELD(gl, tex_parameteri),
        REG_FIELD(gl, tex_parameterf),
        REG_FIELD(gl, tex_image2d),
        REG_FIELD(gl, setup),
        REG_FIELD(gl, get_string),
    END_REG()
    luaL_newlib(L, reg);
    LOAD_ENUM(gl_Enums);
    return 1;
}
