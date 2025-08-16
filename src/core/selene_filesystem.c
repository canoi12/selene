#if 0
#include "selene.h"
#include "lua_helper.h"
#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

static int l_selene_File__read(lua_State* L) {
    CHECK_META(selene_File);
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetIOSize(self->handle);
#else
    size_t size = SDL_RWsize(self->handle);
#endif
    size = luaL_optinteger(L, arg++, size);

    luaL_Buffer buf;
    luaL_buffinit(L, &buf);
    void* buff_ptr = luaL_prepbuffsize(&buf, size);
#if defined(SELENE_USE_SDL3)
    SDL_ReadIO(self->handle, buff_ptr, size);
#else
    SDL_RWread(self->handle, buff_ptr, 1, size);
#endif
    luaL_addsize(&buf, size);
    luaL_pushresult(&buf);

    return 1;
}

static int l_selene_File__write(lua_State* L) {
    CHECK_META(selene_File);
    void* data = NULL;
    size_t size = 0;
    if (lua_isstring(L, arg))
        data = (void*)lua_tolstring(L, arg, &size);
    else if (lua_isuserdata(L, arg)) {
        data = lua_touserdata(L, arg++);
        size = luaL_checkinteger(L, arg);
    }
#if defined(SELENE_USE_SDL3)
    size_t res = SDL_WriteIO(self->handle, data, size);
#else
    size_t res = SDL_RWwrite(self->handle, data, 1, size);
#endif
    PUSH_BOOLEAN(res == size);
    return 1;
}

static int l_selene_File__data(lua_State* L) {
    CHECK_META(selene_File);
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetIOSize(self->handle);
#else
    size_t size = SDL_RWsize(self->handle);
#endif
    void* data = lua_newuserdata(L, size);
#if defined(SELENE_USE_SDL3)
    SDL_ReadIO(self->handle, data, size);
#else
    SDL_RWread(self->handle, data, 1, size);
#endif
    return 1;
}

static int l_selene_File__size(lua_State* L) {
    CHECK_META(selene_File);
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetIOSize(self->handle);
#else
    size_t size = SDL_RWsize(self->handle);
#endif
    PUSH_INTEGER(size);
    return 1;
}

const char* str_whence[] = {"set", "curr", "end", NULL};

static int l_selene_File__seek(lua_State* L) {
    CHECK_META(selene_File);
    CHECK_INTEGER(offset);
    int opt = luaL_checkoption(L, arg++, "set", str_whence);
#if defined(SELENE_USE_SDL3)
    Sint64 off = SDL_SeekIO(self->handle, offset, opt);
#else
    Sint64 off = SDL_RWseek(self->handle, offset, opt);
#endif
    PUSH_INTEGER(off);
    return 1;
}

static int l_selene_File__tell(lua_State* L) {
    CHECK_META(selene_File);
#if defined(SELENE_USE_SDL3)
    Sint64 tell = SDL_TellIO(self->handle);
#else
    Sint64 tell = SDL_RWtell(self->handle);
#endif
    PUSH_INTEGER(tell);
    return 1;
}

static int l_selene_File__close(lua_State* L) {
    CHECK_META(selene_File);
#if defined(SELENE_USE_SDL3)
    SDL_CloseIO(self->handle);
#else
    SDL_RWclose(self->handle);
#endif
    return 0;
}

