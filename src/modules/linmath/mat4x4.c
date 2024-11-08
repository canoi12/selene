#include "selene.h"
#include "lua_helper.h"
#include "linmath.h"

static MODULE_FUNCTION(mat4x4, create) {
    // NEW_UDATA(Mat4, m);
    float* mat = (float*)lua_newuserdata(L, sizeof(mat4x4));
    luaL_setmetatable(L, "mat4x4");
    mat4x4_identity((vec4*)mat);
    return 1;
}

static MODULE_FUNCTION(mat4x4, alloc) {
    float* mat = (float*)lua_newuserdata(L, sizeof(mat4x4));
    luaL_setmetatable(L, "mat4x4");
    return 1;
}

static MODULE_FUNCTION(mat4x4, identity) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (!self)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    mat4x4_identity(self);
    return 0;
}

static MODULE_FUNCTION(mat4x4, dup) {
    INIT_ARG();
    vec4* mat = (vec4*)lua_touserdata(L, arg++);
    if (!mat)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    vec4* other = (vec4*)lua_touserdata(L, arg++);
    if (!other)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    mat4x4_dup(mat, (vec4* const)other);
    return 0;
}

static MODULE_FUNCTION(mat4x4, transpose) {
    INIT_ARG();
    vec4* mat = lua_touserdata(L, arg++);
    if (!mat)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    vec4* other = (vec4*)lua_touserdata(L, arg++);
    if (!other)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    mat4x4_transpose(mat, (vec4* const)other);
    return 0;
}

static MODULE_FUNCTION(mat4x4, translate_in_place) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_translate_in_place(self, x, y, z);
    return 0;
}

static MODULE_FUNCTION(mat4x4, translate) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_translate(self, x, y, z);
    return 0;
}

static MODULE_FUNCTION(mat4x4, scale) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    vec4* other = (vec4*)lua_touserdata(L, arg++);
    if (other == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, k);
    mat4x4_scale(self, (vec4* const)other, k);
    return 0;
}

static MODULE_FUNCTION(mat4x4, scale_aniso) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    const vec4* other = (const vec4*)lua_touserdata(L, arg++);
    if (other == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_scale_aniso(self, other, x, y, z);
    return 0;
}

static MODULE_FUNCTION(mat4x4, mul) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    vec4* a = (vec4*)lua_touserdata(L, arg++);
    if (a == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    vec4* b = (vec4*)lua_touserdata(L, arg++);
    if (b == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    mat4x4_mul((vec4*)self, (vec4* const)a, (vec4* const)b);
    return 0;
}

static MODULE_FUNCTION(mat4x4, rotate) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    const vec4* a = (const vec4*)lua_touserdata(L, arg++);
    if (a == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    CHECK_NUMBER(float, z);
    CHECK_NUMBER(float, angle);
    mat4x4_rotate(self, a, x, y, z, angle);
    return 0;
}

static MODULE_FUNCTION(mat4x4, ortho) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, vnear);
    CHECK_NUMBER(float, vfar);
    mat4x4_ortho(self, left, right, bottom, top, vnear, vfar);
    return 0;
}

static MODULE_FUNCTION(mat4x4, frustum) {
    // CHECK_META(Mat4);
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, vnear);
    CHECK_NUMBER(float, vfar);
    mat4x4_frustum(self, left, right, bottom, top, vnear, vfar);
    return 0;
}

static MODULE_FUNCTION(mat4x4, perspective) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_NUMBER(float, y_fov);
    CHECK_NUMBER(float, aspect);
    CHECK_NUMBER(float, n);
    CHECK_NUMBER(float, f);
    mat4x4_perspective(self, y_fov, aspect, n, f);
    return 0;
}

static MODULE_FUNCTION(mat4x4, look_at) {
    INIT_ARG();
    vec4* self = (vec4*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    if (lua_type(L, arg) != LUA_TTABLE) return luaL_argerror(L, arg, "Must be a table");
    float eye[3];
    for (int i = 0; i < 3; i++) {
        lua_rawgeti(L, arg, i+1);
        eye[i] = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    arg++;
    if (lua_type(L, arg) != LUA_TTABLE) return luaL_argerror(L, arg, "Must be a table");
    float center[3];
    for (int i = 0; i < 3; i++) {
        lua_rawgeti(L, arg, i+1);
        center[i] = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    arg++;
    if (lua_type(L, arg) != LUA_TTABLE) return luaL_argerror(L, arg, "Must be a table");
    float up[3];
    for (int i = 0; i < 3; i++) {
        lua_rawgeti(L, arg, i+1);
        up[i] = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    
    mat4x4_look_at((vec4*)self, eye, center, up);
    return 0;
}

static META_FUNCTION(mat4x4, index) {
    INIT_ARG();
    float* self = (float*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(index);
        if (index < 1 || index > 16)
            return luaL_argerror(L, arg, "the value must be at the range 1-16");
        lua_pushnumber(L, (float)self[index-1]);
        return 1;
    }
    return 1;
}

static META_FUNCTION(mat4x4, newindex) {
    INIT_ARG();
    float* self = (float*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_INTEGER(index);
    CHECK_NUMBER(float, value);
    if (index < 1 || index > 16)
        return luaL_argerror(L, arg, "the index must be at the range 1-16");
    self[index-1] = value;
    return 0;
}

BEGIN_META(mat4x4) {
    BEGIN_REG(reg)
        // REG_FIELD(mat4x4, create),
        REG_FIELD(mat4x4, alloc),
        REG_FIELD(mat4x4, identity),
        REG_FIELD(mat4x4, dup),
        REG_FIELD(mat4x4, transpose),
        REG_FIELD(mat4x4, scale),
        REG_FIELD(mat4x4, scale_aniso),
        REG_FIELD(mat4x4, mul),
        REG_FIELD(mat4x4, translate),
        REG_FIELD(mat4x4, translate_in_place),
        REG_FIELD(mat4x4, rotate),
        REG_FIELD(mat4x4, ortho),
        REG_FIELD(mat4x4, frustum),
        REG_FIELD(mat4x4, perspective),
        REG_FIELD(mat4x4, look_at),
    END_REG()
    luaL_newlib(LUA_STATE_NAME, reg);
    luaL_newmetatable(L, "mat4x4");
    lua_pushcfunction(L, l_mat4x4__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_mat4x4__newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
    // NEW_META(mat4x4, _reg, _index_reg);
    return 1;
}