local functions = {
    {
        name = 'get_string',
        returns = {'string'},
        args = func_args('integer pname'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(pname);
    PUSH_STRING((const char*)glGetString(pname));
    return 1;
]]
    },
    {
        name = 'clear',
        rettype = 'void',
        args = func_args('integer mask'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(flags);
    glClear(flags);
    return 0;
]]
    },
    {
        name = 'clear_color',
        rettype = 'void',
        args = func_args('number r', 'number g', 'number b', 'number a'),
        cdef =
[[
    INIT_ARG();
    CHECK_NUMBER(float, r);
    CHECK_NUMBER(float, g);
    CHECK_NUMBER(float, b);
    CHECK_NUMBER(float, a);
    glClearColor(r, g, b, a);
    return 0;
]]
    },
    {
        name = 'enable',
        returns = {},
        args = func_args('integer pname'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(enable);
    glEnable(enable);
    return 0;
]]
    },
    {
        name = 'disable',
        returns = {},
        args = func_args('integer pname'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(disable);
    glDisable(disable);
    return 0;
]]
    },
    {
        name = 'blend_func',
        returns = {},
        args = func_args('integer sfactor', 'integer dfactor'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(sfactor);
    CHECK_INTEGER(dfactor);
    glBlendFunc(sfactor, dfactor);
    return 0;
]]
    },
    {
        name = 'blend_equation',
        returns = {},
        args = func_args('integer eq'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(eq);
    glBlendEquation(eq);
    return 0;
]]
    },
    {
        name = 'draw_arrays',
        returns = {},
        args = func_args('integer mode', 'integer start', 'integer count'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(mode);
    CHECK_INTEGER(start);
    CHECK_INTEGER(count);
    glDrawArrays(mode, start, count);
    return 0;
]]
    },
    {
        name = 'draw_elements',
        returns = {},
        args = func_args('integer mode', 'integer start', 'integer type', 'integer indices'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(mode);
    CHECK_INTEGER(count);
    CHECK_INTEGER(type);
    CHECK_INTEGER(indices);
    glDrawElements(mode, count, type, (void*)indices);
    return 0;
]]
    },
    { name = 'break' },
    {
        name = 'get_uniform_location',
        returns = {'integer'},
        args = func_args('string name'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_STRING(name);
    int loc = glGetUniformLocation(prog, name);
    PUSH_INTEGER(loc);
    return 1;
]]
    },
    {
        name = 'get_attrib_location',
        returns = {'integer'},
        args = func_args('string name'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_STRING(name);
    int loc = glGetAttribLocation(prog, name);
    PUSH_INTEGER(loc);
    return 1;
]]
    },
    {
        name = 'uniform_matrix4fv',
        rettype = 'void',
        args = func_args('integer loc', 'integer count', 'integer transpose', 'const float* v'),
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
    },
    { name = 'break' },
    {
        name = 'gen_vertex_arrays',
        rettype = 'void',
        args = func_args('integer n', 'integer* vaos'),
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
    },
    {
        name = 'delete_vertex_arrays',
        rettype = 'void',
        args = func_args('integer n', 'integer* vaos'),
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
    },
    {
        name = 'enable_vertex_attrib_array',
        rettype = 'void',
        args = func_args('integer attrib'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glEnableVertexAttribArray(attrib);
    return 0;
]]
    },
    {
        name = 'disable_vertex_attrib_array',
        rettype = 'void',
        args = func_args('integer attrib'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glDisableVertexAttribArray(attrib);
    return 0;
]]
    },
    {
        name = 'vertex_attrib_pointer',
        rettype = 'void',
        args = func_args("integer attrib", "integer size", "integer type", "boolean normalized", "integer stride", "integer offset"),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(attrib);
    CHECK_INTEGER(size);
    CHECK_INTEGER(type);
    CHECK_BOOLEAN(normalized);
    CHECK_INTEGER(stride);
    CHECK_INTEGER(offset);
    glVertexAttribPointer(attrib, size, type, normalized, stride, (void*)offset);
    return 0;
]]
    },
    {
        name = 'bind_vertex_array',
        rettype = 'void',
        args = func_args('integer vao'),
        cdef =
[[
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    INIT_ARG();
    OPT_INTEGER(vao, 0);
    glBindVertexArray(vao);
#endif
    return 0;
]]
    },
    { name = 'break' },
    {
        name = 'gen_buffers',
        rettype = 'void',
        args = func_args('integer n', 'integer* buffers'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        glObject t;
        glGenBuffers(1, &t);
        lua_pushinteger(L, t);
    }
    return n;
]]
    },
    {
        name = 'delete_buffers',
        rettype = 'void',
        args = func_args('integer n', 'integer* buffers'),
        cdef =
[[
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        glDeleteBuffers(1, &b);
    }
    return 0;
]]
    },
    {
        name = 'bind_buffer',
        rettype = 'void',
        args = func_args('integer target', 'integer buffer'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(buf, 0);
    glBindBuffer(target, buf);
    return 0;
]]
    },
    {
        name = 'buffer_data',
        rettype = 'void',
        args = func_args("uint32_t target", "size_t size", "const void* data", "uint32_t usage"),
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
    },
    {
        name = 'buffer_sub_data',
        rettype = 'void',
        args = func_args("uint32_t target", "integer offset", "size_t size", "const void* data"),
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
    },
    { name = 'break' },
    {
        name = 'gen_textures',
        rettype = 'void',
        args = func_args('integer n', 'integer* textures'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(tex, 0);
    glBindTexture(target, tex);
    return 0;
]]
    },
    {
        name = 'delete_textures',
        rettype = 'void',
        args = func_args('integer n', 'integer* textures'),
        cdef =
[[
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(tex);
        glDeleteTextures(1, &tex);
    }
    return 0;
]]
    },
    {
        name = 'bind_texture',
        rettype = 'void',
        args = func_args('integer target', 'integer texture'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(tex, 0);
    glBindTexture(target, tex);
    return 0;
]]
    },
    {
        name = 'tex_parameteri',
        rettype = 'void',
        args = func_args('integer pname', 'integer param'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(tex);
    CHECK_INTEGER(pname);
    CHECK_INTEGER(param);
    glTexParameteri(tex, pname, param);
    return 0;
]]
    },
    {
        name = 'tex_parameterf',
        rettype = 'void',
        args = func_args('integer pname', 'float param'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(tex);
    CHECK_INTEGER(pname);
    CHECK_INTEGER(param);
    glTexParameterf(tex, pname, param);
    return 0;
]]
    },
    {
        name = 'tex_image2d',
        rettype = 'void',
        args = func_args("integer target", "integer level", "integer internalf", "integer w", "integer h", "integer border", "integer format", "integer type", "const void* data"),
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
    },
    { name = 'break' },
    {
        name = 'gen_framebuffers',
        rettype = 'void',
        args = func_args('integer n', 'integer* framebuffers'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int f;
        glGenFramebuffers(1, &f);
        lua_pushinteger(L, f);
    }
    return n;
]]
    },
    {
        name = 'delete_framebuffers',
        rettype = 'void',
        args = func_args('integer n', 'integer* framebuffers'),
        cdef =
[[
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(fbo);
        glDeleteFramebuffers(1, &fbo);
    }
    return 0;
]]
    },
    {
        name = 'bind_framebuffer',
        rettype = 'void',
        args = func_args('integer target', 'integer framebuffer'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(fbo, 0);
    glBindFramebuffer(target, fbo);
    return 0;
]]
    },
    {
        name = 'framebuffer_framebuffer2d',
        rettype = 'void',
        args = func_args("integer target", "integer attachment", "integer textarget", "integer texture", "integer level"),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(attachment);
    CHECK_INTEGER(textarget);
    CHECK_INTEGER(texture);
    CHECK_INTEGER(level);
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
    return 0;
]]
    },
    { name = 'break' },
    {
        name = 'create_program',
        rettype = 'integer',
        args = {},
        cdef =
[[
    int p = glCreateProgram();
    PUSH_INTEGER(p);
    return 1;
]]
    },
    {
        name = 'delete_program',
        rettype = 'void',
        args = func_args('integer program'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(p);
    glDeleteProgram(p);
    return 0;
]]
    },
    {
        name = 'use_program',
        rettype = 'void',
        args = func_args('integer program'),
        cdef =
[[
    INIT_ARG();
    OPT_INTEGER(p, 0);
    glUseProgram(p);
    return 0;
]]
    },
    {
        name = 'attach_shader',
        rettype = 'void',
        args = func_args('integer program', 'integer shader'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_INTEGER(shader);
    glAttachShader(prog, shader);
    return 0;
]]
    },
    {
        name = 'link_program',
        rettype = 'void',
        args = func_args('integer program'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(p);
    glLinkProgram(p);
    return 0;
]]
    },
    {
        name = 'get_programiv',
        rettype = 'void',
        args = func_args('integer program', 'integer pname', 'int* pout'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(s);
    CHECK_INTEGER(pname);
    int out;
    glGetProgramiv(s, pname, &out);
    PUSH_INTEGER(out);
    return 1;
]]
    },
    {
        name = 'get_program_info_log',
        returns = {'string'},
        args = func_args('integer program'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(s);
    char log[1024];
    glGetProgramInfoLog(s, 1024, NULL, log);
    PUSH_STRING(log);
    return 1;
]]
    },

    { name = 'break' },
    --[[ Shader ]]
    {
        name = 'create_shader',
        rettype = 'integer',
        args = func_args('integer shader_type'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(type);
    int s = glCreateShader(type);
    PUSH_INTEGER(s);
    return 1;
]]
    },
    {
        name = 'delete_shader',
        rettype = 'void',
        args = func_args('integer shader'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(s);
    glDeleteShader(s);
    return 0;
]]
    },
    {
        name = 'shader_source',
        rettype = 'void',
        args = func_args('integer shader', 'string sources', 'const int* out'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(s);
    CHECK_STRING(src);
    glShaderSource(s, 1, &src, NULL);
    return 0;
]]
    },
    {
        name = 'compile_shader',
        rettype = 'void',
        args = func_args('integer shader'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(s);
    glCompileShader(s);
    return 0;
]]
    },
    {
        name = 'get_shaderiv',
        rettype = 'void',
        args = func_args('integer shader', 'integer pname'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(s);
    CHECK_INTEGER(pname);
    int out;
    glGetShaderiv(s, pname, &out);
    PUSH_INTEGER(out);
    return 1;
]]
    },
    {
        name = 'get_shader_info_log',
        returns = {'string'},
        args = func_args('integer shader'),
        cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(s);
    char log[1024];
    glGetShaderInfoLog(s, 1024, NULL, log);
    PUSH_STRING(log);
    return 1;
]]
    },
    { name = 'break' }
}
return functions