#include "selene.h"

#define WRITE(type)\
CHECK_INTEGER(offset);\
int args = lua_gettop(L);\
int error = (offset + ((args - 2) * sizeof(type))) > self->size;\
if (error) return luaL_error(L, "Data overflow");\
type* data = (type*)((char*)self->data + offset);\
while (arg <= args) {\
    CHECK_NUMBER(type, value);\
    *data = value;\
    data++;\
}\
PUSH_INTEGER(offset + (args * sizeof(type)));

static BEGIN_FUNCTION(data, NewData)
    CHECK_INTEGER(size);
    NEW_UDATA(Data, data);
    data->size = size;
    data->data = malloc(size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, Realloc)
    CHECK_INTEGER(size);
    self->data = realloc(self->data, size);
    self->size = size;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, Free)
    if (self->data) {
        free(self->data);
        self->data = NULL;
        self->size = 0;
    }
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, GetSize)
    PUSH_INTEGER(self->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, GetPointer)
    PUSH_LUDATA(self->data);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, GetString)
    lua_pushlstring(L, self->data, self->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, ReadBytes)
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    char* data = (char*)self->data + offset;
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
END_FUNCTION(n)

static BEGIN_META_FUNCTION(Data, ReadInts)
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    int* data = (int*)((char*)self->data + offset);
    for (int i = 0; i < n; i++)
        PUSH_INTEGER(data[i]);
END_FUNCTION(n)

static BEGIN_META_FUNCTION(Data, ReadFloats)
    CHECK_INTEGER(offset);
    OPT_INTEGER(n, 1);
    if (offset < 0 || offset + n > self->size)
        return luaL_error(L, "Invalid memory access");
    float* data = (float*)((char*)self->data + offset);
    for (int i = 0; i < n; i++)
        PUSH_NUMBER(data[i]);
END_FUNCTION(n)

static BEGIN_META_FUNCTION(Data, WriteByte)
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
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteInt)
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
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteFloat)
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
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteString)
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
END_FUNCTION(1)

static BEGIN_META(Data)
    BEGIN_REG(Data)
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
    NEW_META(Data);
END_FUNCTION(1)

BEGIN_MODULE(data)
    BEGIN_REG(data)
        REG_FIELD(data, NewData),
    END_REG()
    NEW_MODULE(data);
    LOAD_META(Data);
END_MODULE(1)
