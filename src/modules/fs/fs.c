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
        REG_FIELD(fs, CreateDir),
        REG_FIELD(fs, RemoveDir),
        REG_FIELD(fs, Load),
    END_REG()
    luaL_newlib(LUA_STATE_NAME, _reg);
    return 1;
}