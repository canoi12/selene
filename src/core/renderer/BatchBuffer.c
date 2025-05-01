#include "selene_renderer.h"

static inline int s_check_buf_size(BatchBuffer* buf, int size) {
    if (buf->offset + size > buf->count) {
        fprintf(stdout, "Resizing BatchBuffer %p, old count: %d, new count: %d\n", buf, buf->count, size);
        while (buf->offset + size > buf->count) buf->count *= 2;
        buf->data = realloc(buf->data, buf->count*buf->stride);
    }
    return 0;
}

int l_BatchBuffer_create(lua_State* L) {
    INIT_ARG();
    CHECK_INTEGER(stride);
    CHECK_INTEGER(count);
    NEW_UDATA(BatchBuffer, buffer);
    buffer->offset = 0;
    buffer->stride = stride;
    buffer->count = count;
    buffer->data = malloc(stride * count);
    buffer->z = 0.f;
    buffer->color[0] = buffer->color[1] =
        buffer->color[2] = buffer->color[3] = 1.f;
    return 1;
}

int l_BatchBuffer__destroy(lua_State* L) {
    CHECK_META(BatchBuffer);
    free(self->data);
    return 0;
}

int l_BatchBuffer__get_offset(lua_State* L) {
    CHECK_META(BatchBuffer);
    lua_pushinteger(L, self->offset);
    return 1;
}

int l_BatchBuffer__get_count(lua_State* L) {
    CHECK_META(BatchBuffer);
    lua_pushinteger(L, self->count);
    return 1;
}

int l_BatchBuffer__get_stride(lua_State* L) {
    CHECK_META(BatchBuffer);
    lua_pushinteger(L, self->stride);
    return 1;
}

int l_BatchBuffer__get_data(lua_State* L) {
    CHECK_META(BatchBuffer);
    lua_pushlightuserdata(L, self->data);
    return 1;
}

int l_BatchBuffer__set_color(lua_State* L) {
    CHECK_META(BatchBuffer);
    for (int i = 0; i < 4; i++) self->color[i] = luaL_checknumber(L, arg++);
    return 0;
}

int l_BatchBuffer__set_z(lua_State* L) {
    CHECK_META(BatchBuffer);
    self->z = luaL_checknumber(L, arg++);
    return 0;
}

int l_BatchBuffer__reset(lua_State* L) {
    CHECK_META(BatchBuffer);
    self->offset = 0;
    return 0;
}

int l_BatchBuffer__push_vertex2d(lua_State* L) {
    CHECK_META(BatchBuffer);
    Vertex2D v;
    v.x = (float)luaL_checknumber(L, arg++);
    v.y = (float)luaL_checknumber(L, arg++);
    v.z = (float)luaL_checknumber(L, arg++);
    v.r = (float)luaL_checknumber(L, arg++);
    v.g = (float)luaL_checknumber(L, arg++);
    v.b = (float)luaL_checknumber(L, arg++);
    v.a = (float)luaL_checknumber(L, arg++);
    v.u = (float)luaL_checknumber(L, arg++);
    v.v = (float)luaL_checknumber(L, arg++);
    //fprintf(stdout, "%f %f %f %f %f %f %f %f %f\n", v.x, v.y, v.z, v.r, v.g, v.b, v.a, v.u, v.v);
    char* data = (char*)self->data + (self->stride*self->offset);
    memcpy(data, &v, sizeof(Vertex2D));
    self->offset += 1;
    return 0;
}

int l_BatchBuffer__push_line(lua_State* L) {
    CHECK_META(BatchBuffer);
    Vertex2D v[2];
    //memcpy(&(v), &(self->aux_vertex), sizeof(Vertex2D));
    v[0].x = (float)luaL_checknumber(L, arg++);
    v[0].y = (float)luaL_checknumber(L, arg++);
    v[0].z = self->z;
    v[1].x = (float)luaL_checknumber(L, arg++);
    v[1].y = (float)luaL_checknumber(L, arg++);
    v[1].z = self->z;

    v[0].r = v[1].r = self->color[0];
    v[0].g = v[1].g = self->color[1];
    v[0].b = v[1].b = self->color[2];
    v[0].a = v[1].a = self->color[3];

    v[0].u = v[0].v = v[1].u = v[1].v = 0.f;
    //fprintf(stdout, "%f %f %f %f %f %f %f %f %f\n", v.x, v.y, v.z, v.r, v.g, v.b, v.a, v.u, v.v);
    char* data = (char*)self->data + (self->stride*self->offset);
    memcpy(data, v, sizeof(v));
    self->offset += 2;
    return 0;
}

