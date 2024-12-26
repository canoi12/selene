--- @alias generator.BlockType
--- | 'breakline'
--- | 'comment'
--- | 'c code'
--- | 'lua code'
--- | 'module function'
--- | 'enum'

--- @alias generator.Group [generator.CommentBlock|generator.CCodeBlock|generator.ModuleFuncBlock]

--- @class generator.Lib
--- @field name string
--- @field header generator.Group
--- @field source generator.Group
Lib = {}
Lib.__index = Lib

function Lib.create(name)
    local l = {}
    l.name = name
    l.header = {}
    l.source = {}
    return setmetatable(l, Lib)
end

function Lib:add_source(blk)
    if #blk > 0 then
        for i,b in ipairs(blk) do
            table.insert(self.source, b)
        end
    else
        table.insert(self.source, blk)
    end
end

function Lib:add_header(blk)
    if #blk > 0 then
        for i,b in ipairs(blk) do
            table.insert(self.header, b)
        end
    else
        table.insert(self.header, blk)
    end
end

local function generate_enums(lib)
    local file = ''
    if lib.enums then
        file = file .. 'BEGIN_ENUM(' .. lib.name .. '_Enums)\n'
        for i,enum in ipairs(lib.enums) do
            local prefix = lib.enum_prefix
            local name = enum
            if type(enum) == 'table' then
                if enum.iif then
                    file = file .. '#if ' .. enum.iif .. '\n'
                end
                name = enum[1]
            end
            if prefix and string.contains(name, prefix) then
                local plen = string.len(prefix)
                local e = string.sub(name, plen, string.len(name))
                if string.sub(e, 1, 1) == '_' then e = string.sub(e, 2, string.len(e)) end
                file = file .. '    ENUM_FIELD(' .. e .. ', ' .. prefix .. '),\n'
            else
                file = file .. '    {"' .. name .. '", ' .. name .. '},\n'
            end
            if type(enum) == 'table' then
                if enum.iif then
                    file = file .. '#endif\n'
                end
            end
        end
        file = file .. 'END_ENUM()\n'
    end
    return file
end

function generate_header(lib)
    print('Generating header')
    local hfile = '#ifndef SELENE_' .. string.upper(lib.name) .. '_H_\n'
    hfile = hfile .. '#define SELENE_' .. string.upper(lib.name) .. '_H_\n'

    if lib.header_include then
        hfile = hfile .. lib.header_include
    end

    hfile = hfile .. generate_enums(lib)

    if lib.meta_types then
        hfile = hfile .. '\n'
        for i,meta in ipairs(lib.meta_types) do
            hfile = hfile .. 'typedef ' .. meta.ctype .. ' ' .. meta.name .. ';\n'
            hfile = hfile .. 'extern MODULE_FUNCTION(' .. meta.name .. ', meta);\n'
        end
        hfile = hfile .. '\n'
        hfile = hfile .. 'const BEGIN_REG(' .. lib.name .. '_MetaTypes)\n'
        for i,meta in ipairs(lib.meta_types) do
            hfile = hfile .. '    REG_FIELD(' .. meta.name .. ', meta),\n'
        end
        hfile = hfile .. 'END_REG()\n\n'
    end
    hfile = hfile .. process_blocks(lib, lib.header)
    hfile = hfile .. 'SELENE_API int luaopen_' .. lib.name .. '(lua_State* L);\n'
    hfile = hfile .. '\n#endif /* SELENE_' .. string.upper(lib.name) .. '_H_ */'

    return hfile
end

