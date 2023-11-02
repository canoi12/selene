#include "gl.h"

static MODULE_FUNCTION(Framebuffer, create) {
    NEW_UDATA(Framebuffer, fbo);
    glGenFramebuffers(1, fbo);
    return 1;
}

static MODULE_FUNCTION(Framebuffer, bind) {
    INIT_ARG();
    CHECK_INTEGER(target);
    TEST_UDATA(Framebuffer, fbo);
    if (fbo) glBindFramebuffer(target, *fbo);
    else glBindFramebuffer(target, 0);
    return 0;
}

static MODULE_FUNCTION(Framebuffer, attachTexture2D) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(attachment);
    CHECK_INTEGER(tex_target);
    CHECK_UDATA(Texture, tex);
    glFramebufferTexture2D(target, attachment, tex_target, *tex, 0);
    return 0;
}

// Meta

static META_FUNCTION(Framebuffer, destroy) {
    CHECK_META(Framebuffer);
    glDeleteFramebuffers(1, self);
    return 0;
}

BEGIN_META(Framebuffer) {
    BEGIN_REG()
        REG_FIELD(Framebuffer, create),
        REG_FIELD(Framebuffer, bind),
        REG_FIELD(Framebuffer, attachTexture2D),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Framebuffer, destroy),
    END_REG()
    NEW_META(Framebuffer, _reg, _index_reg);
    return 1;
}
