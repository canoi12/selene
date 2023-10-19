#include "fs.h"

static MODULE_FUNCTION(File, Open) {
    INIT_ARG();
    CHECK_STRING(path);
    NEW_UDATA(File, file);
     #if defined(OS_WIN)
        fopen_s(&(file->handle), path, "rb");
    #else
        file->handle = fopen(path, "rb");
    #endif
    if (!file->handle)
        return luaL_error(L, "Failed to load file\n");
    fseek(file->handle, 0, SEEK_END);
    file->size = ftell(file->handle);
    fseek(file->handle, 0, SEEK_SET);
    return 1;
}

// Meta

static META_FUNCTION(File, Seek) {
    CHECK_META(File);
    CHECK_INTEGER(n);
    fseek(self->handle, n, SEEK_SET);
    return 0;
}

static META_FUNCTION(File, Tell) {
    CHECK_META(File);
    PUSH_INTEGER(ftell(self->handle));
    return 1;
}

static META_FUNCTION(File, Read) {
    CHECK_META(File);
    if (!self->handle)
        return luaL_error(L, "File is closed");
    int offset = ftell(self->handle);
    int type = lua_type(L, arg);
    void* data = NULL;
    size_t size;
    if (type == LUA_TUSERDATA) {
        CHECK_UDATA(Data, dt);
        data = dt->data;
        size = dt->size;
    } else {
        size = luaL_optinteger(L, arg++, self->size - offset);
    }
    if (size + offset > self->size)
        return luaL_error(L, "Read buffer exceed file size");
    
    if (data == NULL) {
        NEW_UDATA(Data, dt);
        dt->data = malloc(size);
        fread(dt->data, 1, size, self->handle);
        dt->size = size;
        fseek(self->handle, offset, SEEK_SET);
    }
    else
        PUSH_VALUE(arg);

    
    return 1;
}

static META_FUNCTION(File, Write) {
    CHECK_META(File);
    return 0;
}

static META_FUNCTION(File, Append) {
    CHECK_META(File);
    return 0;
}

static META_FUNCTION(File, GetSize) {
    CHECK_META(File);
    PUSH_INTEGER(self->size);
    return 1;
}

static META_FUNCTION(File, Close) {
    CHECK_META(File);
    fclose(self->handle);
    return 0;
}

BEGIN_META(File) {
    BEGIN_REG()
        REG_FIELD(File, Open),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(File, Seek),
        REG_META_FIELD(File, Tell),
        REG_META_FIELD(File, Read),
        REG_META_FIELD(File, Write),
        REG_META_FIELD(File, Append),
        REG_META_FIELD(File, GetSize),
        REG_META_FIELD(File, Close),
    END_REG()
    NEW_META(File, _reg, _index_reg);
    return 1;
}