local gl = {}
gl.enums = require('enums')

function func_args(...)
    local args = {...}
    local res = {}
    for i,a in ipairs(args) do
        -- print(a)
        local p = string.explode(a, ' ')
        if #p > 2 then
            for c=2,#p-1 do
                p[1] = p[1] .. ' ' .. p[c]
            end
            p[2] = p[#p]
        end
        res[i] = {
            type = p[1],
            name = p[2]
        }
    end
    return res
end

gl.functions = require('functions')

function uniform_functions(ftype)
    local ret = {}
    local aux = {
        {ftype .. ' v0'},
        {ftype .. ' v0', ftype .. ' v1'},
        {ftype .. ' v0', ftype .. ' v1', ftype .. ' v2'},
        {ftype .. ' v0', ftype .. ' v1', ftype .. ' v2', ftype .. ' v3'},
    }
    local suffix = 'f'
    if ftype == 'int' then
        suffix = 'i'
    end
    local ltype = 'number'
    if ftype == 'int' then
        ltype = 'integer'
    end
    local laux = {
        {ltype .. ' v0'},
        {ltype .. ' v0', ltype .. ' v1'},
        {ltype .. ' v0', ltype .. ' v1', ltype .. ' v2'},
        {ltype .. ' v0', ltype .. ' v1', ltype .. ' v2', ltype .. ' v3'},
    }
    -- print(suffix)
    for i=1,4 do
        local args = {}
        local b = {
            name = 'uniform' .. i .. suffix,
            args = func_args('integer loc', table.unpack(laux[i])),
            cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(loc);
]]
        }
        for j=1,i do
            b.cdef = b.cdef .. '    CHECK_NUMBER(' .. ftype .. ', v' .. j .. ');\n'
        end
        b.cdef = b.cdef .. '    glUniform' .. i .. suffix .. '(loc, '
        for j=1,i do
            b.cdef = b.cdef .. 'v' .. j
            if j ~= i then
                b.cdef = b.cdef .. ', '
            end
        end
        b.cdef = b.cdef .. ');\n'
        b.cdef = b.cdef .. '    return 0;\n'
        table.insert(gl.functions, b)

        local d = {
            name = 'uniform' .. i .. suffix .. 'v',
            args = func_args('integer loc', 'number ...'),
            cdef =
[[
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(n);
    CHECK_LUDATA(float, data);
]]
        }
        d.cdef = d.cdef .. '    glUniform' .. i .. suffix .. 'v(loc, n, data);\n'
        d.cdef = d.cdef .. '    return 0;\n'
        table.insert(gl.functions, d)
    end
    return table.unpack(ret)
end

for i,func in ipairs({uniform_functions('float')}) do
    table.insert(gl.functions, func)
end
for i,func in ipairs({uniform_functions('int')}) do
    table.insert(gl.functions, func)
end

function generate_header()
    local file =
[[
#ifndef SELENE_GL_H_
#define SELENE_GL_H_

#if defined(OS_EMSCRIPTEN) || defined(OS_ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <glad/glad.h>
#endif

#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

typedef unsigned int glObject;

]]

    file = file .. 'BEGIN_ENUM(gl_Enums)\n'
    for i,enum in pairs(gl.enums) do
        file = file .. '    ENUM_FIELD(' .. enum .. ', GL_),\n'
    end
    file = file .. 'END_ENUM()\n'

    file = file .. '\n'

    -- for i,func in ipairs(gl.functions) do
    --     if func.name == 'break' then
    --         file = file .. '\n'
    --     else
    --         file = file .. 'typedef ' .. func.rettype
    --         file = file .. '(*GLPROC' .. func.name .. ')('
    --         local args = func.args or {}
    --         for j,a in ipairs(args) do
    --             -- print(a.type, a.name)
    --             file = file .. a.type .. ' ' .. a.name
    --             if j ~= #args then
    --                 file = file .. ', '
    --             end
    --         end
    --         file = file .. ');\n'
    --         file = file .. 'extern GLPROC' .. func.name .. ' gl' .. func.name .. ';\n'
    --     end
    -- end

    file = file ..
[[

int luaopen_gl(lua_State* L);

#endif /* SELENE_GL_H_ */]]

    io.writefile(_MAIN_SCRIPT_DIR .. '/lgl.h', file)
end

function generate_source()
    local file =
[[
#include "lgl.h"

float aux[512];

]]

    for i,func in ipairs(gl.functions) do
        if func.name == 'break' then
            file = file .. '\n'
        else
            file = file .. 'static MODULE_FUNCTION(gl, ' .. func.name .. ') {\n'
            if func.cdef then
                file = file .. func.cdef
            elseif func.call then
                file = file .. '    INIT_ARG();\n'
                local args = func.args or {}
                for _,a in ipairs(args) do
                    if a.type == 'integer' then
                        file = file .. '    CHECK_INTEGER(' .. a.name .. ');\n'
                    end
                end
                local returns = func.returns or {}
                for _,r in ipairs(returns) do
                    if r == 'string' then
                        
                    end
                end
            end
            file = file .. '}\n\n'
        end
    end

    file = file .. 'int luaopen_gl(lua_State* L) {\n'
    file = file .. '    BEGIN_REG(reg)\n'
    -- for i,func in ipairs(sdl.functions.lib.blocks) do
    --     file = file .. '        /* ' .. block.name .. ' */\n'
    for _,func in ipairs(gl.functions) do
        if func.name ~= 'break' then
            file = file .. '        REG_FIELD(gl, ' .. func.name .. '),\n'
        end
    end
    -- end
    file = file .. '    END_REG()\n'
    file = file .. '    luaL_newlib(L, reg);\n\n'
    file = file .. '    LOAD_ENUM(gl_Enums);\nreturn 1;\n}\n'

    io.writefile(_MAIN_SCRIPT_DIR .. '/lgl.c', file)
end

function generate_emmy()
local file =
[[
--- @meta
--- @class gl
]]

    for _,enum in pairs(gl.enums) do
        file = file .. '--- @field ' .. enum .. ' string\n'
    end
    file = file .. 'gl = {}\n'

    for _,func in ipairs(gl.functions) do
        if func.name == 'break' then
            file = file .. '\n'
        else
            local args = func.args or {}
            local returns = func.returns or {}
            for j,a in ipairs(args) do
                -- print(func.name, a.name, a.type)
                if a.name == '...' then
                    file = file .. '--- @vararg ' .. a.type .. '\n'
                else
                    file = file .. '--- @param ' .. a.name .. ' ' .. a.type .. '\n'
                end
            end
            if #returns > 0 then
                file = file .. '--- @return '
            
                for j,r in ipairs(returns) do
                    file = file .. r
                    if j ~= #returns then
                        file = file .. ', '
                    end
                end

                file = file .. '\n'
            end
            file = file .. 'function gl.' .. func.name .. '('
            for j,a in ipairs(args) do
                file = file .. a.name
                if j ~= #func.args then
                    file = file .. ', '
                end
            end
            file = file .. ') end\n'
        end
        
        -- file = file .. ')\n'
    end
    io.writefile(_MAIN_SCRIPT_DIR .. '/gl.lua', file)
end


function generate_files()
    generate_header()
    generate_source()
    generate_emmy()
end

newaction {
    trigger = 'generate',
    description = 'generate minigl files',
    execute = generate_files
}
