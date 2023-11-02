#include "selene.h"
#include "lua_helper.h"

MODULE_FUNCTION(Data, create) {
    INIT_ARG();
    CHECK_INTEGER(size);
    NEW_UDATA(Data, data);
    data->size = size;
    data->data = malloc(size);
    return 1;
}

META_FUNCTION(Data, realloc) {
    GET_META(Data);
    CHECK_INTEGER(size);
    self->size = size;
    self->data = realloc(self->data, size);
    return 0;
}

META_FUNCTION(Data, free) {
    GET_META(Data);
    if (self->data) {
        free(self->data);
        self->data = NULL;
        self->size = 0;
    }
    return 1;
}

META_FUNCTION(Data, getSize) {
    GET_META(Data);
    PUSH_INTEGER(self->size);
    return 1;
}

META_FUNCTION(Data, getPointer) {
    GET_META(Data);
    OPT_INTEGER(offset, 0);
    char* data = (char*)self->data + offset;
    PUSH_LUDATA(data);
    return 1;
}

META_FUNCTION(Data, getString) {
    GET_META(Data);
    lua_pushlstring(L, self->data, self->size);
    return 1;
}

META_FUNCTION(Data, readBytes) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    char* data = (char*)self->data + offset;
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
    return n;
}

META_FUNCTION(Data, readInts) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    int* data = (int*)((char*)self->data + offset);
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
    return n;
}

META_FUNCTION(Data, readFloats) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    float* data = (float*)((char*)self->data + offset);
    for (int i = 0; i < n; i++)
        PUSH_NUMBER(data[i]);
    return n;
}

META_FUNCTION(Data, writeBytes) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset + ((args - 2) * sizeof(char))) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    char* data = (char*)self->data + offset;
    while (arg <= args) {
        CHECK_NUMBER(char, value);
        *data = value;
        data++;
    }
    PUSH_INTEGER(offset + (args * sizeof(char)));
    return 1;
}

META_FUNCTION(Data, writeInts) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset + ((args - 2) * sizeof(int))) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    int* data = (int*)((char*)self->data + offset);
    while (arg <= args) {
        CHECK_INTEGER(value);
        *data = value;
        data++;
    }
    PUSH_INTEGER(offset + (args * sizeof(int)));
    return 1;
}

META_FUNCTION(Data, writeFloats) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset + ((args - 2) * sizeof(float))) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    float* data = (float*)((char*)self->data + offset);
    while (arg <= args) {
        CHECK_NUMBER(float, value);
        *data = value;
        data++;
    }
    PUSH_INTEGER(offset + (args * sizeof(float)));
    return 1;
}

META_FUNCTION(Data, writeString) {
    GET_META(Data);
    CHECK_INTEGER(offset);
    size_t size;
    const char* str = luaL_checklstring(L, arg++, &size);
    int error = (offset + size) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    char* data = (char*)self->data + offset;
    for (char* p = (char*)str; *p != '\0'; p++) {
        *data = *p;
        data++;
    }
    offset += size;
    PUSH_INTEGER(offset + size);
    return 1;
}

BEGIN_META(Data) {
    BEGIN_REG()
        REG_FIELD(Data, create),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Data, realloc),
        REG_META_FIELD(Data, free),
        REG_META_FIELD(Data, getSize),
        REG_META_FIELD(Data, getPointer),
        REG_META_FIELD(Data, getString),
        REG_META_FIELD(Data, readBytes),
        REG_META_FIELD(Data, readInts),
        REG_META_FIELD(Data, readFloats),
        REG_META_FIELD(Data, writeBytes),
        REG_META_FIELD(Data, writeInts),
        REG_META_FIELD(Data, writeFloats),
        REG_META_FIELD(Data, writeString),
    END_REG()
    NEW_META(Data, _reg, _index_reg);
    return 1;
}