int l_BatchBuffer__push_line_triangle(lua_State* L) {
    CHECK_META(BatchBuffer);
    s_check_buf_size(self, 6);
    Vertex2D* v = (Vertex2D*)((char*)self->data + (self->offset+self->stride));
    for (int i = 0; i < 6; i++) {
        v[i].r = self->color[0];
        v[i].g = self->color[1];
        v[i].b = self->color[2];
        v[i].a = self->color[3];
    }
    v[0].x = (float)luaL_checknumber(L, arg++);
    v[0].y = (float)luaL_checknumber(L, arg++);
    v[1].x = (float)luaL_checknumber(L, arg++);
    v[1].y = (float)luaL_checknumber(L, arg++);

    v[2].x = v[1].x;
    v[2].y = v[1].y;
    v[3].x = (float)luaL_checknumber(L, arg++);
    v[3].y = (float)luaL_checknumber(L, arg++);

    v[4].x = v[0].x;
    v[4].y = v[0].y;
    v[5].x = v[3].x;
    v[5].y = v[3].y;

    v[0].z = v[1].z = v[2].z =
        v[3].z = v[4].z = v[5].z = self->z;
    // memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 3);
    self->offset += 6;
    return 0;
}

static int l_BatchBuffer__push_fill_triangle(lua_State* L) {
    CHECK_META(BatchBuffer);
    s_check_buf_size(self, 3);

    Vertex2D* v = (Vertex2D*)self->data;
    v += self->offset;
    memset(v, 0, self->stride*3);
    for (int i = 0; i < 3; i++) {
        v[i].r = self->color[0];
        v[i].g = self->color[1];
        v[i].b = self->color[2];
        v[i].a = self->color[3];
    }
    v[0].x = (float)luaL_checknumber(L, 2);
    v[0].y = (float)luaL_checknumber(L, 3);
    v[1].x = (float)luaL_checknumber(L, 4);
    v[1].y = (float)luaL_checknumber(L, 5);
    v[2].x = (float)luaL_checknumber(L, 6);
    v[2].y = (float)luaL_checknumber(L, 7);
    v[0].z = v[1].z = v[2].z = self->z;
    //memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 3);
    self->offset += 3;
    return 0;
}

static int l_BatchBuffer__push_line_rect(lua_State* L) {
    CHECK_META(BatchBuffer);
    s_check_buf_size(self, 8);
    Vertex2D* v = (Vertex2D*)((char*)self->data + (self->offset*self->stride));
    for (int i = 0; i < 8; i++) {
        v[i].r = self->color[0];
        v[i].g = self->color[1];
        v[i].b = self->color[2];
        v[i].a = self->color[3];
    }
    float x, y, w, h;
    x = (float)luaL_checknumber(L, 2);
    y = (float)luaL_checknumber(L, 3);
    w = (float)luaL_checknumber(L, 4);
    h = (float)luaL_checknumber(L, 5);
    int inv = 0;
    float yy = 0.f;
    if (lua_isboolean(L, 6)) {
        inv = lua_toboolean(L, 6);
        yy = 1.f;
    }
    // top
    v[0].x = x;
    v[0].y = y;
    v[0].u = 0.f;
    v[0].v = yy;

    v[1].x = x + w;
    v[1].y = y;
    v[1].u = 1.f;
    v[1].v = yy;

    // right
    v[2].x = x + w;
    v[2].y = y;
    v[2].u = 1.f;
    v[2].v = yy;

    v[3].x = x + w;
    v[3].y = y + h;
    v[3].u = 1.f;
    v[3].v = 1.f-yy;

    // bottom
    v[4].x = x + w;
    v[4].y = y + h;
    v[4].u = 1.f;
    v[4].v = 1.f-yy;

    v[5].x = x;
    v[5].y = y + h;
    v[5].u = 0.f;
    v[5].v = 1.f-yy;

    // left
    v[6].x = x;
    v[6].y = y + h;
    v[6].u = 0.f;
    v[6].v = 1.f-yy;

    v[7].x = x;
    v[7].y = y;
    v[7].u = 0.f;
    v[7].v = yy;

    self->offset += 8;
    return 0;
}

