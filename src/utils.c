#include "selene.h"

#include "stb_image.h"
#include "stb_truetype.h"
#include "font8x8/font8x8_latin.h"

// Matrix
typedef mat4x4 Mat4;
static BEGIN_META_FUNCTION(Mat4, Identity)
    mat4x4_identity(*self);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Translate)
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_translate_in_place(*self, x, y, z);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Scale)
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_scale_aniso(*self, *self, x, y, z);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Rotate)
    CHECK_NUMBER(float, angle);
    mat4x4_rotate_Z(*self, *self, angle);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Ortho)
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_ortho(*self, left, right, bottom, top, near, far);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Frustum)
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_frustum(*self, left, right, bottom, top, near, far);
END_FUNCTION(0)

static BEGIN_META(Mat4)
    BEGIN_REG(Mat4)
        REG_META_FIELD(Mat4, Identity),
        REG_META_FIELD(Mat4, Translate),
        REG_META_FIELD(Mat4, Scale),
        REG_META_FIELD(Mat4, Rotate),
        REG_META_FIELD(Mat4, Ortho),
        REG_META_FIELD(Mat4, Frustum),
    END_REG()
    NEW_META(Mat4);
END_META(1)

static BEGIN_FUNCTION(utils, NewMat4)
    NEW_UDATA(Mat4, m);
    mat4x4_identity(*m);
END_FUNCTION(1)

static BEGIN_FUNCTION(utils, LoadImageData)
    CHECK_STRING(path);
    OPT_INTEGER(req, STBI_rgb_alpha);
    int w, h, c;
    Uint8* pixels = stbi_load(path, &w, &h, &c, req);
    NEW_UDATA(Data, d);
    d->size = w * h * req;
    d->data = pixels;

    PUSH_INTEGER(w);
    PUSH_INTEGER(h);
    PUSH_INTEGER(req);
END_FUNCTION(4)

static BEGIN_FUNCTION(utils, LoadTTF)
    CHECK_STRING(path);
    OPT_INTEGER(font_size, 16);
    stbtt_fontinfo info;
#if defined(OS_WIN)
    FILE* fp;
    fopen_s(&fp, path, "rb");
#else
    FILE* fp = fopen(path, "rb");
#endif
    if (!fp)
        return luaL_error(L, "Failed to open font: %s", path);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    void* data = malloc(size);
    if (!data)
        return luaL_error(L, "Failed to alloc memory for font");
    fread(data, size, 1, fp);

    fclose(fp);

    if (!stbtt_InitFont(&info, data, 0))
        return luaL_error(L, "Failed to init font data");

    int ascent, descent, line_gap;
    float fsize = (float)font_size;
    float scale = stbtt_ScaleForMappingEmToPixels(&info, fsize);
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
    int baseline = (int)(ascent * scale);

    int tw, th;
    tw = th = 0;

    int i;
    struct {
        int ax, ay;
        int bl, bw;
        int bh, bt;
        int tx;
    } glyphs[256];

    for (i = 0; i < 256; i++) {
        int ax, bl;
        int x0, y0, x1, y1;
        int w, h;

        stbtt_GetCodepointHMetrics(&info, i, &ax, &bl);
        stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &x0, &y0, &x1, &y1);

        w = x1 - x0;
        h = y1 - y0;

        glyphs[i].ax = (int)(ax * scale);
        glyphs[i].ay = 0;
        glyphs[i].bl = (int)(bl * scale);
        glyphs[i].bw = w;
        glyphs[i].bh = h;
        glyphs[i].bt = baseline + y0;

        tw += w;
        th = MAX(th, h);
    }

    int height = th;
    const int final_size = tw * th;

    NEW_UDATA(Data, dt);
    dt->size = final_size * sizeof(Uint32);
    dt->data = malloc(dt->size);

    Uint32* bitmap = dt->data;
    memset(bitmap, 0, dt->size);
    int x = 0;

    PUSH_INTEGER(tw);
    PUSH_INTEGER(th);

    lua_newtable(L);
    for (int i = 0; i < 256; i++) {
        int ww = glyphs[i].bw;
        int hh = glyphs[i].bh;
        int ssize = ww * hh;
        int ox, oy;

        unsigned char* bmp = stbtt_GetCodepointBitmap(&info, 0, scale, i, NULL, NULL, &ox, &oy);
        int xx, yy;
        xx = yy = 0;
        for (int j = 0; j < ssize; j++) {
            xx = (j % ww) + x;
            if (j != 0 && j % ww == 0)
                yy += tw;

            Uint8* b = (Uint8*)&bitmap[xx + yy];
            b[0] = 255;
            b[1] = 255;
            b[2] = 255;
            b[3] = bmp[j];
        }
        stbtt_FreeBitmap(bmp, info.userdata);

        glyphs[i].tx = x;

        x += glyphs[i].bw;

        lua_newtable(L);
        int p = 1;
        lua_pushinteger(L, glyphs[i].ax);
        lua_setfield(L, -2, "ax");
        lua_pushinteger(L, glyphs[i].ay);
        lua_setfield(L, -2, "ay");
        lua_pushinteger(L, glyphs[i].bh);
        lua_setfield(L, -2, "bh");
        lua_pushinteger(L, glyphs[i].bl);
        lua_setfield(L, -2, "bl");
        lua_pushinteger(L, glyphs[i].bt);
        lua_setfield(L, -2, "bt");
        lua_pushinteger(L, glyphs[i].bw);
        lua_setfield(L, -2, "bw");
        lua_pushinteger(L, glyphs[i].tx);
        lua_setfield(L, -2, "tx");
        lua_rawseti(L, -2, i);
    }
    free(data);