function generate_source(lib)
    print('Generating source')
    local cfile = process_blocks(lib, lib.source)

    cfile = cfile .. 'int luaopen_' .. lib.name .. '(lua_State* L) {\n'
    cfile = cfile .. '    BEGIN_REG(reg)\n'
    for i,block in ipairs(lib.source) do
        if block.type == 'function'  then
            cfile = cfile .. '        REG_FIELD(' .. lib.name .. ', ' .. block.name .. '),\n'
        end 
    end
    cfile = cfile .. '    END_REG()\n'
    cfile = cfile .. '    luaL_newlib(L, reg);\n'

    if lib.meta_types and #lib.meta_types > 0 then
        cfile = cfile .. '    for (int i = 0; ' .. lib.name .. '_MetaTypes[i].name != NULL; i++) {\n'
        cfile = cfile .. '        ' .. lib.name .. '_MetaTypes[i].func(L);\n'
        cfile = cfile .. '        lua_setfield(L, -2, ' .. lib.name .. '_MetaTypes[i].name);\n'
        cfile = cfile .. '    }\n'
    end
    if lib.enums then
        cfile = cfile .. '    LOAD_ENUM(' .. lib.name .. '_Enums);\n'
    end
    cfile = cfile .. '    return 1;\n'
    cfile = cfile .. '}\n'

    if lib.meta_types then
        cfile = cfile .. '\n'
        for i,meta in ipairs(lib.meta_types) do
            cfile = cfile .. '\n/* ' .. meta.name .. ' */\n\n'
            cfile = cfile .. process_blocks(meta, meta.blocks)
            cfile = cfile .. 'MODULE_FUNCTION(' .. meta.name .. ', meta) {\n'
            cfile = cfile .. '    BEGIN_REG(reg)\n'
            for j,blk in ipairs(meta.blocks) do
                if blk.type == 'function' then
                    cfile = cfile .. '        REG_FIELD(' .. meta.name .. ', ' .. blk.name .. '),\n'
                end
            end
            cfile = cfile .. '    END_REG()\n'
            cfile = cfile .. '    luaL_newmetatable(L, "' .. meta.name .. '");\n'
            cfile = cfile .. '    luaL_setfuncs(L, reg, 0);\n'
            cfile = cfile .. '    lua_pushvalue(L, -1);\n'
            cfile = cfile .. '    lua_setfield(L, -2, "__index");\n'
            cfile = cfile .. '    return 1;\n'
            cfile = cfile .. '}\n'
        end
        cfile = cfile .. '\n'
    end
    return cfile
end

function generate_single(lib)
    local file = ''

    if lib.header_include then
        file = file .. lib.header_include
    end

    file = file .. generate_enums(lib)

    if lib.meta_types then
        file = file .. '\n'
        for i,meta in ipairs(lib.meta_types) do
            file = file .. 'typedef ' .. meta.ctype .. ' ' .. meta.name .. ';\n'
            file = file .. 'extern MODULE_FUNCTION(' .. meta.name .. ', meta);\n'
        end
        file = file .. '\n'
        file = file .. 'const BEGIN_REG(' .. lib.name .. '_MetaTypes)\n'
        for i,meta in ipairs(lib.meta_types) do
            file = file .. '    REG_FIELD(' .. meta.name .. ', meta),\n'
        end
        file = file .. 'END_REG()\n\n'
    end

    file = file .. process_blocks(lib, lib.header)

    file = file .. process_blocks(lib, lib.source)

    file = file .. 'int luaopen_' .. lib.name .. '(lua_State* L) {\n'
    file = file .. '    BEGIN_REG(reg)\n'
    for i,block in ipairs(lib.source) do
        if block.type == 'function'  then
            file = file .. '        REG_FIELD(' .. lib.name .. ', ' .. block.name .. '),\n'
        end 
    end
    file = file .. '    END_REG()\n'
    file = file .. '    luaL_newlib(L, reg);\n'

    if lib.meta_types and #lib.meta_types > 0 then
        file = file .. '    for (int i = 0; ' .. lib.name .. '_MetaTypes[i].name != NULL; i++) {\n'
        file = file .. '        ' .. lib.name .. '_MetaTypes[i].func(L);\n'
        file = file .. '        lua_setfield(L, -2, ' .. lib.name .. '_MetaTypes[i].name);\n'
        file = file .. '    }\n'
    end
    if lib.enums then
        file = file .. '    LOAD_ENUM(' .. lib.name .. '_Enums);\n'
    end
    file = file .. '    return 1;\n'
    file = file .. '}\n'

    if lib.meta_types then
        file = file .. '\n'
        for i,meta in ipairs(lib.meta_types) do
            file = file .. '\n/* ' .. meta.name .. ' */\n\n'
            file = file .. process_blocks(meta, meta.blocks)
            file = file .. 'MODULE_FUNCTION(' .. meta.name .. ', meta) {\n'
            file = file .. '    BEGIN_REG(reg)\n'
            for j,blk in ipairs(meta.blocks) do
                if blk.type == 'function' then
                    file = file .. '        REG_FIELD(' .. meta.name .. ', ' .. blk.name .. '),\n'
                end
            end
            file = file .. '    END_REG()\n'
            file = file .. '    luaL_newmetatable(L, "' .. meta.name .. '");\n'
            file = file .. '    luaL_setfuncs(L, reg, 0);\n'
            file = file .. '    lua_pushvalue(L, -1);\n'
            file = file .. '    lua_setfield(L, -2, "__index");\n'
            file = file .. '    return 1;\n'
            file = file .. '}\n'
        end
        file = file .. '\n'
    end
    return file
