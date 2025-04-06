#include "selene_renderer.h"

/**
 * Texture2D
 */

 int l_Texture2D_create(lua_State* L) {
    INIT_ARG();
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int format = luaL_checkoption(L, arg++, "rgba", pixel_formats);
    Uint8* pixels = NULL;
    if (lua_isuserdata(L, arg) || lua_islightuserdata(L, arg)) {
        pixels = lua_touserdata(L, arg++);
    }
    Uint32 tex;
    glGenTextures(1, &tex);
    NEW_UDATA(Texture2D, texture);
    texture->handle = tex;
    texture->width = width;
    texture->height = height;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, pixel_formats_values[format], width, height, 0, pixel_formats_values[format], GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return 1;
}

int l_Texture2D_load(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(filename);
    int format = luaL_checkoption(L, arg++, "rgba", pixel_formats);
    SDL_RWops* rw = SDL_RWFromFile(filename, "rb");
    if (!rw) return luaL_error(L, "[selene] failed to load image: %s", filename);
    size_t size = SDL_RWsize(rw);
    void* dt = malloc(size);
    SDL_RWread(rw, dt, 1, size);
    SDL_RWclose(rw);
    int width, height, comp;
    int req_comp = STBI_rgb;
    if (format == 0) req_comp = STBI_rgb;
    else if (format == 1) req_comp = STBI_rgb_alpha;
    stbi_uc* pixels = stbi_load_from_memory((stbi_uc const*)dt, size, &width, &height, &comp, req_comp);
    free(dt);
    if (pixels == NULL)
        return luaL_error(L, "[selene] failed to load image from memory");
    Uint32 tex;
    glGenTextures(1, &tex);
    NEW_UDATA(Texture2D, texture);
    texture->handle = tex;
    texture->width = width;
    texture->height = height;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, pixel_formats_values[format], width, height, 0, pixel_formats_values[format], GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);
    return 1;
}

static inline int l_Texture2D_destroy(lua_State* L) {
    CHECK_META(Texture2D);
    if (self->handle != 0) glDeleteTextures(1, &(self->handle));
    self->handle = 0;
    return 0;
}

static inline int l_Texture2D_set_filter(lua_State* L) {
    CHECK_META(Texture2D);
    int filter_min = luaL_checkoption(L, arg++, "nearest", texture_filters);
    int filter_mag = luaL_checkoption(L, arg++, "nearest", texture_filters);
    glBindTexture(GL_TEXTURE_2D, self->handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_filters_values[filter_min]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_filters_values[filter_mag]);
    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}

int l_Texture2D_open_meta(lua_State* L) {
    luaL_newmetatable(L, TEXTURE2D_CLASS);
    const luaL_Reg reg[] = {
        REG_FIELD(Texture2D, create),
        REG_FIELD(Texture2D, destroy),
        REG_FIELD(Texture2D, set_filter),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 0;
}
