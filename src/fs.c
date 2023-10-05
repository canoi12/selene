#include "selene.h"

#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

typedef FILE* File;

static BEGIN_META_FUNCTION(File, SeekSet, file)
    CHECK_INTEGER(n);
    fseek(*file, n, SEEK_SET);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, SeekEnd, file)
    fseek(*file, 0, SEEK_END);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, SeekCur, file)
    CHECK_INTEGER(n);
    fseek(*file, n, SEEK_CUR);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, Tell, file)
    PUSH_INTEGER(ftell(*file));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(File, Read, file)
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, Write, file)
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, Append, file)
END_FUNCTION(0)

static BEGIN_META(File)
    BEGIN_REG(File)
        REG_META_FIELD(File, SeekSet),
        REG_META_FIELD(File, SeekEnd),
        REG_META_FIELD(File, SeekCur),
        REG_META_FIELD(File, Tell),
        REG_META_FIELD(File, Read),
        REG_META_FIELD(File, Write),
        REG_META_FIELD(File, Append),
    END_REG()
    NEW_META(File);
END_META(1)

static BEGIN_FUNCTION(fs, open)
    INIT_ARG();
    CHECK_STRING(path);
    NEW_UDATA(FILE, *file, sizeof(void*));
    #if defined(OS_WIN)
        fopen_s(file, path, "rb");
    #else
        *file = fopen(path, "rb");
    #endif
    if (!(*file)) {
        return luaL_error(L, "Failed to load file");
    }
END_FUNCTION(1)

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

BEGIN_MODULE(fs)
    BEGIN_REG(fs)
        REG_FIELD(fs, open),
        REG_FIELD(fs, exists),
        REG_FIELD(fs, read),
        REG_FIELD(fs, write),
        REG_FIELD(fs, mkdir),
        REG_FIELD(fs, rmdir),
        REG_FIELD(fs, load),
    END_REG()
    NEW_MODULE(fs);
    LOAD_META(File);
END_MODULE(1)