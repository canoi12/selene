#include "lauxlib.h"
#include "selene.h"

#include "stb_image.h"
#include "stb_truetype.h"
#include "linmath.h"

static BEGIN_META_FUNCTION(Data, Realloc, data)
    CHECK_INTEGER(size);
    if (size < data->offset)
        data->offset = size - 1;
    data->data = realloc(data->data, size);
    data->size = size;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, GetOffset, data)
    PUSH_INTEGER(data->offset);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, SetOffset, data)
    CHECK_INTEGER(offset);
    data->offset = offset > data->size ? data->size : offset;
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, GetSize, data)
    PUSH_INTEGER(data->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Data, Write, data)
    int args = lua_gettop(L)-1;
    int error = (data->offset+args) > data->size;
    if (error) return luaL_error(L, "Data overflow");

    char* dt = ((char*)data->data) + data->offset;
    for (int i = 0; i < args; i++) {
        char value = (char)luaL_checkinteger(L, 2+i);
        dt[i] = value;
    }
    data->offset += args;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, WriteInt, data)
    int args = lua_gettop(L)-1;
    int error = (data->offset+(args*4)) > data->size;
    if (error) return luaL_error(L, "Data overflow");
    int* dt = (int*)((char*)data->data + data->offset);
    for (int i = 0; i < args; i++) {
        int value = (int)luaL_checkinteger(L, 2+i);
        dt[i] = value;
    }
    data->offset += args*4;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, WriteFloat, data)
    int args = lua_gettop(L)-1;
    int error = (data->offset+(args*4)) > data->size;
    if (error) return luaL_error(L, "Data overflow");
    float* dt = (float*)((char*)data->data + data->offset);
    for (int i = 0; i < args; i++) {
        float value = (float)luaL_checknumber(L, 2+i);
        dt[i] = value;
    }
    data->offset += args*4;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, WriteString, data)
    size_t size;
    const char* str = luaL_checklstring(L, 2, &size);
    int error = (data->offset+size) > data->size;
    if (error) return luaL_error(L, "Data overflow");
    char* dt = (char*)data->data + data->offset;
    for (char* p = (char*)str; *p != '\0'; p++) {
        *dt = *p;
        dt++;
    }
    data->offset += size;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Data, gc, data)
    if (data->data) {
        free(data->data);
        data->data = NULL;
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
    int size = (int)luaL_checkinteger(L, 1);
    NEW_UDATA(Data, data, sizeof(*data));
    data->offset = 0;
    data->size = size;
    data->data = malloc(size);
END_FUNCTION(1)


// Matrix
typedef mat4x4 Mat4;
static BEGIN_META_FUNCTION(Mat4, Identity, mat)
    mat4x4_identity(*mat);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Translate, mat)
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_translate_in_place(*mat, x, y, z);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Scale, mat)
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_scale_aniso(*mat, *mat, x, y, z);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Rotate, mat)
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Ortho, mat)
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_ortho(*mat, left, right, bottom, top, near, far);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Mat4, Frustum, mat)
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_frustum(*mat, left, right, bottom, top, near, far);
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

static int l_utils_NewMat4(lua_State* L) {
    mat4x4* m = lua_newuserdata(L, sizeof(*m));
    luaL_setmetatable(L, "Mat4");
    mat4x4_identity(*m);
    return 1;
}

static int l_utils_LoadImageData(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int req = (int)luaL_optinteger(L, 2, STBI_rgb_alpha);
    int w, h, c;
    Uint8* pixels = stbi_load(path, &w, &h, &c, req);
    Data* d = lua_newuserdata(L, sizeof(*d));
    luaL_setmetatable(L, "Data");
    d->offset = 0;
    d->size = w * h * req;
    d->data = pixels;

    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    lua_pushinteger(L, req);
    return 4;
}

static int l_utils_LoadTTF(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int font_size = (int)luaL_optinteger(L, 2, 16);
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
    int tw, th;
    tw = th = 0;

    int i;
    for (i = 0; i < 256; i++) {

    }
    return 4;
}

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