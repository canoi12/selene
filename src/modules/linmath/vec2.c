#include "selene.h"
#include "lua_helper.h"
#include "linmath.h"

static MODULE_FUNCTION(vec2, alloc) {
    // NEW_UDATA(Mat4, m);]
    float x, y;
    x = (float)luaL_optnumber(L, 1, 0.f);
    y = (float)luaL_optnumber(L, 2, 0.f);
    float* vec = (float*)lua_newuserdata(L, sizeof(vec2));
    luaL_setmetatable(L, "vec2");
    vec[0] = x;
    vec[1] = y;
    return 1;
}

static MODULE_FUNCTION(vec2, add) {
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
    vec2_add(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec2, sub) {
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
    vec2_sub(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec2, scale) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    CHECK_NUMBER(float, scale);
    vec2_scale(v, a, scale);
    return 0;
}

static MODULE_FUNCTION(vec2, mul_inner) {
    INIT_ARG();
    const float* v = (const float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    PUSH_NUMBER(vec2_mul_inner(v, a));
    return 0;
}

static MODULE_FUNCTION(vec2, len) {
    INIT_ARG();
    const float* v = (const float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    PUSH_NUMBER(vec2_len(v));
    return 1;
}

static MODULE_FUNCTION(vec2, norm) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec2_norm(v, a);
    return 0;
}

static MODULE_FUNCTION(vec2, min) {
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
    vec2_min(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec2, max) {
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
    vec2_max(v, a, b);
    return 0;
}

static MODULE_FUNCTION(vec2, dup) {
    INIT_ARG();
    float* v = (float*)lua_touserdata(L, arg++);
    if (!v)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    const float* a = (const float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    vec2_dup(v, a);
    return 0;
}

static META_FUNCTION(vec2, index) {
    INIT_ARG();
    float* vec = (float*)luaL_checkudata(L, arg++, "vec2");
    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(index);
        if (index < 1 || index > 2)
            return luaL_argerror(L, arg, "the value must be at the range 1-2");
        PUSH_NUMBER(vec[index-1]);
        return 1;
    }
    CHECK_STRING(key);
    luaL_getmetatable(L, "vec2");
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    return 1;
}

static META_FUNCTION(vec2, newindex) {
    INIT_ARG();
    float* vec = luaL_checkudata(L, arg++, "vec2");
    CHECK_INTEGER(index);
    if (index < 1 || index > 2)
        return luaL_argerror(L, arg, "the value must be at the range 1-2");
    CHECK_NUMBER(float, value);
    vec[index-1] = value;
    return 0;
}

BEGIN_META(vec2) {
    BEGIN_REG(reg)
        REG_FIELD(vec2, alloc),
        REG_FIELD(vec2, add),
        REG_FIELD(vec2, sub),
        REG_FIELD(vec2, scale),
        REG_FIELD(vec2, mul_inner),
        REG_FIELD(vec2, len),
        REG_FIELD(vec2, norm),
        REG_FIELD(vec2, min),
        REG_FIELD(vec2, max),
        REG_FIELD(vec2, dup),
    END_REG()
    luaL_newlib(L, reg);
    luaL_newmetatable(L, "vec2");
    lua_pushcfunction(L, l_vec2__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_vec2__newindex);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    // NEW_META(vec2, _reg, _index_reg);
    return 1;
}