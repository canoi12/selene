#include "selene.h"
#include "lua_helper.h"

#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

static MODULE_FUNCTION(fs, Exists) {
    struct stat info;
    INIT_ARG();
    CHECK_STRING(path);
    int exists = 1;
    if (stat(path, &info) == -1)
        exists = 0;
    PUSH_BOOLEAN(exists);
    return 1;
}

static MODULE_FUNCTION(fs, Read) {
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
    char* data = malloc(size);
    fread(data, 1, size, fp);
    fclose(fp);

    NEW_UDATA(Data, d);
    d->data = data;
    d->size = size;
    return 1;
}

static MODULE_FUNCTION(fs, Write) {
    INIT_ARG();
    int len;
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
	fprintf(fp, "%*s", len, text);
	fclose(fp);
    PUSH_BOOLEAN(1);
    return 1;
}

static MODULE_FUNCTION(fs, CreateDir) {
    INIT_ARG();
    CHECK_STRING(path);
#if defined(OS_WIN)
    mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return 0;
}

static MODULE_FUNCTION(fs, RemoveDir) {
    INIT_ARG();
    CHECK_STRING(path);
    rmdir(path);
    return 0;
}

static MODULE_FUNCTION(fs, Load) {
    INIT_ARG();
    CHECK_STRING(path);
    luaL_loadfile(L, path);
    return 1;
}

BEGIN_MODULE(fs) {
    BEGIN_REG()
        REG_FIELD(fs, Exists),
        REG_FIELD(fs, Read),
        REG_FIELD(fs, Write),
        REG_FIELD(fs, CreateDir),
        REG_FIELD(fs, RemoveDir),
        REG_FIELD(fs, Load),
    END_REG()
    luaL_newlib(LUA_STATE_NAME, _reg);
    return 1;
}