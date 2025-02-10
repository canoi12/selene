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
    NEW_UDATA_ADD(Data, data, sizeof(Data)+size);
    lua_setfield(L, -2, "data");
    *data = size;
    memcpy(&(data[1]), pixels, size);
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

static MODULE_FUNCTION(image, from_memory) {
    INIT_ARG();
    CHECK_UDATA(Data, dt);
    int w, h, comp;
    OPT_INTEGER(req_comp, 4);
    stbi_uc* pixels = stbi_load_from_memory((stbi_uc const*)(&dt[1]), *dt, &w, &h, &comp, req_comp);
    if (pixels == NULL)
        return luaL_error(L, "[selene] failed to load image from memory");
    lua_newtable(L);
    const size_t size = w*h*req_comp;
    NEW_UDATA_ADD(Data, data, sizeof(Data)+size);
    lua_setfield(L, -2, "data");
    *data = size;
    memcpy(&data[1], pixels, *data);
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