static int l_BatchBuffer__push_fill_rect(lua_State* L) {
    CHECK_META(BatchBuffer);
    s_check_buf_size(self, 6);
    Vertex2D* v = (char*)self->data + (self->offset*self->stride);
    for (int i = 0; i < 6; i++) {
        v[i].r = self->color[0];
        v[i].g = self->color[1];
        v[i].b = self->color[2];
        v[i].a = self->color[3];
    }
    if (lua_isnumber(L, arg)) {
        float x, y, w, h;
        x = (float)luaL_checknumber(L, 2);
        y = (float)luaL_checknumber(L, 3);
        w = (float)luaL_checknumber(L, 4);
        h = (float)luaL_checknumber(L, 5);
        int inv = 0;
        float yy = 0.f;
        if (lua_isboolean(L, 6)) {
            inv = lua_toboolean(L, 6);
            yy = 1.f;
        }
        v[0].x = x;
        v[0].y = y;
        v[0].u = 0.f;
        v[0].v = yy;

        v[1].x = x + w;
        v[1].y = y;
        v[1].u = 1.f;
        v[1].v = yy;

        v[2].x = x + w;
        v[2].y = y + h;
        v[2].u = 1.f;
        v[2].v = 1.f-yy;

        v[3].x = x;
        v[3].y = y;
        v[3].u = 0.f;
        v[3].v = yy;

        v[4].x = x;
        v[4].y = y + h;
        v[4].u = 0.f;
        v[4].v = 1.f-yy;

        v[5].x = x + w;
        v[5].y = y + h;
        v[5].u = 1.f;
        v[5].v = 1.f-yy;

        v[0].z = v[1].z = v[2].z =
            v[3].z = v[4].z = v[5].z = self->z;
    } else if (lua_istable(L, arg)) {
        float x, y, w, h;
        vec2 uv[4] = {
                {0.f, 0.f},
                {1.f, 0.f},
                {1.f, 1.f},
                {0.f, 1.f}
        };
        lua_rawgeti(L, arg, 1);
        x = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 2);
        y = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 3);
        w = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 4);
        h = (float)luaL_checknumber(L, -1);
        lua_pop(L, 4);

#if 0
        arg++;
        if (lua_istable(L, arg)) {
            SDL_FRect rect;
            lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
            Texture2D* tex = (Texture2D*)lua_touserdata(L, -1);
            lua_pop(L, 1);
            vec2 texel;
            texel[0] = 1.f / (float)tex->width;
            texel[1] = 1.f / (float)tex->height;
            // fprintf(stdout, "tex(%d) w(%d) h(%d)\n", tex->handle, tex->width, tex->height);

            lua_rawgeti(L, arg, 1);
            rect.x = texel[0] * (float)luaL_checknumber(L, -1);
            lua_rawgeti(L, arg, 2);
            rect.y = texel[1] * (float)luaL_checknumber(L, -1);
            lua_rawgeti(L, arg, 3);
            rect.w = texel[0] * (float)luaL_checknumber(L, -1);
            lua_rawgeti(L, arg, 4);
            rect.h = texel[1] * (float)luaL_checknumber(L, -1);
            lua_pop(L, 4);

            uv[0][0] = rect.x;
            uv[0][1] = rect.y;

            uv[1][0] = rect.x+rect.w;
            uv[1][1] = rect.y;

            uv[2][0] = rect.x+rect.w;
            uv[2][1] = rect.y+rect.h;

            uv[3][0] = rect.x;
            uv[3][1] = rect.y+rect.h;
        }
#endif

        v[0].x = x;
        v[0].y = y;
        v[0].u = uv[0][0];
        v[0].v = uv[0][1];

        v[1].x = x + w;
        v[1].y = y;
        v[1].u = uv[1][0];
        v[1].v = uv[1][1];

        v[2].x = x + w;
        v[2].y = y + h;
        v[2].u = uv[2][0];
        v[2].v = uv[2][1];

        v[3].x = x;
        v[3].y = y;
        v[3].u = uv[0][0];
        v[3].v = uv[0][1];

        v[4].x = x;
        v[4].y = y + h;
        v[4].u = uv[3][0];
        v[4].v = uv[3][1];

        v[5].x = x + w;
        v[5].y = y + h;
        v[5].u = uv[2][0];
        v[5].v = uv[2][1];

    } else return luaL_argerror(L, arg, "invalid argument, number or table expected");

    self->offset += 6;
    return 0;
}