end

function generate_lua_defs(lib)
    print('Generating lua definitions')
    local lfile = '--- @meta\n'
    lfile = lfile .. '--- @class ' .. lib.name .. '\n'
    if lib.enums then
        for i,enum in ipairs(lib.enums) do
            -- print(enum)
            local prefix = lib.enum_prefix
            if type(enum) == 'table' then
                enum = enum[1]
            end
            if prefix and string.contains(enum, prefix) then
                local plen = string.len(prefix)
                local e = string.sub(enum, plen, string.len(enum))
                if string.sub(e, 1, 1) == '_' then e = string.sub(e, 2, string.len(e)) end
                lfile = lfile .. '--- @field ' .. e .. ' integer\n'
            else
                lfile = lfile .. '--- @field ' .. enum .. ' integer\n'
            end
        end
    end
    lfile = lfile .. 'local ' .. lib.name .. ' = {}\n\n'
    for i,blk in ipairs(lib.source) do
        if blk.type == 'function' then
            for j,farg in ipairs(blk.args) do
                -- print(blk.name)
                lfile = lfile .. '--- @param ' .. farg.name .. ' ' .. farg.type .. '\n'
            end
            if blk.returns and #blk.returns > 0 then
                lfile = lfile .. '--- @return '
                for k,r in ipairs(blk.returns) do
                    lfile = lfile .. r
                    if k ~= #blk.returns then
                        lfile = lfile .. ', '
                    end
                end
                lfile = lfile .. '\n'
            end
            lfile = lfile .. 'function ' .. lib.name .. '.' .. blk.name .. '('
            for j,a in ipairs(blk.args) do
                lfile = lfile .. a.name
                if j ~= #blk.args then
                    lfile = lfile .. ', '
                end
            end
            lfile = lfile .. ') end\n\n'
        end
    end

    if lib.meta_types then
        for i,meta in ipairs(lib.meta_types) do
            lfile = lfile .. '--- @class ' .. meta.name .. '\n'
            lfile = lfile .. 'local ' .. meta.name .. ' = {}\n\n'
            for j,blk in ipairs(meta.blocks) do
                if blk.type == 'function' then
                    for j,farg in ipairs(blk.args) do
                        -- print(blk.name)
                        if farg.name ~= 'self' then
                            lfile = lfile .. '--- @param ' .. farg.name .. ' ' .. farg.type .. '\n'
                        end
                    end
                    if blk.returns and #blk.returns > 0 then
                        lfile = lfile .. '--- @return '
                        for k,r in ipairs(blk.returns) do
                            lfile = lfile .. r
                            if k ~= #blk.returns then
                                lfile = lfile .. ', '
                            end
                        end
                        lfile = lfile .. '\n'
                    end
                    lfile = lfile .. 'function ' .. meta.name .. ':' .. blk.name .. '('
                    for k,a in ipairs(blk.args) do
                        if a.name ~= 'self' then
                            lfile = lfile .. a.name
                        end
                        if a.name ~= 'self' and k ~= #blk.args then
                            lfile = lfile .. ', '
                        end
                    end
                    lfile = lfile .. ') end\n\n'
                end
            end
        end
    end
    return lfile
end

