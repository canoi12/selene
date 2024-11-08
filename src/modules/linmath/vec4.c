#include "selene.h"
#include "lua_helper.h"
#include "linmath.h"

static MODULE_FUNCTION(vec4, alloc) {
    // NEW_UDATA(Mat4, m);]
    float x, y, z, w;
    x = (float)luaL_optnumber(L, 1, 0.f);
    y = (float)luaL_optnumber(L, 2, 0.f);
    z = (float)luaL_optnumber(L, 3, 0.f);
    w = (float)luaL_optnumber(L, 4, 0.f);
    float* vec = (float*)lua_newuserdata(L, sizeof(vec4));
    luaL_setmetatable(L, "vec4");
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = w;
    return 1;
}

static MODULE_FUNCTION(vec4, add) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* b = (const float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_add(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec4, sub) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* b = (const float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_sub(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec4, scale) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    CHECK_NUMBER(float, scale);
    vec4_scale(v, a, scale);
    return 0;
}

static MODULE_FUNCTION(vec4, mul_inner) {
    INIT_ARG();
    const float* v = (const float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    PUSH_NUMBER(vec4_mul_inner(v, a));
    return 1;
}

static MODULE_FUNCTION(vec4, len) {
    INIT_ARG();
    const float* v = (const float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    PUSH_NUMBER(vec4_len(v));
    return 1;
}

static MODULE_FUNCTION(vec4, norm) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_norm(v, a);
    return 0;
}

static MODULE_FUNCTION(vec4, min) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* b = (const float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_min(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec4, max) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* b = (const float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_max(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec4, dup) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_dup(v, a);
    return 0;
}

static MODULE_FUNCTION(vec4, mul_cross) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* b = (const float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_mul_cross(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec4, reflect) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* b = (const float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec4_reflect(v, a, b);
    return 0;
}

static META_FUNCTION(vec4, index) {
    INIT_ARG();
    float* vec = (float*)luaL_checkudata(L, arg++, "vec4");
    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(index);
        if (index < 1 || index > 4)
            return luaL_argerror(L, arg, "the value must be at the range 1-4");
        PUSH_NUMBER(vec[index-1]);
        return 1;
    }
    return 0;
}

static META_FUNCTION(vec4, newindex) {
    INIT_ARG();
    float* vec = (float*)luaL_checkudata(L, arg++, "vec4");
    CHECK_INTEGER(index);
    if (index < 1 || index > 4)
        return luaL_argerror(L, arg, "the value must be at the range 1-4");
    CHECK_NUMBER(float, value);
    vec[index-1] = value;
    return 0;
}

BEGIN_META(vec4) {
    BEGIN_REG(reg)
        REG_FIELD(vec4, alloc),
        REG_FIELD(vec4, add),
        REG_FIELD(vec4, sub),
        REG_FIELD(vec4, scale),
        REG_FIELD(vec4, mul_inner),
        REG_FIELD(vec4, len),
        REG_FIELD(vec4, norm),
        REG_FIELD(vec4, min),
        REG_FIELD(vec4, max),
        REG_FIELD(vec4, dup),
        REG_FIELD(vec4, mul_cross),
        REG_FIELD(vec4, reflect),
    END_REG()
    luaL_newlib(L, reg);
    luaL_newmetatable(L, "vec4");
    lua_pushcfunction(L, l_vec4__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_vec4__newindex);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    return 1;
}