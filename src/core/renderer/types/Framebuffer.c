#include "renderer.h"

static inline int l_Framebuffer_create(lua_State* L) {
    Uint32 fbo;
    glGenFramebuffers(1, &fbo);
    return 1;
}

static inline int l_Framebuffer_destroy(lua_State* L) {
    CHECK_META(Framebuffer);
    if (self->handle != 0) glDeleteTextures(1, &(self->handle));
    self->handle = 0;
    return 0;
}

static inline int l_Framebuffer_attach_texture(lua_State* L) {
    CHECK_META(Framebuffer);
    CHECK_UDATA(Texture2D, tex);
    int attachment = luaL_optinteger(L, arg++, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, self->handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_2D, tex->handle, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

int l_Framebuffer_open_meta(lua_State* L) {
    luaL_newmetatable(L, FRAMEBUFFER_CLASS);
    luaL_Reg reg[] = {
        REG_FIELD(Framebuffer, create),
        REG_FIELD(Framebuffer, destroy),
        REG_FIELD(Framebuffer, attach_texture),
        { NULL, NULL }
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}