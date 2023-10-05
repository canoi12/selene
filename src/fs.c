#include "selene.h"

#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

typedef FILE* File;

static META_FUNCTION(File, SeekSet) {
    INIT_GET_UDATA(File, file);
    CHECK_INTEGER(n);
    fseek(*file, n, SEEK_SET);
    return 0;
}

static META_FUNCTION(File, SeekEnd) {
    INIT_GET_UDATA(File, file);
    fseek(*file, 0, SEEK_END);
    return 0;
}

static META_FUNCTION(File, SeekCur) {
    INIT_GET_UDATA(File, file);
    CHECK_INTEGER(n);
    fseek(*file, n, SEEK_CUR);
    return 0;
}

static META_FUNCTION(File, Tell) {
    INIT_GET_UDATA(File, file);
    PUSH_INTEGER(ftell(*file));
    return 1;
}

static META_FUNCTION(File, Read) {
    INIT_GET_UDATA(File, file);
    return 0;
}

static META_FUNCTION(File, Write) {
    INIT_GET_UDATA(File, file);
    return 0;
}

static META_FUNCTION(File, Append) {
    INIT_GET_UDATA(File, file);
    return 0;
}

BEGIN_REG(File)
    META_FIELD(File, SeekSet),
    META_FIELD(File, SeekEnd),
    META_FIELD(File, SeekCur),
    META_FIELD(File, Tell),
    META_FIELD(File, Read),
    META_FIELD(File, Write),
    META_FIELD(File, Append),
END_REG()
NEW_META(File)

static int l_fs_open(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    FILE** file = lua_newuserdata(L, sizeof(void*));
    luaL_setmetatable(L, "File");
    #if defined(OS_WIN)
        fopen_s(file, path, "rb");
    #else
        *file = fopen(path, "rb");
    #endif
    if (!(*file)) {
        return luaL_error(L, "Failed to load file");
    }
    return 1;
}

static int l_fs_exists(lua_State* L) {
    struct stat info;
    const char* path = luaL_checkstring(L, 1);
    int exists = 1;
    if (stat(path, &info) == -1) exists = 0;
    lua_pushboolean(L, exists);
    return 1;
}

static int l_fs_read(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    #if defined(OS_WIN)
        FILE* fp;
        fopen_s(&fp, path, "rb");
    #else
        FILE* fp = fopen(path, "rb");
    #endif
    
    if (!fp)
        return luaL_error(L, "Failed to read file: %s", path);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* data = malloc(size+1);
    data[size] = '\0';
    fread(data, 1, size+1, fp);
    fclose(fp);
    lua_pushstring(L, (const char*)data);
    lua_pushinteger(L, size);
    free(data);
    return 2;
}

static int l_fs_write(lua_State* L) {
	const char* path = luaL_checkstring(L, 1);
	const char* text = luaL_checkstring(L, 2);

    #if defined(OS_WIN)
        FILE* fp;
        fopen_s(&fp, path, "w");
    #else
	    FILE* fp = fopen(path, "w");
    #endif
	if (!fp)
        return luaL_error(L, "failed to create file %s\n", path);
	fprintf(fp, "%s", text);
	fclose(fp);
	lua_pushboolean(L, 1);
	return 1;
}

static int l_fs_mkdir(lua_State* L) {
	const char* path = luaL_checkstring(L, 1);
#if defined(OS_WIN)
    mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return 0;
}

static int l_fs_rmdir(lua_State* L) {
	const char* path = luaL_checkstring(L, 1);
    rmdir(path);
    return 0;
}

static int l_fs_load(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    luaL_loadfile(L, path);
    return 1;
}

int seleneopen_fs(lua_State* L) {
    luaL_Reg reg[] = {
        {"open", l_fs_open},
		{"exists", l_fs_exists},
		{"read", l_fs_read},
		{"write", l_fs_write},
		{"mkdir", l_fs_mkdir},
		{"rmdir", l_fs_rmdir},
        {"load", l_fs_load},
		{NULL, NULL}
    };
    luaL_newlib(L, reg);

    LOAD_META(File);
    return 1;
}
