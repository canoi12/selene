#include "lauxlib.h"
#include "selene.h"

#include "stb_image.h"
#include "stb_truetype.h"
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

static BEGIN_META_FUNCTION(Data, Realloc)
    CHECK_INTEGER(size);
    self->data = realloc(self->data, size);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, GetSize)
    PUSH_INTEGER(self->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, Write)
    CHECK_INTEGER(offset);
    int args = lua_gettop(L)-2;
    int error = (offset+args) > self->size;
    if (error) return luaL_error(L, "Data overflow");

    char* dt = ((char*)self->data) + offset;
    for (int i = 0; i < args; i++) {
        char value = (char)luaL_checkinteger(L, 3+i);
        dt[i] = value;
    }
    PUSH_INTEGER(offset + args);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteInt)
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset+(args*4)) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    int* data = (int*)((char*)self->data + offset);
    while (arg <= args) {
        CHECK_INTEGER(value);
        *data = value;
        data++;
    }
    PUSH_INTEGER(offset + (args*4));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteFloat)
    CHECK_INTEGER(offset);
    int args = lua_gettop(L);
    int error = (offset+(args*4)) > self->size;
    if (error) return luaL_error(L, "Data overflow");

    float* data = (float*)((char*)self->data +offset);
    while (arg <= args) {
        CHECK_NUMBER(float, value);
        *data = value;
        data++;
    }
   PUSH_INTEGER(offset + (args*4));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, WriteString)
    CHECK_INTEGER(offset);
    size_t size;
    const char* str = luaL_checklstring(L, arg++, &size);
    int error = (offset+size) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    char* data = (char*)self->data + offset;
    for (char* p = (char*)str; *p != '\0'; p++) {
        *data = *p;
        data++;
    }
    offset += size;
    PUSH_INTEGER(offset + size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, gc)
    if (self->data) {
        free(self->data);
        self->data = NULL;
        self->size = 0;
    }
END_FUNCTION(0)

static BEGIN_META(Data)
    BEGIN_REG(Data)
        REG_META_FIELD(Data, Realloc),
        REG_META_FIELD(Data, GetSize),
        REG_META_FIELD(Data, Write),
        REG_META_FIELD(Data, WriteInt),
        REG_META_FIELD(Data, WriteFloat),
        REG_META_FIELD(Data, WriteString),
        REG_META_FIELD(Data, gc),
    END_REG()
    NEW_META(Data);
END_META(1)

static BEGIN_FUNCTION(utils, NewData)
    CHECK_INTEGER(size);
    NEW_UDATA(Data, data);
    data->size = size;
    data->data = malloc(size);
END_FUNCTION(1)


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
    char* p = (char*)str;
    int codepoint = *str;
    if (codepoint < 0x80) {
        PUSH_INTEGER(codepoint);
        return 1;
    }

    switch (codepoint & 0xf0)
    {
        case 0xf0:
            codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
            break;
        case 0xe0: {
            codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
            break;
        }
        case 0xc0:
        case 0xd0: {
            codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
            break;
        }
        default:
            codepoint = -1;
    }
    if (codepoint > MAX_UNICODE) codepoint = -1;
    PUSH_INTEGER(codepoint);
END_FUNCTION(1)

static void _audio_callback(void* userdata, Uint8* stream, int len) {

}

static BEGIN_FUNCTION(utils, GetAudioCallback)
    lua_pushlightuserdata(L, _audio_callback);
END_FUNCTION(1)

static BEGIN_FUNCTION(utils, LoadOgg)
END_FUNCTION(1)

static BEGIN_META(AudioSource)
END_FUNCTION(1)

BEGIN_MODULE(utils)
    BEGIN_REG(utils)
        REG_FIELD(utils, NewData),
        REG_FIELD(utils, NewMat4),
        REG_FIELD(utils, LoadImageData),
        REG_FIELD(utils, LoadTTF),
        REG_FIELD(utils, UTF8Codepoint),
        REG_FIELD(utils, GetAudioCallback),
        REG_FIELD(utils, LoadOgg),
    END_REG()
    NEW_MODULE(utils);
    LOAD_META(Data);
    LOAD_META(Mat4);
END_MODULE(1)

#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"