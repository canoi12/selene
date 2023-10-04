#include "selene.h"

#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

static int l_file__SeekSet(lua_State* L) {
    FILE** file = luaL_checkudata(L, 1, "File");
    int n = luaL_checkinteger(L, 2);
    fseek(*file, n, SEEK_SET);
    return 0;
}

static int l_file__SeekEnd(lua_State* L) {
    FILE** file = luaL_checkudata(L, 1, "File");
    fseek(*file, 0, SEEK_END);
    return 0;
}

static int l_file__SeekCur(lua_State* L) {
    FILE** file = luaL_checkudata(L, 1, "File");
    int n = luaL_checkinteger(L, 2);
    fseek(*file, n, SEEK_CUR);
    return 0;
}

static int l_file__Tell(lua_State* L) {
    FILE** file = luaL_checkudata(L, 1, "File");
    lua_pushnumber(L, ftell(*file));
    return 1;
}

static int l_file__Read(lua_State* L) {
    FILE** file = luaL_checkudata(L, 1, "File");
    return 1;
}

static int l_file__Write(lua_State* L) {
    FILE** file = luaL_checkudata(L, 1, "File");
    return 1;
}

static int l_file__Append(lua_State* L) {
    FILE** file = luaL_checkudata(L, 1, "File");
    return 1;
}

static int l_file_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"Read", l_file__Read},
        {"Write", l_file__Write},
        {"Append", l_file__Append},
        {"SeekSet", l_file__SeekSet},
        {"SeekEnd", l_file__SeekEnd},
        {"SeekCur", l_file__SeekCur},
        {"Tell", l_file__Tell},
//        {"decode_image", l_file__decode_image},
//        {"to_data", l_file__to_data},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "File");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static int l_fs_open(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    FILE** file = lua_newuserdata(L, sizeof(void*));
    luaL_setmetatable(L, "File");
    *file = fopen(path, "rb");
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
    FILE* fp = fopen(path, "rb");
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

	FILE* fp = fopen(path, "w");
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

    l_file_meta(L);
    lua_setfield(L, -2, "File");
    return 1;
}
