#include "selene.h"
#include "lua_helper.h"

static MODULE_FUNCTION(Data, New) {
    INIT_ARG();
    CHECK_INTEGER(size);
    NEW_UDATA(Data, data);
    data->size = size;
    data->data = malloc(size);
    return 1;
}

static META_FUNCTION(Data, Realloc) {
    CHECK_META(Data);
    CHECK_INTEGER(size);
    self->size = size;
    self->data = realloc(self->data, size);
    return 0;
}

static META_FUNCTION(Data, Free) {
    CHECK_META(Data);
    if (self->data) {
        free(self->data);
        self->data = NULL;
        self->size = 0;
    }
    return 1;
}

static META_FUNCTION(Data, GetSize) {
    CHECK_META(Data);
    PUSH_INTEGER(self->size);
    return 1;
}

static META_FUNCTION(Data, GetPointer) {
    CHECK_META(Data);
    OPT_INTEGER(offset, 0);
    char* data = (char*)self->data + offset;
    PUSH_LUDATA(data);
    return 1;
}

static META_FUNCTION(Data, GetString) {
    CHECK_META(Data);
    lua_pushlstring(L, self->data, self->size);
    return 1;
}

static META_FUNCTION(Data, ReadBytes) {
    CHECK_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    char* data = (char*)self->data + offset;
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
    return n;
}

static META_FUNCTION(Data, ReadInts) {
    CHECK_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    int* data = (int*)((char*)self->data + offset);
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
    return n;
}

static META_FUNCTION(Data, ReadFloats) {
    CHECK_META(Data);
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    float* data = (float*)((char*)self->data + offset);
    for (int i = 0; i < n; i++)
        PUSH_NUMBER(data[i]);
    return n;
}

static META_FUNCTION(Data, WriteByte) {
    CHECK_META(Data);
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

static META_FUNCTION(Data, WriteInt) {
    CHECK_META(Data);
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

static META_FUNCTION(Data, WriteFloat) {
    CHECK_META(Data);
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

static META_FUNCTION(Data, WriteString) {
    CHECK_META(Data);
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
        REG_FIELD(Data, New),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Data, Realloc),
        REG_META_FIELD(Data, Free),
        REG_META_FIELD(Data, GetSize),
        REG_META_FIELD(Data, GetPointer),
        REG_META_FIELD(Data, GetString),
        REG_META_FIELD(Data, ReadBytes),
        REG_META_FIELD(Data, ReadInts),
        REG_META_FIELD(Data, ReadFloats),
        REG_META_FIELD(Data, WriteByte),
        REG_META_FIELD(Data, WriteInt),
        REG_META_FIELD(Data, WriteFloat),
        REG_META_FIELD(Data, WriteString),
    END_REG()
    NEW_META(Data, _reg, _index_reg);
    return 1;
}
