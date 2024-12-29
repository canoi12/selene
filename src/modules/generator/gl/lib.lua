local sources = {}

--- @type generator.Group
sources['Core'] = {
    function_block('', 'setup', 'lightuserdata|nil proc_func',
    {
        cdef =
[[
    INIT_ARG();
    if (lua_type(L, arg) == LUA_TLIGHTUSERDATA) {
        gladLoadGLLoader(lua_touserdata(L, arg));
        return 0;
    } else gladLoadGL();
    return 0;
]]
    }),
    function_block('string', 'get_string', 'integer pname', { ccall = "PUSH_STRING((const char*)glGetString(pname));" }),
}

--- @type generator.Group
sources['Draw Functions'] = {
    c_code_block(
[[
/************************
 #                      #
 #    Draw Functions    #
 #                      #
 ************************/
]]
    ),
    function_block('', 'clear', 'integer mask', { ccall = 'glClear(mask);' }),
    function_block('', 'clear_color', 'number r, number g, number b, number a', { ccall = 'glClearColor(r, g, b, a);' }),
    function_block('', 'enable', 'integer val', { ccall = 'glEnable(val);' }),
    function_block('', 'disable', 'integer val', { ccall = 'glDisable(val);' }),
    function_block('', 'blend_func', 'integer sfactor, integer dfactor', { ccall = 'glBlendFunc(sfactor, dfactor);' }),
    function_block('', 'blend_equation', 'integer eq', { ccall = 'glBlendEquation(eq);' }),
    function_block('', 'draw_arrays', 'integer mode, integer start, integer count', { ccall = 'glDrawArrays(mode, start, count);' }),
    function_block('', 'draw_elements', 'integer mode, integer start, integer type, integer indices', { ccall = 'glDrawElements(mode, start, type, indices);' }),
}

--- @type generator.Group
sources['Vertex Arrays'] = {
    c_code_block(
[[
/************************
 #                      #
 #    Vertex Arrays     #
 #                      #
 ************************/
]]
    ),
    function_block('...integer', 'gen_vertex_arrays', 'integer n',
    {
        cdef =
[[
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
]]
    }),
    function_block('', 'delete_vertex_arrays', '...integer vaos',
    {
        cdef =
[[
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(vao);
        glDeleteVertexArrays(1, &vao);
    }
#endif
    return 0;
]]
    }),
    function_block('', 'enable_vertex_attrib_array', 'integer attrib', { ccall = 'glEnableVertexAttribArray(attrib);' }),
    function_block('', 'disable_vertex_attrib_array', 'integer attrib', { ccall = 'glDisableVertexAttribArray(attrib);' }),
    function_block('', 'vertex_attrib_pointer', 'integer attrib, integer size, integer type, boolean normalized, integer stride, integer offset', { ccall = 'glVertexAttribPointer(attrib, size, type, normalized, stride, (void*)offset);' }),
    function_block('', 'bind_vertex_array', 'integer|nil vao',
    {
        ccall =
[[
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    glBindVertexArray(vao);
#endif]]
    }),
}

--- @type generator.Group
sources['Buffer'] = {
    c_code_block(
[[
/************************
 #                      #
 #        Buffer        #
 #                      #
 ************************/
]]
    ),
    function_block('...integer', 'gen_buffers', 'integer n',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        glGenBuffers(1, &t);
        PUSH_INTEGER(t);
    }
    return n;]]
    }),
    function_block('', 'delete_buffers', '...integer buffers',
    {
        cdef =
[[
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        glDeleteBuffers(1, &b);
    }
    return 0;
]]
    }),
    function_block('', 'bind_buffer', 'integer target, integer|nil buf', { ccall = "glBindBuffer(target, buf);" }),
    function_block('', 'buffer_data', 'integer target, integer size, lightuserdata data, integer usage',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(size);
    const char* data = lua_touserdata(L, arg++);
    CHECK_INTEGER(usage);
    glBufferData(target, size, data, usage);
    return 0;
]]
    }),
    function_block('', 'buffer_sub_data', 'integer target, integer start, integer size, lightuserdata data',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(start);
    CHECK_INTEGER(size);
    CHECK_LUDATA(const void*, data);
    glBufferSubData(target, start, size, data);
    return 0;
]]
    }),
}

--- @type generator.Group
sources['Texture'] = {
    c_code_block(
[[
/************************
 #                      #
 #       Texture        #
 #                      #
 ************************/
]]
    ),
    function_block('...integer', 'gen_textures', 'integer n',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        glGenTextures(1, &t);
        PUSH_INTEGER(t);
    }
    return n;
]]
    }),
    function_block('', 'delete_textures', '...integer textures',
    {
        cdef =
[[
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        glDeleteTextures(1, &b);
    }
    return 0;
]]
    }),
    function_block('', 'bind_texture', 'integer target, integer|nil tex', { ccall = "glBindTexture(target, tex);" }),
    function_block('', 'tex_parameteri', 'integer tex, integer pname, integer param', { ccall = "glTexParameteri(tex, pname, param);" }),
    function_block('', 'tex_parameterf', 'integer tex, integer pname, number param', { ccall = "glTexParameterf(tex, pname, param);" }),
    function_block('', 'tex_image2d', 'integer target, integer level, integer internal, integer width, integer height, integer border, integer format, integer type, lightuserdata data',
    {
        cdef =
[[
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
]]
    }),
}

