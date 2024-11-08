#include "selene.h"
#include "lua_helper.h"
#include "linmath.h"

static MODULE_FUNCTION(vec3, alloc) {
    // NEW_UDATA(Mat4, m);]
    float x, y, z;
    x = (float)luaL_optnumber(L, 1, 0.f);
    y = (float)luaL_optnumber(L, 2, 0.f);
    z = (float)luaL_optnumber(L, 3, 0.f);
    float* vec = (float*)lua_newuserdata(L, sizeof(vec3));
    luaL_setmetatable(L, "vec3");
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    return 1;
}

static MODULE_FUNCTION(vec3, add) {
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
    vec3_add(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec3, sub) {
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
    vec3_sub(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec3, scale) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    CHECK_NUMBER(float, scale);
    vec3_scale(v, a, scale);
    return 0;
}

static MODULE_FUNCTION(vec3, mul_inner) {
    INIT_ARG();
    const float* v = (const float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    PUSH_NUMBER(vec3_mul_inner(v, a));
    return 1;
}

static MODULE_FUNCTION(vec3, len) {
    INIT_ARG();
    const float* v = (const float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    PUSH_NUMBER(vec3_len(v));
    return 1;
}

static MODULE_FUNCTION(vec3, norm) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec3_norm(v, a);
    return 0;
}

static MODULE_FUNCTION(vec3, min) {
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
    vec3_min(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec3, max) {
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
    vec3_max(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec3, dup) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec3_dup(v, a);
    return 0;
}

static MODULE_FUNCTION(vec3, mul_cross) {
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
    vec3_mul_cross(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec3, reflect) {
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
    vec3_reflect(v, a, b);
    return 0;
}

static META_FUNCTION(vec3, index) {
    INIT_ARG();
    float* vec = (float*)luaL_checkudata(L, arg++, "vec3");
    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(index);
        if (index < 1 || index > 3)
            return luaL_argerror(L, arg, "the value must be at the range 1-3");
        PUSH_NUMBER(vec[index-1]);
        return 1;
    }
    return 0;
}

static META_FUNCTION(vec3, newindex) {
    INIT_ARG();
    float* vec = (float*)luaL_checkudata(L, arg++, "vec3");
    CHECK_INTEGER(index);
    if (index < 1 || index > 3)
        return luaL_argerror(L, arg, "the value must be at the range 1-3");
    CHECK_NUMBER(float, value);
    vec[index-1] = value;
    return 0;
}

BEGIN_META(vec3) {
    BEGIN_REG(reg)
        REG_FIELD(vec3, alloc),
        REG_FIELD(vec3, add),
        REG_FIELD(vec3, sub),
        REG_FIELD(vec3, scale),
        REG_FIELD(vec3, mul_inner),
        REG_FIELD(vec3, len),
        REG_FIELD(vec3, norm),
        REG_FIELD(vec3, min),
        REG_FIELD(vec3, max),
        REG_FIELD(vec3, dup),
        REG_FIELD(vec3, mul_cross),
        REG_FIELD(vec3, reflect),
    END_REG()
    luaL_newlib(L, reg);
    luaL_newmetatable(L, "vec3");
    lua_pushcfunction(L, l_vec3__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_vec3__newindex);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    return 1;
}