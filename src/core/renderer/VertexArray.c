#include "selene_renderer.h"

static int l_VertexArray_create(lua_State* L) {
    INIT_ARG();
    NEW_UDATA(VertexArray, vao);
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    glGenVertexArrays(1, &(vao->handle));
#endif
    return 0;
}

static int l_VertexArray__bind(lua_State* L) {
    INIT_ARG();
    TEST_UDATA(VertexArray, self);
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    if (!self) glBindVertexArray(0);
    else glBindVertexArray(self->handle);
#endif
    return 0;
}

int l_VertexArray_open_meta(lua_State* L) {
    luaL_newmetatable(L, "VertexArray");
    const luaL_Reg _reg[] = {
        REG_FIELD(VertexArray, create),
        REG_META_FIELD(VertexArray, bind),
        {NULL, NULL}
    };
    return 1;
}
