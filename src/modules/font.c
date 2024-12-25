#include "common.h"
#ifndef SELENE_NO_FONT
#include "selene.h"
#include "lua_helper.h"

#include "font8x8/font8x8_latin.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static MODULE_FUNCTION(FontGlyph, __index) {
    CHECK_META(FontGlyph);
    int index = luaL_checkinteger(L, 2);
    lua_pushinteger(L, self[index].ax);
    lua_pushinteger(L, self[index].ay);
    lua_pushinteger(L, self[index].bl);
    lua_pushinteger(L, self[index].bt);
    lua_pushinteger(L, self[index].bw);
    lua_pushinteger(L, self[index].bh);
    lua_pushinteger(L, self[index].tx);
    return 7;
}

static MODULE_FUNCTION(FontGlyph, meta) {
    luaL_newmetatable(L, "FontGlyph");
    lua_pushcfunction(L, l_FontGlyph___index);
    lua_setfield(L, -2, "__index");
    return 1;
}

static MODULE_FUNCTION(font, create8x8) {
    int w = 2048;
    int h = 8;
    uint8_t* bitmap = malloc(w*h*4);
    int ox = 0;
    int stride = w * sizeof(int);
    for (int i = 0; i < 128; i++) {
        for (int y = 0; y < 8; y++) {
            uint8_t l = font8x8_basic[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
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
            uint8_t l = font8x8_control[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
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
            uint8_t l = font8x8_ext_latin[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    lua_newtable(L);
    size_t size = w*h*4;
    NEW_UDATA_ADD(Data, dt, sizeof(Data)+size);
    memcpy(&dt[1], bitmap, size);
    free(bitmap);
    lua_setfield(L, -2, "data");
    lua_pushinteger(L, w);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, h);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, 4);
    lua_setfield(L, -2, "channels");
    lua_pushinteger(L, SELENE_PIXEL_RGBA);
    lua_setfield(L, -2, "format");
    NEW_UDATA_ADD(FontGlyph, glyphs, sizeof(FontGlyph)*256);
    lua_setfield(L, -2, "glyphs");
    for (int i = 0; i < 256; i++) {
        glyphs[i].ax = 8;
        glyphs[i].ay = 0;
        glyphs[i].bl = 0;
        glyphs[i].bt = 0;
        glyphs[i].bw = 8;
        glyphs[i].bh = 8;
        glyphs[i].tx = 8 * i;
    }
    return 1;
}

static MODULE_FUNCTION(font, from_ttf) {
    INIT_ARG();
    CHECK_STRING(path);
    OPT_INTEGER(font_size, 16);
    stbtt_fontinfo info;
#ifndef SELENE_NO_SDL
    SDL_RWops* fp = SDL_RWFromFile(path, "rb");
#else
    #if defined(OS_WIN)
        FILE* fp;
        fopen_s(&fp, path, "rb");
    #else
        FILE* fp = fopen(path, "rb");
    #endif
#endif
    if (!fp)
        return luaL_error(L, "Failed to open font: %s", path);
#ifndef SELENE_NO_SDL
    size_t size = SDL_RWsize(fp);
#else
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
#endif
    void* data = malloc(size);
    if (!data)
        return luaL_error(L, "Failed to alloc memory for font");
#ifndef SELENE_NO_SDL
    SDL_RWread(fp, data, 1, size);
    SDL_RWclose(fp);
#else
    fread(data, size, 1, fp);
    fclose(fp);
#endif

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
    FontGlyph aux_glyphs[256];

    for (i = 0; i < 256; i++) {
        int ax, bl;
        int x0, y0, x1, y1;
        int w, h;

        stbtt_GetCodepointHMetrics(&info, i, &ax, &bl);
        stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &x0, &y0, &x1, &y1);

        w = x1 - x0;
        h = y1 - y0;

        aux_glyphs[i].ax = (int)(ax * scale);
        aux_glyphs[i].ay = 0;
        aux_glyphs[i].bl = (int)(bl * scale);
        aux_glyphs[i].bw = w;
        aux_glyphs[i].bh = h;
        aux_glyphs[i].bt = baseline + y0;

        tw += w;
        th = MAX(th, h);
    }
    int height = th;
    const int final_size = tw * th;
    lua_newtable(L);
    NEW_UDATA_ADD(Data, dt, final_size * sizeof(Data));
    lua_setfield(L, -2, "data");
    lua_pushinteger(L, tw);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, th);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, 4);
    lua_setfield(L, -2, "channels");
    lua_pushinteger(L, SELENE_PIXEL_RGBA);
    lua_setfield(L, -2, "format");

    uint32_t* bitmap = &dt[1];
    memset(bitmap, 0, final_size*sizeof(Data));
    int x = 0;

    NEW_UDATA_ADD(FontGlyph, glyphs, sizeof(FontGlyph)*256);
    memcpy(glyphs, aux_glyphs, sizeof(aux_glyphs));
    lua_setfield(L, -2, "glyphs");
    for (int i = 0; i < 256; i++) {
        int ww = glyphs[i].bw;
        int hh = glyphs[i].bh;
        int ssize = ww * hh;
        int ox, oy;

        uint8_t* bmp = stbtt_GetCodepointBitmap(&info, 0, scale, i, NULL, NULL, &ox, &oy);
        int xx, yy;
        xx = yy = 0;
        for (int j = 0; j < ssize; j++) {
            xx = (j % ww) + x;
            if (j != 0 && j % ww == 0)
                yy += tw;

            uint8_t* b = (uint8_t*)&bitmap[xx + yy];
            b[0] = 255;
            b[1] = 255;
            b[2] = 255;
            b[3] = bmp[j];
        }
        stbtt_FreeBitmap(bmp, info.userdata);

        glyphs[i].tx = x;

        x += glyphs[i].bw;
    }
    return 1;
}

int luaopen_font(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(font, create8x8),
        REG_FIELD(font, from_ttf),
    END_REG()
    luaL_newlib(L, reg);
    l_FontGlyph_meta(L);
    lua_setfield(L, -2, "FontGlyph");
    return 1;
}
#endif /* SELENE_NO_FONT */
