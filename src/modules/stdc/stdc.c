#include "stdc.h"

static MODULE_FUNCTION(stdc, malloc) {
    INIT_ARG();
    CHECK_INTEGER(size);
    void* data = malloc(size);
    if (!data)
        return luaL_error(L, "Failed to alloc memory\n");
    PUSH_LUDATA(data);
    return 1;
}

static MODULE_FUNCTION(stdc, realloc) {
    INIT_ARG();
    GET_LUDATA(void, data);
    CHECK_INTEGER(size);
    data = realloc(data, size);
    PUSH_LUDATA(data);
    return 1;
}

static MODULE_FUNCTION(stdc, free) {
    INIT_ARG();
    GET_LUDATA(void, data);
    free(data);
    return 0;
}

static MODULE_FUNCTION(stdc, write_byte) {
    INIT_ARG();
    GET_LUDATA(void, data);
    OPT_INTEGER(offset, 0);
    char* d = (char*)data + offset;
    return 0;
}

BEGIN_MODULE(stdc) {
    BEGIN_REG()
        REG_FIELD(stdc, malloc),
        REG_FIELD(stdc, realloc),
        REG_FIELD(stdc, free),
    END_REG()
    luaL_newlib(L, _reg);
    return 1;
}
