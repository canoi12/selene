#include "common.h"
#include "lua_helper.h"

typedef mat4x4 Mat4;

static MODULE_FUNCTION(Mat4, create) {
    NEW_UDATA(Mat4, m);
    mat4x4_identity(*m);
    return 1;
}

static META_FUNCTION(Mat4, identity) {
    return 0;
}

static META_FUNCTION(Mat4, translate) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_translate_in_place(*self, x, y, z);
    return 0;
}

static META_FUNCTION(Mat4, scale) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_scale_aniso(*self, *self, x, y, z);
    return 0;
}

static META_FUNCTION(Mat4, rotate) {
    CHECK_META(Mat4);
    CHECK_NUMBER(float, angle);
    mat4x4_rotate_Z(*self, *self, angle);
    return 0;
}

static META_FUNCTION(Mat4, ortho) {
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

static META_FUNCTION(Mat4, frustum) {
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
        REG_FIELD(Mat4, create),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Mat4, identity),
        REG_META_FIELD(Mat4, translate),
        REG_META_FIELD(Mat4, scale),
        REG_META_FIELD(Mat4, rotate),
        REG_META_FIELD(Mat4, ortho),
        REG_META_FIELD(Mat4, frustum),
    END_REG()
    NEW_META(Mat4, _reg, _index_reg);
    return 1;
}