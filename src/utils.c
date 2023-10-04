#include "lauxlib.h"
#include "selene.h"

#include "stb_image.h"
#include "stb_truetype.h"
#include "linmath.h"

static int l_Data__Realloc(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    int size = luaL_checkinteger(L, 2);
    if (size < data->offset) data->offset = size - 1;
    data->data = realloc(data->data, size);
    data->size = size;
    return 0;
}

static int l_Data__GetOffset(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    lua_pushinteger(L, data->offset);
    return 1;
}

static int l_Data__SetOffset(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    int offset = luaL_checkinteger(L, 2);
    data->offset = offset > data->size ? data->size : offset;
    return 0;
}

static int l_Data__GetSize(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    lua_pushinteger(L, data->size);
    return 1;
}

static int l_Data__Write(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    int args = lua_gettop(L)-1;
    int error = (data->offset+args) > data->size;
    if (error) return luaL_error(L, "Data overflow");

    char* dt = ((char*)data->data) + data->offset;
    for (int i = 0; i < args; i++) {
        char value = luaL_checkinteger(L, 2+i);
        dt[i] = value;
    }
    data->offset += args;
    return 0;
}

static int l_Data__WriteInt(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    int args = lua_gettop(L)-1;
    int error = (data->offset+(args*4)) > data->size;
    if (error) return luaL_error(L, "Data overflow");
    int* dt = (int*)((char*)data->data + data->offset);
    for (int i = 0; i < args; i++) {
        int value = luaL_checknumber(L, 2+i);
        dt[i] = value;
    }
    data->offset += args*4;
    return 0;
}

static int l_Data__WriteFloat(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    int args = lua_gettop(L)-1;
    int error = (data->offset+(args*4)) > data->size;
    if (error) return luaL_error(L, "Data overflow");
    float* dt = (float*)((char*)data->data + data->offset);
    for (int i = 0; i < args; i++) {
        float value = luaL_checknumber(L, 2+i);
        dt[i] = value;
    }
    data->offset += args*4;
    return 0;
}

static int l_Data__WriteString(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
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

static int l_Data__gc(lua_State* L) {
    Data* data = luaL_checkudata(L, 1, "Data");
    if (data->data) {
        free(data->data);
        data->data = NULL;
    }
    return 0;
}

static int l_Data_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"Realloc", l_Data__Realloc},
        {"GetOffset", l_Data__GetOffset},
        {"SetOffset", l_Data__SetOffset},
        {"GetSize", l_Data__GetSize},
        {"Write", l_Data__Write},
        {"WriteInt", l_Data__WriteInt},
        {"WriteFloat", l_Data__WriteFloat},
        {"WriteString", l_Data__WriteString},
        {"__gc", l_Data__gc},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Data");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static int l_utils_NewData(lua_State* L) {
    int size = luaL_checkinteger(L, 1);
    Data* data = lua_newuserdata(L, sizeof(*data));
    luaL_setmetatable(L, "Data");
    data->offset = 0;
    data->size = size;
    data->data = malloc(size);
    return 1;
}


// Matrix
static int l_Mat4__Identity(lua_State* L) {
    mat4x4* mat = luaL_checkudata(L, 1, "Mat4");
    mat4x4_identity(*mat);
    return 0;
}

static int l_Mat4__Translate(lua_State* L) {
    mat4x4* mat = luaL_checkudata(L, 1, "Mat4");
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_optnumber(L, 4, 0.f);
    mat4x4_translate_in_place(*mat, x, y, z);
    return 0;
}

static int l_Mat4__Scale(lua_State* L) {
    mat4x4* mat = luaL_checkudata(L, 1, "Mat4");
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_optnumber(L, 4, 1.f);
    mat4x4_scale_aniso(*mat, *mat, x, y, z);
    return 0;
}

static int l_Mat4__Rotate(lua_State* L) {
    return 0;
}

static int l_Mat4__Ortho(lua_State* L) {
    mat4x4* mat = luaL_checkudata(L, 1, "Mat4");
    float left = luaL_checknumber(L, 2);
    float right = luaL_checknumber(L, 3);
    float bottom = luaL_checknumber(L, 4);
    float top = luaL_checknumber(L, 5);
    float near = luaL_checknumber(L, 6);
    float far = luaL_checknumber(L, 7);
    mat4x4_ortho(*mat, left, right, bottom, top, near, far);
    return 0;
}

static int l_Mat4__Frustum(lua_State* L) {
    mat4x4* mat = luaL_checkudata(L, 1, "Mat4");
    float left = luaL_checknumber(L, 2);
    float right = luaL_checknumber(L, 3);
    float bottom = luaL_checknumber(L, 4);
    float top = luaL_checknumber(L, 5);
    float near = luaL_checknumber(L, 6);
    float far = luaL_checknumber(L, 7);
    mat4x4_frustum(*mat, left, right, bottom, top, near, far);
    return 0;
}

static int l_Mat4_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"Identity", l_Mat4__Identity},
        {"Translate", l_Mat4__Translate},
        {"Scale", l_Mat4__Scale},
        {"Rotate", l_Mat4__Rotate},
        {"Ortho", l_Mat4__Ortho},
        {"Frustum", l_Mat4__Frustum},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Mat4");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static int l_utils_NewMat4(lua_State* L) {
    mat4x4* m = lua_newuserdata(L, sizeof(*m));
    luaL_setmetatable(L, "Mat4");
    mat4x4_identity(*m);
    return 1;
}

static int l_utils_LoadImageData(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int req = luaL_optinteger(L, 2, STBI_rgb_alpha);
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
    int font_size = luaL_optinteger(L, 2, 16);
    stbtt_fontinfo info;
    FILE* fp = fopen(path, "rb");
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
    float fsize = font_size;
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