static int l_selene_File_meta(lua_State* L) {
    const luaL_Reg reg[] = {
        REG_META_FIELD(selene_File, read),
        REG_META_FIELD(selene_File, write),
        REG_META_FIELD(selene_File, data),
        REG_META_FIELD(selene_File, size),
        REG_META_FIELD(selene_File, seek),
        REG_META_FIELD(selene_File, tell),
        REG_META_FIELD(selene_File, close),
        {NULL, NULL}
    };
    luaL_newmetatable(L, selene_File_METANAME);
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static MODULE_FUNCTION(filesystem, open) {
    const char* filename = luaL_checkstring(L, 1);
    const char* mode = luaL_optstring(L, 2, "rb");
#if defined(SELENE_USE_SDL3)
    SDL_IOStream *rw = SDL_IOFromFile(filename, mode);
#else
    SDL_RWops *rw = SDL_RWFromFile(filename, mode);
#endif
    selene_File* file = (selene_File*)lua_newuserdata(L, sizeof(*file));
    luaL_setmetatable(L, selene_File_METANAME);
    file->handle = rw;
    return 1;
}

static MODULE_FUNCTION(filesystem, load) {
    const char* filename = luaL_checkstring(L, 1);
#if defined(SELENE_USE_SDL3)
    SDL_IOStream *rw = SDL_IOFromFile(filename, "rb");
    size_t size = SDL_GetIOSize(rw);
#else
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
    size_t size = SDL_RWsize(rw);
#endif
    void* data = malloc(size);
#if defined(SELENE_USE_SDL3)
    SDL_ReadIO(rw, data, size);
    SDL_CloseIO(rw);
#else
    SDL_RWread(rw, data, 1, size);
    SDL_RWclose(rw);
#endif
    luaL_loadbuffer(L, data, size, filename);
    free(data);
    return 1;
}

static MODULE_FUNCTION(filesystem, file_exists) {
    INIT_ARG();
    CHECK_STRING(path);
    int exists = 1;
    struct stat info;
    if (stat(path, &info) == -1)
        exists = 0;
    PUSH_BOOLEAN(exists);
    return 1;
}

static MODULE_FUNCTION(filesystem, read) {
    INIT_ARG();
    CHECK_STRING(filename);
    int to_userdata = 0;
    if (lua_isboolean(L, arg)) to_userdata = lua_toboolean(L, arg);
#if defined(SELENE_USE_SDL3)
    SDL_IOStream *rw = SDL_IOFromFile(filename, "rb");
#else
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
#endif
    if (!rw)
        return luaL_error(L, "failed to read file %s: %s\n", filename, SDL_GetError());
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetIOSize(rw);
#else
    size_t size = SDL_RWsize(rw);
#endif
    if (to_userdata) {
        void* data = lua_newuserdata(L, size);
    #if defined(SELENE_USE_SDL3)
        SDL_ReadIO(rw, data, size);
        SDL_CloseIO(rw);
    #else
        SDL_RWread(rw, data, 1, size);
        SDL_RWclose(rw);
    #endif
        PUSH_INTEGER(size);
        return 2;
    }

    luaL_Buffer buf;
    luaL_buffinit(L, &buf);
    void* data = luaL_prepbuffsize(&buf, size);
    #if defined(SELENE_USE_SDL3)
        SDL_ReadIO(rw, data, size);
        SDL_CloseIO(rw);
    #else
        SDL_RWread(rw, data, 1, size);
        SDL_RWclose(rw);
    #endif
    luaL_addsize(&buf, size);
    luaL_pushresult(&buf);
    return 1;
}


static MODULE_FUNCTION(filesystem, write) {
    INIT_ARG();
    size_t len;
    CHECK_STRING(filename);
    void* data = NULL;
    size_t size = 0;
    if (lua_isstring(L, arg)) {
        data = (void*)lua_tolstring(L, arg, &size);
    } else if (lua_isuserdata(L, arg)) {
        data = lua_touserdata(L, arg++);
        size = luaL_checkinteger(L, arg);
    } else
        return luaL_argerror(L, arg, "must be a string or userdata");
#if defined(SELENE_USE_SDL3)
    SDL_IOStream *rw = SDL_IOFromFile(filename, "rb");
#else
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
#endif
    if (!rw)
        return luaL_error(L, "failed to create file %s\n", filename);
#if defined(SELENE_USE_SDL3)
    int res = SDL_WriteIO(rw, data, size);
    SDL_CloseIO(rw);
#else
    int res = SDL_RWwrite(rw, data, 1, size);
    SDL_RWclose(rw);
#endif
    PUSH_BOOLEAN(res == size);
    return 1;
}

static MODULE_FUNCTION(filesystem, mkdir) {
    INIT_ARG();
    CHECK_STRING(path);
#if defined(OS_WIN)
    mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return 0;
}

static MODULE_FUNCTION(filesystem, rmdir) {
    INIT_ARG();
    CHECK_STRING(path);
    rmdir(path);
    return 0;
}

int luaopen_filesystem(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(filesystem, file_exists),
        REG_FIELD(filesystem, open),
        REG_FIELD(filesystem, read),
        REG_FIELD(filesystem, write),
        REG_FIELD(filesystem, mkdir),
        REG_FIELD(filesystem, rmdir),
    END_REG()
    luaL_newlib(L, reg);
    l_selene_File_meta(L);
    lua_setfield(L, -2, "File");
    return 1;
}
#endif