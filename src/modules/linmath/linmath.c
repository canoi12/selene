#include "common.h"
#include "lua_helper.h"

extern MODULE_FUNCTION(Mat4, meta);

BEGIN_MODULE(linmath) {
    lua_newtable(L);
    LOAD_META(Mat4);
    return 1;
}