END_FUNCTION(4)

#define MAX_UNICODE 0x10FFFF
static BEGIN_FUNCTION(utils, UTF8Codepoint)
    CHECK_STRING(str);
    CHECK_INTEGER(pos);
    Uint8* p = (Uint8*)str + pos-1;
    if (*p < 0x80) {
        PUSH_INTEGER(*p);
        PUSH_INTEGER(1);
        return 2;
    }
    int codepoint = *p;
    int size = 1;

    switch (codepoint & 0xf0)
    {
        case 0xf0:
            codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
            size = 4;
            break;
        case 0xe0: {
            codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
            size = 3;
            break;
        }
        case 0xc0:
        case 0xd0: {
            codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
            size = 2;
            break;
        }
        default:
            codepoint = -1;
    }
    if (codepoint > MAX_UNICODE) codepoint = -1;
    PUSH_INTEGER(codepoint);
    PUSH_INTEGER(size);
END_FUNCTION(2)

static BEGIN_FUNCTION(utils, GetDefaultFont)
    int w = 2048;
    int h = 8;
    Uint8* bitmap = malloc(w * h * 4);
    int ox = 0;
    int stride = w * sizeof(int);
    for (int i = 0; i < 128; i++) {
        for (int y = 0; y < 8; y++) {
            Uint8 l = font8x8_basic[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                Uint8* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    ox = 128 * 8 * sizeof(int);
    for (int i = 0; i < 32; i++) {
        for (int y = 0; y < 8; y++) {
            Uint8 l = font8x8_control[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                Uint8* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    ox = 160 * 8 * sizeof(int);
    for (int i = 0; i < 96; i++) {
        for (int y = 0; y < 8; y++) {
            Uint8 l = font8x8_ext_latin[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                Uint8* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    NEW_UDATA(Data, dt);
    dt->size = w * h * 4;
    dt->data = bitmap;
    PUSH_INTEGER(w);
    PUSH_INTEGER(h);
    lua_newtable(L);
    for (int i = 0; i < 256; i++) {
        lua_newtable(L);
        lua_pushinteger(L, 8);
        lua_setfield(L, -2, "ax");
        lua_pushinteger(L, 0);
        lua_setfield(L, -2, "ay");
        lua_pushinteger(L, 8);
        lua_setfield(L, -2, "bh");
        lua_pushinteger(L, 8);
        lua_setfield(L, -2, "bl");
        lua_pushinteger(L, 8);
        lua_setfield(L, -2, "bt");
        lua_pushinteger(L, 8);
        lua_setfield(L, -2, "bw");
        lua_pushinteger(L, (8 * i));
        lua_setfield(L, -2, "tx");
        lua_rawseti(L, -2, i);
    }
END_FUNCTION(4)

BEGIN_MODULE(utils)
    BEGIN_REG(utils)
        REG_FIELD(utils, NewMat4),
        REG_FIELD(utils, LoadImageData),
        REG_FIELD(utils, LoadTTF),
        REG_FIELD(utils, GetDefaultFont),
        REG_FIELD(utils, UTF8Codepoint),
    END_REG()
    NEW_MODULE(utils);
    LOAD_META(Mat4);
END_MODULE(1)
