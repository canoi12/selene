#include "common.h"
#include "lua_helper.h"

typedef mat4x4 Mat4;

static MODULE_FUNCTION(Mat4, New) {
    NEW_UDATA(Mat4, m);
    mat4x4_identity(*m);
    return 1;
}

static META_FUNCTION(Mat4, Identity) {
    return 0;
}

static META_FUNCTION(Mat4, Translate) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_translate_in_place(*self, x, y, z);
    return 0;
}

static META_FUNCTION(Mat4, Scale) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_scale_aniso(*self, *self, x, y, z);
    return 0;
}

static META_FUNCTION(Mat4, Rotate) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, angle);
    mat4x4_rotate_Z(*self, *self, angle);
    return 0;
}

static META_FUNCTION(Mat4, Ortho) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_ortho(*self, left, right, bottom, top, near, far);
    return 0;
}

static META_FUNCTION(Mat4, Frustum) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_frustum(*self, left, right, bottom, top, near, far);
    return 0;
}

BEGIN_META(Mat4) {
    BEGIN_REG()
        REG_FIELD(Mat4, New),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Mat4, Identity),
        REG_META_FIELD(Mat4, Translate),
        REG_META_FIELD(Mat4, Scale),
        REG_META_FIELD(Mat4, Rotate),
        REG_META_FIELD(Mat4, Ortho),
        REG_META_FIELD(Mat4, Frustum),
    END_REG()
    NEW_META(Mat4, _reg, _index_reg);
    return 1;
}