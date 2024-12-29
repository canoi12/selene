#include "selene.h"
#include "lua_helper.h"
#include "linmath.h"

/************************
 #                      #
 #         Vec2         #
 #                      #
 ************************/

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

/************************
 #                      #
 #         Vec3         #
 #                      #
 ************************/

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

/************************
 #                      #
 #         Vec4         #
 #                      #
 ************************/

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

/************************
 #                      #
 #        Mat4x4        #
 #                      #
 ************************/

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

/************************
 #                      #
 #         Quat         #
 #                      #
 ************************/

static MODULE_FUNCTION(quat, create) {
    // NEW_UDATA(Mat4, m);]
    INIT_ARG();
    OPT_NUMBER(float, x, 0.f);
    OPT_NUMBER(float, y, 0.f);
    OPT_NUMBER(float, z, 0.f);
    OPT_NUMBER(float, w, 1.f);
    float* quat = (float*)lua_newuserdata(L, sizeof(quat));
    luaL_setmetatable(L, "quat");
    quat[0] = x;
    quat[1] = y;
    quat[2] = z;
    quat[3] = w;
    return 1;
}

static MODULE_FUNCTION(quat, alloc) {
    INIT_ARG();
    OPT_NUMBER(float, x, 0.f);
    OPT_NUMBER(float, y, 0.f);
    OPT_NUMBER(float, z, 0.f);
    OPT_NUMBER(float, w, 1.f);
    float* quat = (float*)lua_newuserdata(L, sizeof(quat));
    luaL_setmetatable(L, "quat");
    quat[0] = x;
    quat[1] = y;
    quat[2] = z;
    quat[3] = w;
    return 1;
}

static META_FUNCTION(quat, mul) {
    INIT_ARG();
    float* quat = (float*)lua_touserdata(L, arg++);
    if (!quat)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    float* a = (float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    float* b = (float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    quat_mul(quat, a, b);
    return 0;
}

static META_FUNCTION(quat, conj) {
    INIT_ARG();
    float* self = (float*)lua_touserdata(L, arg++);
    if (!self)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    float* other = (float*)lua_touserdata(L, arg++);
    if (!other)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    quat_conj(self, other);
    return 0;
}

static META_FUNCTION(quat, rotate) {
    INIT_ARG();
    float* self = (float*)lua_touserdata(L, arg++);
    if (!self)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    CHECK_NUMBER(float, angle);
    float* other = (float*)lua_touserdata(L, arg++);
    if (!other)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    quat_rotate(self, angle, other);
    return 0;
}

static META_FUNCTION(quat, mul_vec3) {
    INIT_ARG();
    float* q = (float*)lua_touserdata(L, arg++);
    if (!q)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    float* a = (float*)lua_touserdata(L, arg++);
    if (!a)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    float* b = (float*)lua_touserdata(L, arg++);
    if (!b)
        return luaL_argerror(L, arg-1, "must be a userdata or lightuserdata");
    quat_mul_vec3(q, (float* const)a, (float* const)b);
    return 0;
}

static META_FUNCTION(quat, index) {
    INIT_ARG();
    float_t* self = (float_t*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(index);
        if (index < 1 || index > 4)
            return luaL_argerror(L, arg, "the index must be at the range 1-4");
        lua_pushnumber(L, (float)self[index-1]);
        return 1;
    }
    return 1;
}

static META_FUNCTION(quat, newindex) {
    INIT_ARG();
    float* self = (float*)lua_touserdata(L, arg++);
    if (self == NULL)
        return luaL_argerror(L, arg-1, "must be an userdata or lightuserdata");
    CHECK_INTEGER(index);
    CHECK_NUMBER(float, value);
    if (index < 1 || index > 4)
        return luaL_argerror(L, arg, "the index must be at the range 1-4");
    self[index-1] = value;
    return 0;
}

static MODULE_FUNCTION(quat, meta) {
    BEGIN_REG(reg)
        REG_FIELD(quat, alloc),
        REG_META_FIELD(quat, mul),
        REG_META_FIELD(quat, conj),
        REG_META_FIELD(quat, rotate),
        REG_META_FIELD(quat, mul_vec3),
    END_REG()
    luaL_newlib(LUA_STATE_NAME, reg);
    luaL_newmetatable(L, "quat");
    lua_pushcfunction(L, l_quat__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_quat__newindex);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    return 1;
}

int luaopen_linmath(lua_State* L) {
    lua_newtable(L);
    LOAD_META(vec2);
    LOAD_META(vec3);
    LOAD_META(vec4);
    LOAD_META(mat4x4);
    LOAD_META(quat);
    return 1;
}