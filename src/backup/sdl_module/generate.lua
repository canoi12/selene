local sdl = {}
sdl.enums = require('enums')
sdl.functions = require('functions')

function generate_header()
    print ("Generating selene_sdl.h")
    local file =
[[
#ifndef SELENE_SDL_H_
#define SELENE_SDL_H_

#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

typedef SDL_Window* sdlWindow;
typedef SDL_GLContext* sdlGLContext;
typedef SDL_Renderer* sdlRenderer;
typedef SDL_Event sdlEvent;
typedef SDL_Texture* sdlTexture;

typedef SDL_RWops sdlRWops;

typedef SDL_Joystick* sdlJoystick;
typedef SDL_GameController* sdlGamepad;

typedef SDL_AudioDeviceID sdlAudioDeviceID;
typedef SDL_AudioStream* sdlAudioStream;

typedef struct {
    int top, count;
    sdlAudioStream* data;
    int* availables;
} AudioStreamPool;

typedef struct {
	FontData* font_data;
	sdlTexture* texture;
} sdlFont;

]]

    file = file .. 'BEGIN_ENUM(sdl_Enums)\n'
    for i,enum in pairs(sdl.enums) do
        if string.startswith(enum, 'AUDIO_') then
            file = file .. '    {"' .. enum .. '", ' .. enum .. '},\n'
        else
            file = file .. '    ENUM_FIELD(' .. enum .. ', SDL_),\n'
        end
    end
    file = file .. 'END_ENUM()\n'

    file = file .. '\n'

    file = file ..
