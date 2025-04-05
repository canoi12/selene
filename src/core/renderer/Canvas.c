#include "selene_renderer.h"

int l_Canvas_create(lua_State* L) {
    int width = (int)luaL_checkinteger(L, 1);
    int height = (int)luaL_checkinteger(L, 2);
    Canvas* canvas = (Canvas*)lua_newuserdata(L, sizeof(Canvas));
    luaL_setmetatable(L, CANVAS_CLASS);
    glGenFramebuffers(1, &canvas->fbo);
    glGenTextures(1, &canvas->texture.handle);
    canvas->texture.width = width;
    canvas->texture.height = height;
    glGenTextures(1, &canvas->depth);
    glBindTexture(GL_TEXTURE_2D, canvas->depth);
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, canvas->texture.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, canvas->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvas->texture.handle, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, canvas->depth, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

static int l_Canvas_destroy(lua_State* L) {
    CHECK_META(Canvas);
    if (self->fbo != 0) {
        glDeleteFramebuffers(1, &self->fbo);
        self->fbo = 0;
    }
    if (self->texture.handle != 0) {
        glDeleteTextures(1, &(self->texture.handle));
        self->texture.handle = 0;
    }
    return 0;
}

int l_Canvas_open_meta(lua_State* L) {
    luaL_newmetatable(L, CANVAS_CLASS);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_Reg reg[] = {
        REG_FIELD(Canvas, create),
        REG_FIELD(Canvas, destroy),
        { NULL, NULL }
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
