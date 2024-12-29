#include "selene.h"
#include "lua_helper.h"

static MODULE_FUNCTION(Data, malloc) {
    INIT_ARG();
    CHECK_INTEGER(size);
    if (size < 1) return luaL_argerror(L, arg-1, "size must be greater than zero");
    TEST_UDATA(Data, copy);
    NEW_UDATA_ADD(Data, data, size);
    luaL_setmetatable(L, "Data");
    *data = size;
    if (copy) {
        int copy_size = size > *copy ? *copy : size;
        memcpy(data+1, copy+1, copy_size);
    }
    return 1;
}

static MODULE_FUNCTION(Data, size) {
    GET_META(Data);
    PUSH_INTEGER(*self);
    return 1;
}

static MODULE_FUNCTION(Data, root) {
    GET_META(Data);
    OPT_INTEGER(offset, 0);
    char* data = (char*)(&self[1]);
    PUSH_LUDATA(data+offset);
    return 1;
}

static MODULE_FUNCTION(Data, get_string) {
    GET_META(Data);
    lua_pushlstring(L, (const char*)(&self[1]), *self);
    return 1;
}

static MODULE_FUNCTION(Data, read_bytes) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || (offset + n) > self[0])
        return luaL_argerror(L, 1, "invalid offset");
    char* data = (char*)(&self[1]) + offset;
    for (int i = 0; i < n; i++)
         PUSH_INTEGER(data[i]);
    return n;
}

static MODULE_FUNCTION(Data, read_ints) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || (offset + (n*4)) > self[0])
        return luaL_argerror(L, 1, "invalid offset");
    int* data = (int*)((char*)(&self[1]) + offset);
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
    return n;
}

static MODULE_FUNCTION(Data, read_floats) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || (offset + (n*4)) > self[0])
        return luaL_argerror(L, 1, "invalid offset");
    float* data = (float*)((char*)(&self[1]) + offset);
    for (int i = 0; i < n; i++)
        PUSH_NUMBER(data[i]);
    return n;
}

#define WRITE_FUNCTION(type, name)\
static MODULE_FUNCTION(Data, name) {\
    GET_META(Data);\
    CHECK_INTEGER(offset);\
    int args = lua_gettop(L);\
    int error = (offset + ((args - 2) * sizeof(type))) > self[0];\
    if (error)\
        return luaL_error(L, "data overflow");\
    type* data = (type*)((char*)(&self[1]) + offset);\
    while (arg <= args) {\
        CHECK_NUMBER(type, value);\
        *data = value;\
        data++;\
    }\
    PUSH_INTEGER(offset + ((args-2) * sizeof(type)));\
    return 1;\
}

WRITE_FUNCTION(char, write_bytes);
WRITE_FUNCTION(int, write_ints);
WRITE_FUNCTION(float, write_floats);

static MODULE_FUNCTION(Data, write_string) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    size_t size;
    const char* str = luaL_checklstring(L, arg++, &size);
    int error = (offset + size) > self[1];
    if (error)
        return luaL_error(L, "data overflow");
    char* data = (char*)(&self[1]) + offset;
    for (char* p = (char*)str; *p != '\0'; p++) {
        *data = *p;
        data++;
    }
    PUSH_INTEGER(offset + size);
    return 1;
}

META_FUNCTION(Data, index);
META_FUNCTION(Data, index) {
    CHECK_META(Data);
    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(offset);
        if (offset < 0 || offset > self[0]-sizeof(float))
            return luaL_argerror(L, 1, "invalid offset");
        float* data = (float*)((char*)(&self[1]) + offset);
        lua_pushnumber(L, data[0]);
        return 1;
    }
    CHECK_STRING(key);

    luaL_getmetatable(L, "Data");
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    
    return 1;
}

META_FUNCTION(Data, newindex);
META_FUNCTION(Data, newindex) {
    CHECK_META(Data);
    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(index);
        CHECK_NUMBER(float, value);
        float* data = (float*)((char*)(&self[1]) + index);
        data[0] = value;
        return 0;
    }
    return 0;
}

int luaopen_Data(lua_State* L) {
    luaL_newmetatable(L, "Data");
    lua_pushcfunction(L, l_Data__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_Data__newindex);
    lua_setfield(L, -2, "__newindex");
    BEGIN_REG(reg)
        REG_FIELD(Data, malloc),
        REG_FIELD(Data, size),
        REG_FIELD(Data, root),
        REG_FIELD(Data, get_string),
        REG_FIELD(Data, read_bytes),
        REG_FIELD(Data, read_ints),
        REG_FIELD(Data, read_floats),
        REG_FIELD(Data, write_bytes),
        REG_FIELD(Data, write_ints),
        REG_FIELD(Data, write_floats),
        REG_FIELD(Data, write_string),
    END_REG()
    luaL_setfuncs(L, reg, 0);
    return 1;
}