function __generate_lib(filename, lib)
    print('Generating ' .. filename .. '.h')
    local hfile = '#ifndef SELENE_' .. string.upper(lib.name) .. '_H_\n'
    hfile = hfile .. '#define SELENE_' .. string.upper(lib.name) .. '_H_\n'

    if lib.header_include then
        hfile = hfile .. lib.header_include
    end

    if lib.enums then
        hfile = hfile .. 'BEGIN_ENUM(' .. lib.name .. '_Enums)\n'
        for i,enum in ipairs(lib.enums) do
            local prefix = lib.enum_prefix
            if prefix and string.contains(enum, prefix) then
                local plen = string.len(prefix)
                local e = string.sub(enum, plen, string.len(enum))
                if string.sub(e, 1, 1) == '_' then e = string.sub(e, 2, string.len(e)) end
                hfile = hfile .. '    ENUM_FIELD(' .. e .. ', ' .. prefix .. '),\n'
            else
                hfile = hfile .. '    {"' .. enum .. '", ' .. enum .. '},\n'
            end
        end
        hfile = hfile .. 'END_REG()\n'
    end

    if lib.meta_types then
        hfile = hfile .. '\n'
        for i,meta in ipairs(lib.meta_types) do
            hfile = hfile .. 'typedef ' .. meta.ctype .. ' ' .. meta.name .. ';\n'
            hfile = hfile .. 'extern MODULE_FUNCTION(' .. meta.name .. ', meta);\n'
        end
        hfile = hfile .. '\n'
        hfile = hfile .. 'const BEGIN_REG(' .. lib.name .. '_MetaTypes)\n'
        for i,meta in ipairs(lib.meta_types) do
            hfile = hfile .. '    REG_FIELD(' .. meta.name .. ', meta),\n'
        end
        hfile = hfile .. 'END_REG()\n\n'
    end

    hfile = hfile .. process_blocks(lib, lib.header)

    hfile = hfile .. 'SELENE_API int luaopen_' .. lib.name .. '(lua_State* L);\n'

    hfile = hfile .. '\n#endif /* SELENE_' .. string.upper(lib.name) .. '_H_ */'
    io.writefile(_MAIN_SCRIPT_DIR .. '/' .. filename .. '.h', hfile)

    print('Generating ' .. filename .. '.c')
    local cfile = '#include "' .. filename .. '.h"\n\n'

    cfile = cfile .. process_blocks(lib, lib.source)

    cfile = cfile .. 'int luaopen_' .. lib.name .. '(lua_State* L) {\n'
    cfile = cfile .. '    BEGIN_REG(reg)\n'
    for i,block in ipairs(lib.source) do
        if block.type == 'function'  then
            cfile = cfile .. '        REG_FIELD(' .. lib.name .. ', ' .. block.name .. '),\n'
        end 
    end
    cfile = cfile .. '    END_REG()\n'
    cfile = cfile .. '    luaL_newlib(L, reg);\n'
    
    if lib.meta_types and #lib.meta_types > 0 then
        cfile = cfile .. '    for (int i = 0; ' .. lib.name .. '_MetaTypes[i].name != NULL; i++) {\n'
        cfile = cfile .. '        ' .. lib.name .. '_MetaTypes[i].func(L);\n'
        cfile = cfile .. '        lua_setfield(L, -2, ' .. lib.name .. '_MetaTypes[i].name);\n'
        cfile = cfile .. '    }\n'
    end
    cfile = cfile .. '    LOAD_ENUM(' .. lib.name .. '_Enums);\n'
    cfile = cfile .. '    return 1;\n'
    cfile = cfile .. '}\n'

    if lib.meta_types then
        cfile = cfile .. '\n'
        for i,meta in ipairs(lib.meta_types) do
            cfile = cfile .. '\n/* ' .. meta.name .. ' */\n\n'
            cfile = cfile .. process_blocks(meta, meta.blocks)
            cfile = cfile .. 'MODULE_FUNCTION(' .. meta.name .. ', meta) {\n'
            cfile = cfile .. '    BEGIN_REG(reg)\n'
            for j,blk in ipairs(meta.blocks) do
                if blk.type == 'function' then
                    cfile = cfile .. '        REG_FIELD(' .. meta.name .. ', ' .. blk.name .. '),\n'
                end
            end
            cfile = cfile .. '    END_REG()\n'
            cfile = cfile .. '    luaL_newmetatable(L, "' .. meta.name .. '");\n'
            cfile = cfile .. '    luaL_setfuncs(L, reg, 0);\n'
            cfile = cfile .. '    lua_pushvalue(L, -1);\n'
            cfile = cfile .. '    lua_setfield(L, -2, "__index");\n'
            cfile = cfile .. '    return 1;\n'
            cfile = cfile .. '}\n'
        end
        cfile = cfile .. '\n'
    end

    io.writefile(_MAIN_SCRIPT_DIR .. '/' .. filename .. '.c', cfile)

    print('Generating ' .. filename .. '.lua')
    local lfile = '--- @meta\n'
    lfile = lfile .. '--- @class ' .. lib.name .. '\n'
    if lib.enums then
        for i,enum in ipairs(lib.enums) do
            -- print(enum)
            local prefix = lib.enum_prefix
            if type(enum) == 'table' then
                enum = enum[1]
            end
            if prefix and string.contains(enum, prefix) then
                local plen = string.len(prefix)
                local e = string.sub(enum, plen, string.len(enum))
                if string.sub(e, 1, 1) == '_' then e = string.sub(e, 2, string.len(e)) end
                lfile = lfile .. '--- @field ' .. e .. ' integer\n'
            else
                lfile = lfile .. '--- @field ' .. enum .. ' integer\n'
            end
        end
    end
    lfile = lfile .. 'local ' .. lib.name .. ' = {}\n\n'
    for i,blk in ipairs(lib.source) do
        if blk.type == 'function' then
            for j,farg in ipairs(blk.args) do
                -- print(blk.name)
                lfile = lfile .. '--- @param ' .. farg.name .. ' ' .. farg.type .. '\n'
            end
            if blk.returns and #blk.returns > 0 then
                lfile = lfile .. '--- @return '
                for k,r in ipairs(blk.returns) do
                    lfile = lfile .. r
                    if k ~= #blk.returns then
                        lfile = lfile .. ', '
                    end
                end
                lfile = lfile .. '\n'
            end
            lfile = lfile .. 'function ' .. lib.name .. '.' .. blk.name .. '('
            for j,a in ipairs(blk.args) do
                lfile = lfile .. a.name
                if j ~= #blk.args then
                    lfile = lfile .. ', '
                end
            end
            lfile = lfile .. ') end\n\n'
        end
    end

    if lib.meta_types then
        for i,meta in ipairs(lib.meta_types) do
            lfile = lfile .. '--- @class ' .. meta.name .. '\n'
            lfile = lfile .. 'local ' .. meta.name .. ' = {}\n\n'
            for j,blk in ipairs(meta.blocks) do
                if blk.type == 'function' then
                    for j,farg in ipairs(blk.args) do
                        -- print(blk.name)
                        if farg.name ~= 'self' then
                            lfile = lfile .. '--- @param ' .. farg.name .. ' ' .. farg.type .. '\n'
                        end
                    end
                    if blk.returns and #blk.returns > 0 then
                        lfile = lfile .. '--- @return '
                        for k,r in ipairs(blk.returns) do
                            lfile = lfile .. r
                            if k ~= #blk.returns then
                                lfile = lfile .. ', '
                            end
                        end
                        lfile = lfile .. '\n'
                    end
                    lfile = lfile .. 'function ' .. meta.name .. ':' .. blk.name .. '('
                    for k,a in ipairs(blk.args) do
                        if a.name ~= 'self' then
                            lfile = lfile .. a.name
                        end
                        if a.name ~= 'self' and k ~= #blk.args then
                            lfile = lfile .. ', '
                        end
                    end
                    lfile = lfile .. ') end\n\n'
                end
            end
        end
    end
    
    io.writefile(_MAIN_SCRIPT_DIR .. '/' .. filename .. '.lua', lfile)
