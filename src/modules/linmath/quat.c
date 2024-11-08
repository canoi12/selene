#include "selene.h"
#include "lua_helper.h"
#include "linmath.h"

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

BEGIN_META(quat) {
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