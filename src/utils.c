#include "lauxlib.h"
#include "selene.h"

#include "stb_image.h"
#include "stb_truetype.h"

static BEGIN_META_FUNCTION(Data, Realloc)
    CHECK_INTEGER(size);
    if (size < self->offset)
        self->offset = size - 1;
    self->data = realloc(self->data, size);
    self->size = size;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, GetOffset)
    PUSH_INTEGER(self->offset);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, SetOffset)
    CHECK_INTEGER(offset);
    self->offset = offset > self->size ? self->size : offset;
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, GetSize)
    PUSH_INTEGER(self->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, Write)
    int args = lua_gettop(L)-1;
    int error = (self->offset+args) > self->size;
    if (error) return luaL_error(L, "Data overflow");

    char* dt = ((char*)self->data) + self->offset;
    for (int i = 0; i < args; i++) {
        char value = (char)luaL_checkinteger(L, 2+i);
        dt[i] = value;
    }
    self->offset += args;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, WriteInt)
    int args = lua_gettop(L)-1;
    int error = (self->offset+(args*4)) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    int* dt = (int*)((char*)self->data + self->offset);
    for (int i = 0; i < args; i++) {
        int value = (int)luaL_checkinteger(L, 2+i);
        dt[i] = value;
    }
    self->offset += args*4;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, WriteFloat)
    int args = lua_gettop(L)-1;
    int error = (self->offset+(args*4)) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    float* dt = (float*)((char*)self->data + self->offset);
    for (int i = 0; i < args; i++) {
        float value = (float)luaL_checknumber(L, 2+i);
        dt[i] = value;
    }
    self->offset += args*4;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, WriteString)
    size_t size;
    const char* str = luaL_checklstring(L, 2, &size);
    int error = (self->offset+size) > self->size;
    if (error) return luaL_error(L, "Data overflow");
    char* dt = (char*)self->data + self->offset;
    for (char* p = (char*)str; *p != '\0'; p++) {
        *dt = *p;
        dt++;
    }
    self->offset += size;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, gc)
    if (self->data) {
        free(self->data);
        self->data = NULL;
    }
END_FUNCTION(0)

static BEGIN_META(Data)
    BEGIN_REG(Data)
        REG_META_FIELD(Data, Realloc),
        REG_META_FIELD(Data, GetOffset),
        REG_META_FIELD(Data, SetOffset),
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
    data->offset = 0;
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
    CHECK_INTEGER(angle);
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
    d->offset = 0;
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

    Uint32* bitmap = malloc(final_size * sizeof(Uint32));
    memset(bitmap, 0, final_size * sizeof(Uint32));
    int x = 0;
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
    }
    
    NEW_UDATA(Texture, tex);
    int target = GL_TEXTURE_2D;
    glBindTexture(target, *tex);
    glTexImage2D(target, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(target, 0);

    PUSH_INTEGER(tw);
    PUSH_INTEGER(th);

    free(bitmap);
END_FUNCTION(4)

BEGIN_MODULE(utils)
    BEGIN_REG(utils)
        REG_FIELD(utils, NewData),
        REG_FIELD(utils, NewMat4),
        REG_FIELD(utils, LoadImageData),
    END_REG()
    NEW_MODULE(utils);
    LOAD_META(Data);
    LOAD_META(Mat4);
END_MODULE(1)