--- @type generator.Group
sources['Framebuffers'] = {
    c_code_block(
[[
/************************
 #                      #
 #     Framebuffers     #
 #                      #
 ************************/
]]
    ),
    function_block('...integer', 'gen_framebuffers', 'integer n',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        glGenFramebuffers(1, &t);
        PUSH_INTEGER(t);
    }
    return n;
]]
    }),
    function_block('', 'delete_framebuffers', '...integer framebuffers',
    {
        cdef =
[[
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        glDeleteFramebuffers(1, &b);
    }
    return 0;
]]
    }),
    function_block('', 'bind_framebuffer', 'integer target, integer|nil fbo', { ccall = "glBindFramebuffer(target, fbo);" }),
    function_block('', 'framebuffer_texture2d', 'integer target, integer attachment, integer textarget, integer texture, integer level', { ccall = "glFramebufferTexture2D(target, attachment, textarget, texture, level);" }),
}

sources['Program'] = {
    c_code_block(
[[
/************************
 #                      #
 #       Program        #
 #                      #
 ************************/
]]
    ),
    function_block('integer', 'create_program', '', { ccall = 'PUSH_INTEGER(glCreateProgram());'}),
    function_block('', 'delete_program', 'integer prog', { ccall = 'glDeleteProgram(prog);'}),
    function_block('', 'use_program', 'integer|nil prog', { ccall = 'glUseProgram(prog);' }),
    function_block('', 'attach_shader', 'integer prog, integer shader', { ccall = 'glAttachShader(prog, shader);' }),
    function_block('', 'link_program', 'integer prog', { ccall = 'glLinkProgram(prog);' }),
    function_block('integer', 'get_programiv', 'integer prog, integer pname',
    {
        ccall =
[[
    int out;
    glGetProgramiv(prog, pname, &out);
    PUSH_INTEGER(out);
]]
    }),
    function_block('string', 'get_program_info_log', 'integer prog',
    {
        ccall =
[[
    char log[1024];
    glGetProgramInfoLog(prog, 1024, NULL, log);
    PUSH_STRING(log);
]]
    }),
    
    function_block('integer', 'get_uniform_location', 'integer prog, string name',
    {
        ccall = "PUSH_INTEGER(glGetUniformLocation(prog, name));"
    }),
    function_block('integer', 'get_attrib_location', 'integer prog, string name',
    {
        ccall = "PUSH_INTEGER(glGetAttribLocation(prog, name));"
    }),
    c_code_block(
[[
    float aux[512];
]]
    ),
    function_block('', 'uniform_matrix4fv', 'integer loc, integer count, boolean transpose, lightuserdata v',
    {
        cdef =
[[
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
]]
    })
}

sources['Shader'] = {
    c_code_block(
[[
/************************
 #                      #
 #        Shader        #
 #                      #
 ************************/
]]
    ),
    function_block('integer', 'create_shader', 'integer shader_type', { ccall = 'PUSH_INTEGER(glCreateShader(shader_type));'}),
    function_block('', 'delete_shader', 'integer shader', { ccall = 'glDeleteShader(shader);'}),
    function_block('', 'shader_source', 'integer shader, string source', { ccall = 'glShaderSource(shader, 1, &source, NULL);' }),
    function_block('', 'compile_shader', 'integer shader', { ccall = 'glCompileShader(shader);' }),
    function_block('integer', 'get_shaderiv', 'integer shader, integer pname',
    {
        ccall =
[[int out;
    glGetShaderiv(shader, pname, &out);
    PUSH_INTEGER(out);]]
    }),
    function_block('string', 'get_shader_info_log', 'integer shader',
    {
        ccall =
[[
    char log[1024];
    glGetShaderInfoLog(shader, 1024, NULL, log);
    PUSH_STRING(log);
]]
    }),
    function_block('', 'uniform1f', 'integer loc, number v', { ccall = "glUniform1f(loc, v);" }),
    function_block('', 'uniform2f', 'integer loc, number v0, number v1', { ccall = "glUniform2f(loc, v0, v1);" }),
    function_block('', 'uniform3f', 'integer loc, number v0, number v1, number v2', { ccall = "glUniform3f(loc, v0, v1, v2);" }),
    function_block('', 'uniform4f', 'integer loc, number v0, number v1, number v2, number v3', { ccall = "glUniform4f(loc, v0, v1, v2, v3);" }),
    function_block('', 'uniform1fv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform1fv(loc, n, v);" }),
    function_block('', 'uniform2fv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform2fv(loc, n, v);" }),
    function_block('', 'uniform3fv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform3fv(loc, n, v);" }),
    function_block('', 'uniform4fv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform4fv(loc, n, v);" }),

    function_block('', 'uniform1i', 'integer loc, integer v', { ccall = "glUniform1i(loc, v);" }),
    function_block('', 'uniform2i', 'integer loc, integer v0, integer v1', { ccall = "glUniform2i(loc, v0, v1);" }),
    function_block('', 'uniform3i', 'integer loc, integer v0, integer v1, integer v2', { ccall = "glUniform3i(loc, v0, v1, v2);" }),
    function_block('', 'uniform4i', 'integer loc, integer v0, integer v1, integer v2, integer v3', { ccall = "glUniform4i(loc, v0, v1, v2, v3);" }),
    function_block('', 'uniform1iv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform1iv(loc, n, v);" }),
    function_block('', 'uniform2iv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform2iv(loc, n, v);" }),
    function_block('', 'uniform3iv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform3iv(loc, n, v);" }),
    function_block('', 'uniform4iv', 'integer loc, integer n, lightuserdata v', { ccall = "glUniform4iv(loc, n, v);" }),
}

local lib = Lib.create('gl')
for name,value in pairs(sources) do
    lib:add_source(value)
end
lib.enums = require('enums')
lib.enum_prefix = 'GL_'
lib.header_include =
[[

#include "selene.h"
#include "lua_helper.h"

]]

return lib