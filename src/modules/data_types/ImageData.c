#include "common.h"
#include "selene.h"
#include "lua_helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

META_INT_GET_FIELD(ImageData, data_size)
META_INT_GET_FIELD(ImageData, width)
META_INT_GET_FIELD(ImageData, height)
META_PTR_GET_FIELD(ImageData, unsigned char, pixels)

static MODULE_FUNCTION(ImageData, load) {
    INIT_ARG();
    CHECK_STRING(filename);
    OPT_INTEGER(req_comp, 4);
    int w, h, comp;
#ifndef SELENE_NO_SDL
    SDL_RWops* fp = SDL_RWFromFile(filename, "rb");
    size_t data_size = SDL_RWsize(fp);
    void* dt = malloc(data_size);
    SDL_RWread(fp, dt, 1, data_size);
    stbi_uc* pixels = stbi_load_from_memory(dt, data_size, &w, &h, &comp, req_comp);
    free(dt);
    SDL_RWclose(fp);
#else
    stbi_uc* pixels = stbi_load(filename, &w, &h, &comp, req_comp);
#endif
    if (pixels == NULL) return luaL_error(L, "[selene] failed to load image %s", filename);
    NEW_UDATA_ADD(ImageData, data, w*h*req_comp);
    data->data_size = w * h * req_comp;
    data->pixels = (unsigned char*)(data + 1);
    data->width = w;
    data->height = h;
    data->channels = req_comp;
    memcpy(data->pixels, pixels, data->data_size);
    stbi_image_free(pixels);
    if (req_comp == STBI_rgb_alpha)
        data->pixel_format = SELENE_PIXEL_RGBA;
    else
        data->pixel_format = SELENE_PIXEL_RGB;
    return 1;
}

static MODULE_FUNCTION(ImageData, fromMemory) {
    INIT_ARG();
    CHECK_UDATA(Data, dt);
    int w, h, comp;
    OPT_INTEGER(req_comp, 4);
    stbi_uc* pixels = stbi_load_from_memory((stbi_uc const*)dt->root, dt->size, &w, &h, &comp, req_comp);
    if (pixels == NULL)
        return luaL_error(L, "[selene] failed to load image from memory");
    NEW_UDATA_ADD(ImageData, data, w*h*req_comp);
    data->data_size = w * h * req_comp;
    data->pixels = (unsigned char*)(data + 1);
    data->width = w;
    data->height = h;
    data->channels = req_comp;
    memcpy(data->pixels, pixels, data->data_size);
    stbi_image_free(pixels);
    if (req_comp == STBI_rgb_alpha)
        data->pixel_format = SELENE_PIXEL_RGBA;
    else
        data->pixel_format = SELENE_PIXEL_RGB;
    return 1;
}

static BEGIN_META_REG(field_regs)
    META_REG_GET_FIELD(ImageData, data_size),
    META_REG_GET_FIELD(ImageData, width),
    META_REG_GET_FIELD(ImageData, height),
    META_REG_GET_FIELD(ImageData, pixels),
END_META_REG()

META_FUNCTION(ImageData, index);
META_FUNCTION(ImageData, index) {
    CHECK_META(ImageData);
    CHECK_STRING(key);

    lua_rawgetp(L, LUA_REGISTRYINDEX, l_ImageData__index);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_type(L, -1) != LUA_TNIL) {
        lua_pushvalue(L, 1);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK)
            return luaL_error(L, "[selene] failed to get ImageData field %s", key);
        return 1;
    }
    lua_pop(L, 1);

    luaL_getmetatable(L, "ImageData");
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    
    return 1;
}

META_FUNCTION(ImageData, newindex);
META_FUNCTION(ImageData, newindex) {
    CHECK_META(ImageData);
    CHECK_STRING(key);

    lua_rawgetp(L, LUA_REGISTRYINDEX, l_ImageData__index);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_type(L, -1) != LUA_TNIL) {
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 3);
        if (lua_pcall(L, 2, 0, 0) != LUA_OK)
            return luaL_error(L, "[selene] failed to set ImageData field %s", key);
        return 0;
    }
    lua_pop(L, 1);
    return 0;
}

int luaopen_ImageData(lua_State* L) {
    luaL_newmetatable(L, "ImageData");
    lua_pushcfunction(L, l_ImageData__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_ImageData__newindex);
    lua_setfield(L, -2, "__newindex");
    LOAD_META_FIELDS(ImageData, field_regs);
    BEGIN_REG(reg)
        REG_FIELD(ImageData, load),
        REG_FIELD(ImageData, fromMemory),
    END_REG()
    luaL_setfuncs(L, reg, 0);
    return 1;
}