int l_BatchBuffer__push_line_circle(lua_State* L) {
    BatchBuffer* self = (BatchBuffer*)lua_touserdata(L, 1);
    int arg = 2;
    float cx = (float)luaL_checknumber(L, arg++);
    float cy = (float)luaL_checknumber(L, arg++);
    float radius = (float)luaL_checknumber(L, arg++);
    int segments = (int)luaL_optinteger(L, arg++, 16);
    s_check_buf_size(self, 2 * segments);
    Vertex2D* v = (char*)self->data + (self->offset * self->stride);
    float inc = M_PI2 / (float)segments;
    for (int i = 0; i < segments; i++) {
        //memcpy(v, &self->aux_vertex, sizeof(Vertex2D));
        float angle = (float)i * inc;
        v[0].r = self->color[0];
        v[0].g = self->color[1];
        v[0].b = self->color[2];
        v[0].a = self->color[3];
        v[0].x = cx + (radius * cosf(angle));
        v[0].y = cy + (radius * sinf(angle));
        v[0].z = self->z;

        //memcpy(v + 1, &self->aux_vertex, sizeof(Vertex2D));
        angle = (float)(i + 1) * inc;
        v[1].r = self->color[0];
        v[1].g = self->color[1];
        v[1].b = self->color[2];
        v[1].a = self->color[3];
        v[1].x = cx + (radius * cosf(angle));
        v[1].y = cy + (radius * sinf(angle));
        v[1].z = self->z;

        v += 2;
    }
    self->offset += 2 * segments;
    return 0;
}

int l_BatchBuffer__push_fill_circle(lua_State* L) {
    BatchBuffer* self = (BatchBuffer*)lua_touserdata(L, 1);
    int arg = 2;
    float cx = (float)luaL_checknumber(L, arg++);
    float cy = (float)luaL_checknumber(L, arg++);
    float radius = (float)luaL_checknumber(L, arg++);
    int segments = (int)luaL_optinteger(L, arg++, 16);

    s_check_buf_size(self, 3 * segments);
    Vertex2D* v = (char*)self->data + (self->offset * self->stride);

    float inc = M_PI2 / (float)segments;
    for (int i = 0; i < segments; i++) {
        // memcpy(v, &self->aux_vertex, sizeof(Vertex2D));
        v[0].r = self->color[0];
        v[0].g = self->color[1];
        v[0].b = self->color[2];
        v[0].a = self->color[3];
        v[0].x = cx;
        v[0].y = cy;
        v[0].z = self->z;

        // memcpy(v + 1, &self->aux_vertex, sizeof(Vertex2D));
        float angle = (float)i * inc;
        v[1].r = self->color[0];
        v[1].g = self->color[1];
        v[1].b = self->color[2];
        v[1].a = self->color[3];
        v[1].x = cx + (radius * cosf(angle));
        v[1].y = cy + (radius * sinf(angle));
        v[1].z = self->z;

        // memcpy(v + 2, &self->aux_vertex, sizeof(Vertex2D));
        angle = (float)(i + 1) * inc;
        v[2].r = self->color[0];
        v[2].g = self->color[1];
        v[2].b = self->color[2];
        v[2].a = self->color[3];
        v[2].x = cx + (radius * cosf(angle));
        v[2].y = cy + (radius * sinf(angle));
        v[2].z = self->z;

        v += 3;
    }
    self->offset += 3 * segments;
    return 0;
}

