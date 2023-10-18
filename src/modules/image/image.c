#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static MODULE_FUNCTION(image, LoadData) {
    INIT_ARG();
    CHECK_STRING(path);
    OPT_INTEGER(req, STBI_rgb_alpha);
    int w, h, c;
    stbi_uc* pixels = stbi_load(path, &w, &h, &c, req);
    NEW_UDATA(Data, d);
    d->size = w * h * req;
    d->data = pixels;

    PUSH_INTEGER(w);
    PUSH_INTEGER(h);
    PUSH_INTEGER(req);
    return 4;
}

BEGIN_MODULE(image) {
    BEGIN_REG()
        REG_FIELD(image, LoadData),
    END_REG()
    luaL_newlib(L, _reg);
    return 1;
}