end

function generate_lib(filename, lib)
    local hfile = generate_header(lib)
    print("Writing " .. filename .. '.h')
    io.writefile(_MAIN_SCRIPT_DIR .. '/' .. filename .. '.h', hfile)

    local cfile = generate_source(lib)
    cfile = '#include "' .. filename .. '.h"\n\n' .. cfile
    print("Writing " .. filename .. '.c')
    io.writefile(_MAIN_SCRIPT_DIR .. '/' .. filename .. '.c', cfile)

    local lfile = generate_lua_defs(lib)
    print("Writing " .. filename .. '_def.lua')
    io.writefile(_MAIN_SCRIPT_DIR .. '/' .. filename .. '_def.lua', lfile)
end

function comment_block(comment)
    local block = {
        type = 'comment',
        value = comment
    }
    return block
end

function c_code_block(code)
    local block = {
        type = 'c code',
        code = code
    }
    return block
end

function lua_code_block(code)
    local block = {
        type = 'lua code',
        code = code
    }
    return block
end

function enum_block(enums, prefix)
    local block = {
        type = 'enum',
        enums = enums,
        prefix = prefix
    }
    return block
end

function function_block(returns, name, args, options)
    local block = {}
    block.type = 'function'
    block.name = name
    if string.contains(returns, ', ') then
        block.returns = string.explode(returns, ', ')
    elseif string.len(returns) > 0 then
        block.returns = {returns}
    else
        block.returns = {}
    end
    block.args = {}
    if string.contains(args, ', ') then
        local aux = string.explode(args, ', ')
        for i,a in ipairs(aux) do
            local ax = string.explode(a, ' ')
            table.insert(block.args, {type = ax[1], name = ax[2]})
        end
    elseif string.len(args) > 0 then
        local ax = string.explode(args, ' ')
        table.insert(block.args, {type = ax[1], name = ax[2]})
    end

    for key,val in pairs(options or {}) do
        block[key] = val
    end
    return block