int l_BatchBuffer__push_sprite(lua_State* L) {
    CHECK_META(BatchBuffer);
    CHECK_UDATA(Texture2D, tex);
    SDL_FRect src;
    if (lua_istable(L, arg)) {
        lua_rawgeti(L, arg, 1);
        src.x = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 2);
        src.y = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 3);
        src.w = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 4);
        src.h = (float)luaL_checknumber(L, -1);
        lua_pop(L, 4);
        arg++;
    } else {
        src.x = 0.f;
        src.y = 0.f;
        src.w = (float)tex->width;
        src.h = (float)tex->height;
        if (lua_isnil(L, arg)) arg++;
    }
    vec2 uv[4];
    SDL_FRect nrect;
    nrect.x = (1.f / (float)tex->width) * src.x;
    nrect.y = (1.f / (float)tex->height) * src.y;
    nrect.w = (1.f / (float)tex->width) * src.w;
    nrect.h = (1.f / (float)tex->height) * src.h;

    uv[0][0] = nrect.x;
    uv[0][1] = nrect.y;

    uv[1][0] = nrect.x + nrect.w;
    uv[1][1] = nrect.y;

    uv[2][0] = nrect.x + nrect.w;
    uv[2][1] = nrect.y + nrect.h;

    uv[3][0] = nrect.x;
    uv[3][1] = nrect.y + nrect.h;

    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);

    float angle = luaL_optnumber(L, arg++, 0.f);
    float sx = luaL_optnumber(L, arg++, 1.f);
    float sy = luaL_optnumber(L, arg++, 1.f);
    vec2 offset = { 0, 0 };
    if (lua_istable(L, arg)) {
        lua_rawgeti(L, arg, 1);
        offset[0] = sx * (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 2);
        offset[1] = sy * (float)luaL_checknumber(L, -1);
        lua_pop(L, 2);
    }
    arg++;
    int invert[2] = { 0, 0 };
    if (lua_istable(L, arg)) {
        lua_rawgeti(L, arg, 1);
        invert[0] = lua_toboolean(L, -1);
        lua_rawgeti(L, arg, 2);
        invert[1] = lua_toboolean(L, -1);
        lua_pop(L, 2);
        //fprintf(stdout, "invert: %d %d\n", invert[0], invert[1]);
        arg++;
    }

    vec2 pos[4];
    pos[0][0] = 0;
    pos[0][1] = 0;

    pos[1][0] = 1.f; //src.w;//(sx*src.w);
    pos[1][1] = 0;

    pos[2][0] = 1.f; //src.w;//(sx*src.w);
    pos[2][1] = 1.f; //src.h;//(sy*src.h);

    pos[3][0] = 0;
    pos[3][1] = 1.f; //src.h;//(sy*src.h);

#if 1
    mat4 mat = GLM_MAT4_IDENTITY_INIT;
    glm_rotate_z(mat, DEG2RAD(angle), mat);
    glm_translate(mat, (vec3) { -offset[0], -offset[1], 0.f });
    glm_scale(mat, (vec3) { sx* src.w, sy* src.h, 1.f });
#else
    mat3 mat = GLM_MAT3_IDENTITY_INIT;
    glm_scale2d(mat, (vec2) { src.w* sx, src.h* sy });
    //glm_translate2d(mat, (vec2){-offset[0], -offset[1]});
    glm_rotate2d(mat, DEG2RAD(angle));
    //glm_translate2d(mat, (vec2){90.f, 95.f});
#endif

    for (int i = 0; i < 4; i++) {
        vec3 p = { pos[i][0], pos[i][1], 0.f };
#if 1
        glm_mat4_mulv3(mat, p, 1.f, p);
#else
        glm_mat3_mulv(mat, p, p);
#endif
        pos[i][0] = x + p[0];
        pos[i][1] = y + p[1];
    }
    if (invert[0]) {
        const vec2 aux = { uv[0][0], uv[2][0] };
        uv[0][0] = uv[1][0];
        uv[1][0] = aux[0];
        uv[2][0] = uv[3][0];
        uv[3][0] = aux[1];
    }
    if (invert[1]) {
        const vec2 aux = { uv[0][1], uv[1][1] };
        uv[0][1] = uv[3][1];
        uv[3][1] = aux[0];
        uv[1][1] = uv[2][1];
        uv[2][1] = aux[1];
    }

    s_check_buf_size(self, 6);
    Vertex2D* v = (char*)self->data + (self->offset * self->stride);
    for (int i = 0; i < 6; i++) {
        v[i].r = self->color[0];
        v[i].g = self->color[1];
        v[i].b = self->color[2];
        v[i].a = self->color[3];
        v[i].z = self->z;
    }
    v[0].x = pos[0][0];
    v[0].y = pos[0][1];
    v[0].u = uv[0][0];
    v[0].v = uv[0][1];

    v[1].x = pos[1][0];
    v[1].y = pos[1][1];
    v[1].u = uv[1][0];
    v[1].v = uv[1][1];

    v[2].x = pos[2][0];
    v[2].y = pos[2][1];
    v[2].u = uv[2][0];
    v[2].v = uv[2][1];

    v[3].x = pos[0][0];
    v[3].y = pos[0][1];
    v[3].u = uv[0][0];
    v[3].v = uv[0][1];

    v[4].x = pos[3][0];
    v[4].y = pos[3][1];
    v[4].u = uv[3][0];
    v[4].v = uv[3][1];

    v[5].x = pos[2][0];
    v[5].y = pos[2][1];
    v[5].u = uv[2][0];
    v[5].v = uv[2][1];
    self->offset += 6;
    return 0;
}

