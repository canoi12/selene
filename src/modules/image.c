#ifndef SELENE_NO_IMAGE
#include "selene.h"
#include "lua_helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static MODULE_FUNCTION(image, from_file) {
    INIT_ARG();
    CHECK_STRING(filename);
    OPT_INTEGER(req_comp, 4);
    int w, h, comp;
    stbi_uc* pixels = stbi_load(filename, &w, &h, &comp, req_comp);
    if (pixels == NULL) return luaL_error(L, "[selene] failed to load image %s", filename);
    lua_newtable(L);
    const size_t size = w*h*req_comp;
    // NEW_UDATA_ADD(Data, data, size);
    Uint8* data = (Uint8*)lua_newuserdata(L, size);
    luaL_setmetatable(L, "uint8_t[]");
    lua_setfield(L, -2, "data");
    // *data = size;
    memcpy(data, pixels, size);
    stbi_image_free(pixels);

    lua_pushinteger(L, size);
    lua_setfield(L, -2, "size");
    lua_pushinteger(L, w);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, h);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, req_comp);
    lua_setfield(L, -2, "channels");

    char format;
    if (req_comp == STBI_rgb_alpha)
        format = SELENE_PIXEL_RGBA;
    else
        format = SELENE_PIXEL_RGB;
    lua_pushinteger(L, format);
    lua_setfield(L, -2, "format");

    return 1;
}

static MODULE_FUNCTION(image, from_memory) {
    INIT_ARG();
    // CHECK_UDATA(Data, dt);
    if (!lua_isuserdata(L, arg))
        return luaL_argerror(L, arg, "userdata expected");
    void* dt = lua_touserdata(L, arg++);
    CHECK_INTEGER(dt_size);
    int w, h, comp;
    OPT_INTEGER(req_comp, 4);
    stbi_uc* pixels = stbi_load_from_memory((stbi_uc const*)dt, dt_size, &w, &h, &comp, req_comp);
    if (pixels == NULL)
        return luaL_error(L, "[selene] failed to load image from memory");
    lua_newtable(L);
    const size_t size = w*h*req_comp;
    Uint8* data = (Uint8*)lua_newuserdata(L, size);
    luaL_setmetatable(L, "uint8_t[]");
    lua_setfield(L, -2, "data");
    memcpy(data, pixels, size);
    stbi_image_free(pixels);

    lua_pushinteger(L, w);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, h);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, req_comp);
    lua_setfield(L, -2, "channels");

    char format;
    if (req_comp == STBI_rgb_alpha)
        format = SELENE_PIXEL_RGBA;
    else
        format = SELENE_PIXEL_RGB;
    lua_pushinteger(L, format);
    lua_setfield(L, -2, "format");
    return 1;
}

int luaopen_image(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(image, from_file),
        REG_FIELD(image, from_memory),
    END_REG()
    luaL_newlib(L, reg);
    return 1;
}
#endif /* SELENE_NO_IMAGE */