end

function process_blocks(lib, blocks)
    local source = ''
    for i,blk in ipairs(blocks) do
        if blk.type == 'comment' then
            source = source .. '/*' .. blk.value .. '*/'
        elseif blk.type == 'function' then
            source = source .. 'static MODULE_FUNCTION(' .. lib.name .. ', ' .. blk.name .. ') {\n'
            if blk.cdef then
                source = source .. blk.cdef
            elseif blk.ccall then
                source = source .. '    INIT_ARG();\n'
                for _,a in ipairs(blk.args) do
                    if a.type == 'integer' then
                        source = source .. '    CHECK_INTEGER(' .. a.name .. ');\n'
                    elseif a.type == 'integer|nil' then
                        source = source .. '    OPT_INTEGER(' .. a.name .. ', 0);\n'
                    elseif a.type == 'uinteger' then
                        source = source .. '    CHECK_NUMBER(unsigned int, ' .. a.name .. ');\n'
                    elseif a.type == 'number' then
                        source = source .. '    CHECK_NUMBER(float, ' .. a.name .. ');\n'
                    elseif a.type == 'string' then
                        source = source .. '    CHECK_STRING(' .. a.name .. ');\n'
                    elseif a.type == 'boolean' then
                        source = source .. '    GET_BOOLEAN(' .. a.name .. ');\n'
                    elseif a.type == 'lightuserdata' then
                        source = source .. '    CHECK_LUDATA(void, ' .. a.name .. ');\n'
                    end
                end
                source = source .. '    ' .. blk.ccall .. '\n'
                source = source .. '    return ' .. #blk.returns .. ';\n'
            end
            source = source .. '}\n'
        elseif blk.type == 'c code' then
            source = source .. blk.code
        elseif blk.type == 'enum' then
            source = source .. 'BEGIN_ENUM(' .. lib.name .. '_Enums)\n'
            -- print(blk.enums, blk.prefix)
            for i,enum in ipairs(blk.enums) do
                if blk.prefix and string.contains(enum, blk.prefix) then
                    local plen = string.len(blk.prefix)
                    local e = string.sub(enum, plen, string.len(enum))
                    if string.sub(e, 1, 1) == '_' then e = string.sub(e, 2, string.len(e)) end
                    source = source .. '    ENUM_FIELD(' .. e .. ', ' .. blk.prefix .. '),\n'
                else
                    source = source .. '    {"' .. enum .. '", ' .. enum .. '},\n'
                end
            end
            source = source .. 'END_ENUM();\n'
        end
        source = source .. '\n'
    end
    return source
end