static const Vertex2D s_cube_vertices[24] = {
    {-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f },
    { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
    {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },
    { 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },

    {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    {-0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    {-0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    {-0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },

    { 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.5f,  0.5f,  -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },

    { -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
    { -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },
    { 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },
    { 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
};
static const int s_cube_indices[] = {
    0, 1, 2,   0, 2, 3,      //-- front face
    4, 5, 6,   4, 6, 7,      //-- back face
    8, 9, 10,  8, 10, 11,    //-- left face
    12, 13, 14, 12, 14, 15,   //-- right face
    16, 17, 18, 16, 18, 19,   //-- top face
    20, 21, 22, 20, 22, 23,   //-- bottom face
};
static int l_BatchBuffer__push_fill_cube(lua_State* L) {
    CHECK_META(BatchBuffer);
    // self->batch->check_size(self->batch, 36);
    s_check_buf_size(self, 36);
    mat4 m = GLM_MAT4_IDENTITY_INIT;
    if (lua_istable(L, arg)) {
        vec3 pos;
        lua_rawgeti(L, arg, 1);
        pos[0] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 2);
        pos[1] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 3);
        pos[2] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm_translate(m, pos);

    }
    arg++;
    if (lua_istable(L, arg)) {
        vec3 rotate;
        lua_rawgeti(L, arg, 1);
        rotate[0] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 2);
        rotate[1] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 3);
        rotate[2] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        mat4 out;
        glm_mat4_copy(m, out);
        glm_rotate_x(out, rotate[0], m);
        glm_rotate_y(m, rotate[1], out);
        glm_rotate_z(out, rotate[2], m);
    }
    arg++;
    if (lua_istable(L, arg)) {
        vec3 scale;
        lua_rawgeti(L, arg, 1);
        scale[0] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 2);
        scale[1] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 3);
        scale[2] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm_scale(m, scale);
    }
    arg++;
    int is_white = 0;
    if (lua_isboolean(L, arg)) { is_white = 1; }
    Vertex2D* bv = (char*)self->data + (self->offset * self->stride);
    static const vec2 tex_uv[] = {
        {0.f, 0.f},
        {1.f, 0.f},
        {1.f, 1.f},
        {0.f, 1.f}
    };
    for (int i = 0; i < 36; i++) {
        const int index = s_cube_indices[i];
        const Vertex2D* v = s_cube_vertices + index;
        vec4 out;
        // vec3 in = {v->x, v->y, v->z};
        glm_mat4_mulv3(m, (float*)v, 1.f, out);
        memcpy(bv + i, v, sizeof(Vertex2D));
        bv[i].x = out[0];
        bv[i].y = out[1];
        bv[i].z = out[2];
        if (is_white) {
            bv[i].r = bv[i].g = bv[i].b = bv[i].a = 1.f;
        }
    }
    self->offset += 36;
    return 0;
}

