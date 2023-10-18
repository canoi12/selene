#include "audio.h"

#include "selene.h"
#include "lua_helper.h"

extern MODULE_FUNCTION(Decoder, meta);

BEGIN_MODULE(audio) {
    BEGIN_REG()
    END_REG()
    luaL_newlib(L, _reg);
    LOAD_META(Decoder);
    return 1;
}
