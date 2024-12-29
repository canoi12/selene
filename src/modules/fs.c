#include "selene.h"
#include "lua_helper.h"
#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

static MODULE_FUNCTION(fs, exists) {
    INIT_ARG();
    CHECK_STRING(path);
    int exists = 1;
    struct stat info;
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
    NEW_UDATA_ADD(Data, d, size);
    // d->root = (char*)(d + 1);
    d[0] = size;
    
    d++;
    fread(d, 1, size, fp);
    // fread(d->root, 1, size, fp);
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

int luaopen_fs(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(fs, exists),
        REG_FIELD(fs, read),
        REG_FIELD(fs, read_text),
        REG_FIELD(fs, write),
        REG_FIELD(fs, mkdir),
        REG_FIELD(fs, rmdir),
    END_REG()
    luaL_newlib(L, reg);
    return 1;
}