int l_BatchBuffer__push_fill_sphere(lua_State* L) {
    BatchBuffer* self = (BatchBuffer*)lua_touserdata(L, 1);
    int arg = 2;
    float cx = (float)luaL_checknumber(L, arg++);
    float cy = (float)luaL_checknumber(L, arg++);
    float cz = (float)luaL_checknumber(L, arg++);
    float radius = (float)luaL_checknumber(L, arg++);
    int stacks = (int)luaL_optinteger(L, arg++, 16); // Número de pilhas (latitude)
    int slices = (int)luaL_optinteger(L, arg++, 16); // Número de fatias (longitude)

    // Verifique se há espaço suficiente no buffer
    int totalVertices = 6 * slices * stacks; // 6 vértices por face (2 triângulos)
    // s_check_buffer(L, self, totalVertices);
    s_check_buf_size(self, totalVertices);
    Vertex2D* v = (char*)self->data + (self->offset * self->stride);

    float stackStep = M_PI / (float)stacks; // Angle between stacks
    float sliceStep = 2.0f * M_PI / (float)slices; // Angle between slices

    for (int i = 0; i < stacks; i++) {
        float stackAngle1 = (float)i * stackStep;
        float stackAngle2 = stackAngle1 + stackStep;

        float v1 = 1.0f - (float)i / (float)stacks;
        float v2 = 1.0f - (float)(i + 1) / (float)stacks;

        for (int j = 0; j < slices; j++) {
            float sliceAngle1 = (float)j * sliceStep;
            float sliceAngle2 = sliceAngle1 + sliceStep;

            float u1 = (float)j / (float)slices;
            float u2 = (float)(j + 1) / (float)slices;

            // Vértices para o primeiro triângulo
            v[0].x = cx + radius * sinf(stackAngle1) * cosf(sliceAngle1);
            v[0].y = cy + radius * sinf(stackAngle1) * sinf(sliceAngle1);
            v[0].z = cz + radius * cosf(stackAngle1);
            v[0].r = self->color[0];
            v[0].g = self->color[1];
            v[0].b = self->color[2];
            v[0].a = self->color[3];
            v[0].u = u1;
            v[0].v = v1;

            v[1].x = cx + radius * sinf(stackAngle2) * cosf(sliceAngle1);
            v[1].y = cy + radius * sinf(stackAngle2) * sinf(sliceAngle1);
            v[1].z = cz + radius * cosf(stackAngle2);
            v[1].r = self->color[0];
            v[1].g = self->color[1];
            v[1].b = self->color[2];
            v[1].a = self->color[3];
            v[1].u = u1;
            v[1].v = v2;

            v[2].x = cx + radius * sinf(stackAngle2) * cosf(sliceAngle2);
            v[2].y = cy + radius * sinf(stackAngle2) * sinf(sliceAngle2);
            v[2].z = cz + radius * cosf(stackAngle2);
            v[2].r = self->color[0];
            v[2].g = self->color[1];
            v[2].b = self->color[2];
            v[2].a = self->color[3];
            v[2].u = u2;
            v[2].v = v2;

            // Vertices for the second triangle
            v[3].x = cx + radius * sinf(stackAngle1) * cosf(sliceAngle1);
            v[3].y = cy + radius * sinf(stackAngle1) * sinf(sliceAngle1);
            v[3].z = cz + radius * cosf(stackAngle1);
            v[3].r = self->color[0];
            v[3].g = self->color[1];
            v[3].b = self->color[2];
            v[3].a = self->color[3];
            v[3].u = u1;
            v[3].v = v1;

            v[4].x = cx + radius * sinf(stackAngle2) * cosf(sliceAngle2);
            v[4].y = cy + radius * sinf(stackAngle2) * sinf(sliceAngle2);
            v[4].z = cz + radius * cosf(stackAngle2);
            v[4].r = self->color[0];
            v[4].g = self->color[1];
            v[4].b = self->color[2];
            v[4].a = self->color[3];
            v[4].u = u2;
            v[4].v = v2;

            v[5].x = cx + radius * sinf(stackAngle1) * cosf(sliceAngle2);
            v[5].y = cy + radius * sinf(stackAngle1) * sinf(sliceAngle2);
            v[5].z = cz + radius * cosf(stackAngle1);
            v[5].r = self->color[0];
            v[5].g = self->color[1];
            v[5].b = self->color[2];
            v[5].a = self->color[3];
            v[5].u = u2;
            v[5].v = v1;

            v += 6;
        }
    }

    self->offset += totalVertices;
    return 0;
}

