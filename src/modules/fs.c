#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

static MODULE_FUNCTION(fs, exists) {
    struct stat info;
    INIT_ARG();
    CHECK_STRING(path);
    int exists = 1;
    if (stat(path, &info) == -1)
        exists = 0;
    PUSH_BOOLEAN(exists);
    return 1;
}

static MODULE_FUNCTION(fs, read) {
    INIT_ARG();
    CHECK_STRING(path);
    #if defined(OS_WIN)
        FILE* fp;
        fopen_s(&fp, path, "rb");
    #else
        FILE* fp = fopen(path, "rb");
    #endif

    if (!fp)
        return luaL_error(L, "Failed to read file: %s\n", path);

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    NEW_UDATA_ADD(Data, d, sizeof(Data)+size);
    d[0] = size;
    fread(&(d[1]), 1, size, fp);
    fclose(fp);

    return 1;
}

static MODULE_FUNCTION(fs, read_text) {
    INIT_ARG();
    CHECK_STRING(path);
#if defined(OS_WIN)
    FILE* fp;
    fopen_s(&fp, path, "rb");
#else
    FILE* fp = fopen(path, "rb");
#endif

    if (!fp)
        return luaL_error(L, "Failed to read file: %s\n", path);

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* data = malloc(size+1);
    fread(data, 1, size, fp);
    data[size] = '\0';
    fclose(fp);

    lua_pushstring(L, data);
    free(data);

    return 1;
}

static MODULE_FUNCTION(fs, write) {
    INIT_ARG();
    size_t len;
    CHECK_STRING(path);
    CHECK_LSTRING(text, &len);
#if defined(OS_WIN)
    FILE* fp;
    fopen_s(&fp, path, "w");
#else
    FILE* fp = fopen(path, "w");
#endif
	if (!fp)
        return luaL_error(L, "failed to create file %s\n", path);
    fprintf(fp, "%*s", (int)len, text);
	fclose(fp);
    PUSH_BOOLEAN(1);
    return 1;
}

static MODULE_FUNCTION(fs, mkdir) {
    INIT_ARG();
    CHECK_STRING(path);
#if defined(OS_WIN)
    mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return 0;
}

static MODULE_FUNCTION(fs, rmdir) {
    INIT_ARG();
    CHECK_STRING(path);
    rmdir(path);
    return 0;
}

static MODULE_FUNCTION(fs, list) {
    INIT_ARG();
    CHECK_STRING(path);
#if defined(OS_WIN)
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char dirSpec[MAX_PATH];
    snprintf(dirSpec, MAX_PATH, "%s\\*", path);

    hFind = FindFirstFile(dirSpec, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return luaL_error(L, "Invalid file handle. Error is %u\n", GetLastError());
    } else {
        lua_newtable(L);
        int i = 1;
        do {
            const char *name = findFileData.cFileName;
            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                lua_pushstring(L, name);
                lua_rawseti(L, -2, i);
                i++;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#endif
    return 1;
}

static MODULE_FUNCTION(fs, load) {
    INIT_ARG();
    CHECK_STRING(path);
    luaL_loadfile(L, path);
    return 1;
}

int luaopen_fs(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(fs, exists),
        REG_FIELD(fs, read),
        REG_FIELD(fs, read_text),
        REG_FIELD(fs, write),
        REG_FIELD(fs, mkdir),
        REG_FIELD(fs, rmdir),
        REG_FIELD(fs, list),
        REG_FIELD(fs, load),
    END_REG()
    luaL_newlib(LUA_STATE_NAME, reg);
    return 1;
}
