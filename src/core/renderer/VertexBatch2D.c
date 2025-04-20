#include "selene_renderer.h"

static void batch_check_size(VertexBatch2D* v, int count) {
    if ((v->offset + count) < v->count) return;
    while ((v->offset+count) >= v->count) v->count *= 2;
    v->data = (Vertex2D*)realloc(v->data, sizeof(Vertex2D)*v->count);
    glBindBuffer(GL_ARRAY_BUFFER, v->vbo.handle);
    glBufferData(GL_ARRAY_BUFFER, v->count*sizeof(Vertex2D), v->data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void batch_push(VertexBatch2D* self, int count, Vertex2D* vertices) {
    self->check_size(self, count);
    Vertex2D* v = self->data + self->offset;
    memcpy(v, vertices, count*sizeof(*v));
    self->offset += count;
}

static void batch_send(VertexBatch2D* self, int start, size_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo.handle);
    glBufferSubData(GL_ARRAY_BUFFER, start, count*sizeof(Vertex2D), self->data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

MODULE_FUNCTION(VertexBatch2D, create) {
    INIT_ARG();
    size_t count = luaL_optinteger(L, arg++, 1024);
    NEW_UDATA(VertexBatch2D, vb);
    vb->offset = 0;
    vb->count = count;
    vb->data = (Vertex2D*)malloc(sizeof(Vertex2D)*count);
    // methods
    vb->check_size = batch_check_size;
    vb->push = batch_push;
    vb->send = batch_send;
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    glGenVertexArrays(1, &(vb->vao.handle));
#endif
    // create GL buffer
    glGenBuffers(1, &(vb->vbo.handle));
    glBindBuffer(GL_ARRAY_BUFFER, vb->vbo.handle);
    glBufferData(GL_ARRAY_BUFFER, count*sizeof(Vertex2D), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 1;
}

MODULE_FUNCTION(VertexBatch2D, _destroy) {
    CHECK_META(VertexBatch2D);
    if (self->data) free(self->data);
    self->data = NULL;
    if (self->vbo.handle > 0) glDeleteTextures(1, &(self->vbo.handle));
    self->vbo.handle = 0;
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    if (self->vao.handle > 0) glDeleteVertexArrays(1, &(self->vao.handle));
#endif
    self->vao.handle = 0;
    return 0;
}

static MODULE_FUNCTION(VertexBatch2D, _push) {
    CHECK_META(VertexBatch2D);
    if (lua_isuserdata(L, arg)) {
        Vertex2D* data = (Vertex2D*)lua_touserdata(L, arg++);
        int count = luaL_checkinteger(L, arg++);
        self->push(self, count, data);
    } else if (lua_isnumber(L, arg)) {
        if (self->offset+1 >= self->count) {
            self->count *= 2;
            self->data = realloc(self->data, self->count * sizeof(Vertex2D));
            glBindBuffer(GL_ARRAY_BUFFER, self->vbo.handle);
            glBufferData(GL_ARRAY_BUFFER, self->count*sizeof(Vertex2D), self->data, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        Vertex2D* v = self->data + self->offset;
        v->x = luaL_checknumber(L, arg++);
        v->y = luaL_checknumber(L, arg++);
        v->r = luaL_optnumber(L, arg++, 1.f);
        v->g = luaL_optnumber(L, arg++, 1.f);
        v->b = luaL_optnumber(L, arg++, 1.f);
        v->a = luaL_optnumber(L, arg++, 1.f);
        v->u = luaL_optnumber(L, arg++, 0.f);
        v->v = luaL_optnumber(L, arg++, 0.f);
        self->offset += 1;
    }
    return 0;
}

static MODULE_FUNCTION(VertexBatch2D, _send) {
    CHECK_META(VertexBatch2D);
    OPT_INTEGER(offset, 0);
    OPT_INTEGER(count, self->count);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo.handle);
    glBufferSubData(GL_ARRAY_BUFFER, offset, count*sizeof(Vertex2D), self->data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

int l_VertexBatch2D_meta(lua_State* L) {
    luaL_newmetatable(L, "VertexBatch2D");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg reg[] = {
        REG_FIELD(VertexBatch2D, create),
        REG_META_FIELD(VertexBatch2D, destroy),
        REG_META_FIELD(VertexBatch2D, push),
        REG_META_FIELD(VertexBatch2D, send),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