static int l_BatchBuffer__push_text(lua_State* L) {
    CHECK_META(BatchBuffer);
    CHECK_UDATA(Texture2D, tex);
    CHECK_UDATA(FontGlyph, glyphs);
    CHECK_STRING(text);
    OPT_NUMBER(float, x, 0.f);
    OPT_NUMBER(float, y, 0.f);
    float ox = x;
    float oy = y;

    float x0, y0;
    x0 = 0.f;
    y0 = 0.f;
    uint8_t* p = (uint8_t*)text;
    int w = tex->width;
    int h = tex->height;

    int len = 0;
    while (*p != 0) {
        int codepoint;
        int n = utf8_codepoint(p, &codepoint);
        p += n;
        len++;
    }
    p = (uint8_t*)text;
    s_check_buf_size(self, len * 6);
    Vertex2D* v = (char*)self->data + (self->offset * self->stride);
    while (*p != 0) {
        int codepoint;
        int n = utf8_codepoint(p, &codepoint);
        p += n;
        SDL_FRect dest;
        int pos[2];
        int src[4];
        char_rect(glyphs, codepoint, &x0, &y0, pos, src, 0, tex->height);
        dest.x = x + (float)pos[0];
        dest.y = y + (float)pos[1];
        dest.w = (float)src[2];
        dest.h = (float)src[3];
        //fprintf(stderr, "%c:%d", *p, *p);
        //fprintf(stdout, "src (%c): %d %d %d %d\n", *p, src[0], src[1], src[2], src[3]);

        SDL_FRect t;
        t.x = (float)src[0] / (float)tex->width;
        t.y = (float)src[1] / (float)tex->height;
        t.w = (float)src[2] / (float)tex->width;
        t.h = (float)src[3] / (float)tex->height;

        v[0].x = dest.x;
        v[0].y = dest.y;
        v[0].u = t.x;
        v[0].v = t.y;
        v[0].r = self->color[0];
        v[0].g = self->color[1];
        v[0].b = self->color[2];
        v[0].a = self->color[3];

        v[1].x = dest.x + dest.w;
        v[1].y = dest.y;
        v[1].u = t.x + t.w;
        v[1].v = t.y;
        v[1].r = self->color[0];
        v[1].g = self->color[1];
        v[1].b = self->color[2];
        v[1].a = self->color[3];

        v[2].x = dest.x + dest.w;
        v[2].y = dest.y + dest.h;
        v[2].u = t.x + t.w;
        v[2].v = t.y + t.h;
        v[2].r = self->color[0];
        v[2].g = self->color[1];
        v[2].b = self->color[2];
        v[2].a = self->color[3];

        v[3].x = dest.x;
        v[3].y = dest.y;
        v[3].u = t.x;
        v[3].v = t.y;
        v[3].r = self->color[0];
        v[3].g = self->color[1];
        v[3].b = self->color[2];
        v[3].a = self->color[3];

        v[4].x = dest.x + dest.w;
        v[4].y = dest.y + dest.h;
        v[4].u = t.x + t.w;
        v[4].v = t.y + t.h;
        v[4].r = self->color[0];
        v[4].g = self->color[1];
        v[4].b = self->color[2];
        v[4].a = self->color[3];

        v[5].x = dest.x;
        v[5].y = dest.y + dest.h;
        v[5].u = t.x;
        v[5].v = t.y + t.h;
        v[5].r = self->color[0];
        v[5].g = self->color[1];
        v[5].b = self->color[2];
        v[5].a = self->color[3];

        v[0].z = v[1].z = v[2].z =
            v[3].z = v[4].z = v[5].z = self->z;

        v += 6;
        self->offset += 6;
        x += src[2];
        //SDL_RenderCopyF(*self, *(font->texture), &t, &dest);
    }
    // fprintf(stderr, "\n");
    return 0;
}

int l_BatchBuffer_meta(lua_State* L) {
    luaL_newmetatable(L, "BatchBuffer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg _reg[] = {
            REG_META_FIELD(BatchBuffer, destroy),
            REG_META_FIELD(BatchBuffer, get_offset),
            REG_META_FIELD(BatchBuffer, get_count),
            REG_META_FIELD(BatchBuffer, get_stride),
            REG_META_FIELD(BatchBuffer, get_data),
            REG_META_FIELD(BatchBuffer, reset),
            REG_META_FIELD(BatchBuffer, set_color),
            REG_META_FIELD(BatchBuffer, set_z),
            REG_META_FIELD(BatchBuffer, push_vertex2d),
            REG_META_FIELD(BatchBuffer, push_line),
            REG_META_FIELD(BatchBuffer, push_line_triangle),
            REG_META_FIELD(BatchBuffer, push_fill_triangle),
            REG_META_FIELD(BatchBuffer, push_line_rect),
            REG_META_FIELD(BatchBuffer, push_fill_rect),
            REG_META_FIELD(BatchBuffer, push_line_circle),
            REG_META_FIELD(BatchBuffer, push_fill_circle),
            REG_META_FIELD(BatchBuffer, push_sprite),
            REG_META_FIELD(BatchBuffer, push_fill_cube),
            REG_META_FIELD(BatchBuffer, push_fill_sphere),
            REG_META_FIELD(BatchBuffer, push_text),
            {NULL, NULL}
    };
    luaL_setfuncs(L, _reg, 0);
    return 1;
}