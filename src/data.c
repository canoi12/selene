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

static BEGIN_META_FUNCTION(Data, GetSize)
    PUSH_INTEGER(self->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, GetPointer)
    PUSH_LUDATA(self->data);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, GetString)
    lua_pushlstring(L, self->data, self->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, Write)
    WRITE(char);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteInt)
    WRITE(int);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteFloat)
    WRITE(float);
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
        REG_META_FIELD(Data, GetSize),
        REG_META_FIELD(Data, GetPointer),
        REG_META_FIELD(Data, GetString),
        REG_META_FIELD(Data, Write),
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
