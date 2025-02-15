#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdint.h>

#define C_ARRAY_TYPE(type) \
static int l_ctypes_##type##__index(lua_State* L) {\
    type##_t* array = (type##_t*)luaL_checkudata(L, 1, #type"_t[]");\
    int i = (int) luaL_checkinteger(L, 2);\
    lua_pushinteger(L, array[i]);\
    return 1;\
}                          \
static int l_ctypes_##type##__newindex(lua_State* L) {\
    type##_t* array = (type##_t*)luaL_checkudata(L, 1, #type"_t[]");\
    int i = (int) luaL_checkinteger(L, 2);\
    if (lua_istable(L, 3)) {\
        size_t len = lua_rawlen(L, 3);\
        for (size_t j = 1; j <= len; j++) {\
            lua_rawgeti(L, 3, j);\
            if (!lua_isnumber(L, -1)) {\
                lua_pop(L, 1);\
                return luaL_error(L, "expected number at table index %d", (int)j);\
            }\
            array[i + j - 1] = (type##_t) luaL_checkinteger(L, -1);\
        }\
    }\
    else if(lua_isinteger(L, 3)) {\
        type##_t value = (type##_t) luaL_checkinteger(L, 3);\
        array[i] = value;\
    }\
    return 0;\
}                          \
static int l_ctypes_##type(lua_State* L) {         \
    int n = (int)luaL_checkinteger(L, 2);           \
    type##_t* arr = (type##_t*)lua_newuserdata(L, sizeof(*arr)*n);  \
    luaL_setmetatable(L, #type"_t[]");\
    return 1;\
}\
static int l_ctypes_##type##_meta(lua_State* L) {  \
    lua_newtable(L);\
    luaL_newmetatable(L, #type"_t[]");\
    lua_pushcfunction(L, l_ctypes_##type##__index);\
    lua_setfield(L, -2, "__index");\
    lua_pushcfunction(L, l_ctypes_##type##__newindex);\
    lua_setfield(L, -2, "__newindex");             \
    lua_setfield(L, -2, "meta");                   \
    lua_newtable(L);\
    lua_pushcfunction(L, l_ctypes_##type);         \
    lua_setfield(L, -2, "__call");\
    lua_setmetatable(L, -2);\
    lua_pushinteger(L, sizeof(type##_t));\
    lua_setfield(L, -2, "size");\
    return 1;\
}\
static void load_##type##_meta(lua_State* L) {\
    l_ctypes_##type##_meta(L);\
    lua_setfield(L, -2, #type);\
}

/* int8 */
C_ARRAY_TYPE(int8)
C_ARRAY_TYPE(uint8)
/* int16 */
C_ARRAY_TYPE(int16)
C_ARRAY_TYPE(uint16)
/* int32 */
C_ARRAY_TYPE(int32)
C_ARRAY_TYPE(uint32)

static int l_ctypes_float__index(lua_State* L) {
    float* array = (float*)luaL_checkudata(L, 1, "float_t[]");
    int i = (int) luaL_checkinteger(L, 2);
    lua_pushnumber(L, array[i]);
    return 1;
}

static int l_ctypes_float__newindex(lua_State* L) {
    float* array = (float*)luaL_checkudata(L, 1, "float_t[]");
    int i = (int) luaL_checkinteger(L, 2);
    if (lua_istable(L, 3)) {
        size_t len = lua_rawlen(L, 3);
        for (size_t j = 1; j <= len; j++) {
            lua_rawgeti(L, 3, j);
            if (!lua_isnumber(L, -1)) {
                lua_pop(L, 1); 
                return luaL_error(L, "expected number at table index %d", (int)j);
            }
            array[i + j - 1] = (float)lua_tonumber(L, -1);
            lua_pop(L, 1);
        }
    } else if (lua_type(L, 3) == LUA_TNUMBER) {
        array[i] = (float)lua_tonumber(L, 3);
    } else {
        return luaL_error(L, "invalid value");
    }
    return 0;
}

static inline int l_ctypes_float(lua_State* L) {
    int n = (int)luaL_checkinteger(L, 2);
    float* arr = (float*)lua_newuserdata(L, sizeof(*arr)*n);
    luaL_setmetatable(L, "float_t[]");
    return 1;
}

static int l_ctypes_float_meta(lua_State* L) {
    lua_newtable(L);
    luaL_newmetatable(L, "float_t[]");
    lua_pushcfunction(L, l_ctypes_float__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_ctypes_float__newindex);
    lua_setfield(L, -2, "__newindex");
    lua_setfield(L, -2, "meta");
    lua_newtable(L);
    lua_pushcfunction(L, l_ctypes_float);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    lua_pushinteger(L, sizeof(float));
    lua_setfield(L, -2, "size");
    return 1;
}



static void load_float_meta(lua_State* L) {
    l_ctypes_float_meta(L);
    lua_setfield(L, -2, "float");
}

int luaopen_ctypes(lua_State* L) {
    lua_newtable(L);
    load_int8_meta(L);
    load_uint8_meta(L);
    load_int16_meta(L);
    load_uint16_meta(L);
    load_int32_meta(L);
    load_uint32_meta(L);
    load_float_meta(L);
    return 1;
}