[[

int luaopen_sdl(lua_State* L);

#endif /* SELENE_SDL_H_ */]]

    io.writefile(_MAIN_SCRIPT_DIR .. '/selene_sdl.h', file)
end

function generate_source()
    print ("Generating selene_sdl.c")
    file =
[[
#include "selene_sdl.h"

static SDL_Texture* texture_from_image_data(SDL_Renderer* renderer, int access, ImageData* img_data) {
    int w = img_data->width;
    int h = img_data->height;
    int format = 0;
    switch (img_data->pixel_format) {
        case SELENE_PIXEL_BGR: format = SDL_PIXELFORMAT_BGR888; break;
        case SELENE_PIXEL_RGB: format = SDL_PIXELFORMAT_RGB888; break;
        case SELENE_PIXEL_RGBA: format = SDL_PIXELFORMAT_RGBA32; break;
        case SELENE_PIXEL_BGRA: format = SDL_PIXELFORMAT_BGRA32; break;
    }
    return SDL_CreateTexture(renderer, format, access, w, h);
}

]]

    for _,meta in pairs(sdl.functions.metas) do
        file = file .. 'extern MODULE_FUNCTION(sdl' .. meta.name .. ', meta);\n'
    end

    file = file .. '\nstatic const luaL_Reg _sdl_meta_regs[] = {\n'
    for _,meta in pairs(sdl.functions.metas) do
        file = file .. '    {"' .. meta.name .. '", l_sdl' .. meta.name .. '_meta},\n'
    end
    file = file .. '};\n\n'

    for _,block in ipairs(sdl.functions.lib.blocks) do
        if block.description then
            file = file .. block.description .. '\n'
        end
        for i,func in ipairs(block.functions) do
            if func.name == 'break' then
                file = file .. '\n'
            else
                file = file .. 'static MODULE_FUNCTION(sdl, ' .. func.name .. ') {\n'
                if func.ccall then
                    file = file .. func.ccall
                end
                file = file .. '}\n\n'
            end
        end
    end

    file = file .. 'int luaopen_sdl(lua_State* L) {\n'
    file = file .. '    BEGIN_REG(reg)\n'
    for i,block in ipairs(sdl.functions.lib.blocks) do
        file = file .. '        /* ' .. block.name .. ' */\n'
        for _,func in ipairs(block.functions) do
            file = file .. '        REG_FIELD(sdl, ' .. func.name .. '),\n'
        end
    end
    file = file .. '    END_REG()\n'
    file = file .. '    luaL_newlib(L, reg);\n\n'
    file = file ..
[[
    for (int i = 0; _sdl_meta_regs[i].name != NULL; i++) {
        _sdl_meta_regs[i].func(LUA_STATE_NAME);
        lua_setfield(L, -2, _sdl_meta_regs[i].name);
    }
    
    LOAD_ENUM(sdl_Enums);
]]

    file = file .. '    return 1;\n}\n'
    io.writefile(_MAIN_SCRIPT_DIR .. '/selene_sdl.c', file)


    
    for _,meta in ipairs(sdl.functions.metas) do
        file =
[[
#include "selene_sdl.h"

]]
        local functions = meta.functions or {}
        for i,func in ipairs(functions) do
            if func.name == 'break' then
                file = file .. '\n'
            else
                file = file .. 'static META_FUNCTION(sdl' .. meta.name .. ', ' .. func.name .. ') {\n'
                if func.ccall then
                    file = file .. func.ccall
                end
                file = file .. '}\n\n'
            end
        end

        file = file .. 'BEGIN_META(sdl' .. meta.name .. ') {\n'
        file = file .. '    BEGIN_REG(reg)\n'
        for i,func in ipairs(functions) do
            if func.name ~= 'break' then
                file = file .. '        REG_META_FIELD(sdl' .. meta.name .. ', ' .. func.name .. '),\n'
            end
        end
        file = file .. '    END_REG()\n'
        file = file .. '    luaL_newmetatable(L, "sdl' .. meta.name .. '");\n'
        file = file .. '    luaL_setfuncs(L, reg, 0);\n'
        file = file .. '    lua_pushvalue(L, -1);\n'
        file = file .. '    lua_setfield(L, -2, "__index");\n'
        
        file = file .. '    return 1;\n}\n'

        io.writefile(_MAIN_SCRIPT_DIR .. '/' .. meta.name .. '.c', file)
    end
end

function generate_emmy()
    print ("Generating selene_sdl.lua")
local file =
[[
--- @meta
--- @class sdl
]]

    for i,enum in pairs(sdl.enums) do
        file = file .. '--- @field ' .. enum .. ' string\n'
    end
    file = file .. 'sdl = {}\n'

    for _,block in ipairs(sdl.functions.lib.blocks) do
        if block.name then
            file = file .. '-- ' .. block.name .. '\n'
        end
        for i,func in ipairs(block.functions) do
            if func.name == 'break' then
                file = file .. '\n'
            else
                for j,a in ipairs(func.args) do
                    file = file .. '--- @param ' .. a.name .. ' ' .. a.type .. '\n'
                end
                if func.returns then
                    file = file .. '--- @return '
                    for j,r in ipairs(func.returns) do
                        file = file .. r
                        if j ~= #func.returns then
                            file = file .. ', '
                        end
                    end
                    file = file .. '\n'
                end
                file = file .. 'function sdl.' .. func.name .. '('
                for j,a in ipairs(func.args) do
                    file = file .. a.name
                    if j ~= #func.args then
                        file = file .. ', '
                    end
                end
                file = file .. ') end\n'
            end
        end
    end

    file = file .. '\n'

    for _,meta in ipairs(sdl.functions.metas) do
        file = file .. '--- @class sdl' .. meta.name .. '\n'
        file = file .. 'local ' .. meta.name .. ' = {}\n'
        local functions = meta.functions or {}
        for i,func in ipairs(functions) do
            if func.name == 'break' then
                file = file .. '\n'
            else
                local args = func.args or {}
                for j,a in ipairs(args) do
                    file = file .. '--- @param ' .. a.name .. ' ' .. a.type .. '\n'
                end
                if func.returns and #func.returns > 0 then
                    file = file .. '--- @return '
                    for j,r in ipairs(func.returns) do
                        file = file .. r
                        if j ~= #func.returns then
                            file = file .. ', '
                        end
                    end
                    file = file .. '\n'
                end
                file = file .. 'function ' .. meta.name .. ':' .. func.name .. '('
                for j,a in ipairs(args) do
                    file = file .. a.name
                    if j ~= #func.args then
                        file = file .. ', '
                    end
                end
                file = file .. ') end\n'
            end
        end
        file = file .. '\n'
    end
    io.writefile(_MAIN_SCRIPT_DIR .. '/sdl.lua', file)
end


function generate_files()
    generate_header()
    generate_source()
    generate_emmy()
end

newaction {
    trigger = 'generate',
    description = 'generate SDL files',
    execute = generate_files
}
