#include "lauxlib.h"
#include "selene.h"

#include "stb_image.h"
#include "stb_truetype.h"
#include "linmath.h"

static META_FUNCTION(Data, Realloc) {
    INIT_GET_UDATA(Data, data);
    CHECK_INTEGER(size);
    if (size < data->offset)
        data->offset = size - 1;
    data->data = realloc(data->data, size);
    data->size = size;
    return 0;
}

static META_FUNCTION(Data, GetOffset) {
    INIT_GET_UDATA(Data, data);
    PUSH_INTEGER(data->offset);
    return 1;
}

static META_FUNCTION(Data, SetOffset) {
    INIT_GET_UDATA(Data, data);
    CHECK_INTEGER(offset);
    data->offset = offset > data->size ? data->size : offset;
    return 1;
}

static META_FUNCTION(Data, GetSize) {
    INIT_GET_UDATA(Data, data);
    PUSH_INTEGER(data->size);
    return 1;
}

static META_FUNCTION(Data, Write) {
    INIT_GET_UDATA(Data, data);
    int args = lua_gettop(L)-1;
    int error = (data->offset+args) > data->size;
    if (error) return luaL_error(L, "Data overflow");

    char* dt = ((char*)data->data) + data->offset;
    for (int i = 0; i < args; i++) {
        char value = (char)luaL_checkinteger(L, 2+i);
        dt[i] = value;
    }
    data->offset += args;
    return 0;
}
static META_FUNCTION(Data, WriteInt) {
    INIT_GET_UDATA(Data, data);
    int args = lua_gettop(L)-1;
    int error = (data->offset+(args*4)) > data->size;
    if (error) return luaL_error(L, "Data overflow");
    int* dt = (int*)((char*)data->data + data->offset);
    for (int i = 0; i < args; i++) {
        int value = (int)luaL_checkinteger(L, 2+i);
        dt[i] = value;
    }
    data->offset += args*4;
    return 0;
}

static META_FUNCTION(Data, WriteFloat) {
    INIT_GET_UDATA(Data, data);
    int args = lua_gettop(L)-1;
    int error = (data->offset+(args*4)) > data->size;
    if (error) return luaL_error(L, "Data overflow");
    float* dt = (float*)((char*)data->data + data->offset);
    for (int i = 0; i < args; i++) {
        float value = (float)luaL_checknumber(L, 2+i);
        dt[i] = value;
    }
    data->offset += args*4;
    return 0;
}

static META_FUNCTION(Data, WriteString) {
    INIT_GET_UDATA(Data, data);
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
    return 0;
}

static META_FUNCTION(Data, gc) {
    INIT_GET_UDATA(Data, data);
    if (data->data) {
        free(data->data);
        data->data = NULL;
    }
    return 0;
}

BEGIN_REG(Data)
    META_FIELD(Data, Realloc),
    META_FIELD(Data, GetOffset),
    META_FIELD(Data, SetOffset),
    META_FIELD(Data, GetSize),
    META_FIELD(Data, Write),
    META_FIELD(Data, WriteInt),
    META_FIELD(Data, WriteFloat),
    META_FIELD(Data, WriteString),
    META_FIELD(Data, gc),
END_REG()
NEW_META(Data)

static int l_utils_NewData(lua_State* L) {
    int size = (int)luaL_checkinteger(L, 1);
    Data* data = lua_newuserdata(L, sizeof(*data));
    luaL_setmetatable(L, "Data");
    data->offset = 0;
    data->size = size;
    data->data = malloc(size);
    return 1;
}


// Matrix
typedef mat4x4 Mat4;
static META_FUNCTION(Mat4, Identity) {
    INIT_GET_UDATA(Mat4, mat);
    mat4x4_identity(*mat);
    return 0;
}

static META_FUNCTION(Mat4, Translate) {
    INIT_GET_UDATA(Mat4, mat);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_translate_in_place(*mat, x, y, z);
    return 0;
}

static META_FUNCTION(Mat4, Scale) {
    INIT_GET_UDATA(Mat4, mat);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    OPT_NUMBER(float, z, 0.f);
    mat4x4_scale_aniso(*mat, *mat, x, y, z);
    return 0;
}

static META_FUNCTION(Mat4, Rotate) {
    return 0;
}

static META_FUNCTION(Mat4, Ortho) {
    INIT_GET_UDATA(Mat4, mat);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_ortho(*mat, left, right, bottom, top, near, far);
    return 0;
}

static META_FUNCTION(Mat4, Frustum) {
    INIT_GET_UDATA(Mat4, mat);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near);
    CHECK_NUMBER(float, far);
    mat4x4_frustum(*mat, left, right, bottom, top, near, far);
    return 0;
}

BEGIN_REG(Mat4)
    META_FIELD(Mat4, Identity),
    META_FIELD(Mat4, Translate),
    META_FIELD(Mat4, Scale),
    META_FIELD(Mat4, Rotate),
    META_FIELD(Mat4, Ortho),
    META_FIELD(Mat4, Frustum),
END_REG()
NEW_META(Mat4)

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

int seleneopen_utils(lua_State* L) {
    luaL_Reg reg[] = {
        {"NewData", l_utils_NewData},
        {"NewMat4", l_utils_NewMat4},
        {"LoadImageData", l_utils_LoadImageData},
        {NULL, NULL}
    };
    luaL_newlib(L, reg);

    l_Mat4_meta(L);
    lua_setfield(L, -2, "Mat4");
    l_Data_meta(L);
    lua_setfield(L, -2, "Data");
    return 1;
}
