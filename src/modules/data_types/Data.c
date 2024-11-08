#include "selene.h"
#include "lua_helper.h"

META_INT_GET_FIELD(Data, size)
META_PTR_GET_FIELD(Data, char, root)

static BEGIN_META_REG(field_regs)
    META_REG_GET_FIELD(Data, size),
    META_REG_GET_FIELD(Data, root),
END_META_REG();

static MODULE_FUNCTION(Data, create) {
    INIT_ARG();
    CHECK_INTEGER(size);
    if (size < 1) return luaL_error(L, "[selene] invalid data size");
    TEST_UDATA(Data, copy);
    // NEW_UDATA(Data, data);
    // Data* data = lua_newuserdata(L, sizeof(*data) + size);
    NEW_UDATA_ADD(Data, data, size);
    luaL_setmetatable(L, "Data");
    data->size = size;
    // data->data = malloc(size);
    data->root = (char*)(data + 1);
    if (copy) {
        int copy_size = size > copy->size ? copy->size : size;
        memcpy(data->root, copy->root, copy_size);
    }
    return 1;
}

static MODULE_FUNCTION(Data, getSize) {
    GET_META(Data);
    PUSH_INTEGER(self->size);
    return 1;
}

static MODULE_FUNCTION(Data, getPointer) {
    GET_META(Data);
    OPT_INTEGER(offset, 0);
    char* data = (char*)self->root + offset;
    PUSH_LUDATA(data);
    return 1;
}

static MODULE_FUNCTION(Data, getString) {
    GET_META(Data);
    lua_pushlstring(L, self->root, self->size);
    return 1;
}

static MODULE_FUNCTION(Data, readBytes) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    char* data = (char*)self->root + offset;
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
    return n;
}

static MODULE_FUNCTION(Data, readInts) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    int* data = (int*)((char*)self->root + offset);
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
    return n;
}

static MODULE_FUNCTION(Data, readFloats) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    float* data = (float*)((char*)self->root + offset);
    for (int i = 0; i < n; i++)
        PUSH_NUMBER(data[i]);
    return n;
}

static MODULE_FUNCTION(Data, writeBytes) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset + ((args - 2) * sizeof(char))) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    char* data = (char*)self->root + offset;
    while (arg <= args) {
        CHECK_NUMBER(char, value);
        *data = value;
        data++;
    }
    PUSH_INTEGER(offset + (args * sizeof(char)));
    return 1;
}

static MODULE_FUNCTION(Data, writeInts) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset + ((args - 2) * sizeof(int))) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    int* data = (int*)((char*)self->root + offset);
    while (arg <= args) {
        CHECK_INTEGER(value);
        *data = value;
        data++;
    }
    PUSH_INTEGER(offset + ((args-2) * sizeof(int)));
    return 1;
}

static MODULE_FUNCTION(Data, writeFloats) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset + ((args - 2) * sizeof(float))) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    float* data = (float*)((char*)self->root + offset);
    while (arg <= args) {
        CHECK_NUMBER(float, value);
        *data = value;
        data++;
    }
    PUSH_INTEGER(offset + ((args-2) * sizeof(float)));
    return 1;
}

static MODULE_FUNCTION(Data, writeString) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    size_t size;
    const char* str = luaL_checklstring(L, arg++, &size);
    int error = (offset + size) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    char* data = (char*)self->root + offset;
    for (char* p = (char*)str; *p != '\0'; p++) {
        *data = *p;
        data++;
    }
    offset += (int)size;
    PUSH_INTEGER(offset + size);
    return 1;
}

META_FUNCTION(Data, index);
META_FUNCTION(Data, index) {
    CHECK_META(Data);

    if (lua_type(L, arg) == LUA_TNUMBER) {
        CHECK_INTEGER(index);
        lua_pushinteger(L, self->root[index]);
        return 1;
    }
    CHECK_STRING(key);

    lua_rawgetp(L, LUA_REGISTRYINDEX, l_Data__index);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_type(L, -1) != LUA_TNIL) {
        lua_pushvalue(L, 1);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK)
            return luaL_error(L, "[selene] failed to get Data field %s", key);
        return 1;
    }
    lua_pop(L, 1);

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
        CHECK_NUMBER(char, value);
        self->root[index] = value;
        return 0;
    }
    CHECK_STRING(key);

    lua_rawgetp(L, LUA_REGISTRYINDEX, l_Data__newindex);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_type(L, -1) != LUA_TNIL) {
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 3);
        if (lua_pcall(L, 2, 0, 0) != LUA_OK)
            return luaL_error(L, "[selene] failed to set Data field %s", key);
        return 1;
    }
    lua_pop(L, 1);
    return 0;
}

int luaopen_Data(lua_State* L) {
    luaL_newmetatable(L, "Data");
    lua_pushcfunction(L, l_Data__index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_Data__newindex);
    lua_setfield(L, -2, "__newindex");

    LOAD_META_FIELDS(Data, field_regs);

    BEGIN_REG(reg)
        REG_FIELD(Data, create),
        REG_FIELD(Data, getSize),
        REG_FIELD(Data, getPointer),
        REG_FIELD(Data, getString),
        REG_FIELD(Data, readBytes),
        REG_FIELD(Data, readInts),
        REG_FIELD(Data, readFloats),
        REG_FIELD(Data, writeBytes),
        REG_FIELD(Data, writeInts),
        REG_FIELD(Data, writeFloats),
        REG_FIELD(Data, writeString),
    END_REG()
    luaL_setfuncs(L, reg, 0);
    return 1;
}