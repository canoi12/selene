#include "selene.h"
#include "lua_helper.h"

extern MODULE_FUNCTION(vec2, meta);
extern MODULE_FUNCTION(vec3, meta);
extern MODULE_FUNCTION(vec4, meta);
extern MODULE_FUNCTION(mat4x4, meta);
extern MODULE_FUNCTION(quat, meta);

BEGIN_MODULE(linmath) {
    lua_newtable(L);
    LOAD_META(vec2);
    LOAD_META(vec3);
    LOAD_META(vec4);
    LOAD_META(mat4x4);
    LOAD_META(quat);
    return 1;
}