#include "../renderer.h"

//#include "font8x8/font8x8_latin.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

extern char font8x8_basic[128][8];
extern char font8x8_control[32][8];
    extern char font8x8_ext_latin[96][8];

int l_Font_8x8(lua_State* L) {
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
    NEW_UDATA(Font, font);
    font->texture.width = w;
    font->texture.height = h;
    size_t size = w*h*4;

    glGenTextures(1, &(font->texture.handle));
    glBindTexture(GL_TEXTURE_2D, font->texture.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(bitmap);

    FontGlyph* glyphs = font->glyphs;
    for (int i = 0; i < 256; i++) {
        glyphs[i].ax = 8 / w;
        glyphs[i].ay = 0;
        glyphs[i].bl = 0;
        glyphs[i].bt = 0;
        glyphs[i].bw = 8;
        glyphs[i].bh = 8;
        glyphs[i].tx = 8 * i;
    }
    return 1;
}

int l_Font_load(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(path);
    OPT_INTEGER(font_size, 16);
    stbtt_fontinfo info;
#if defined(SELENE_USE_SDL3)
    SDL_IOStream* fp = SDL_IOFromFile(path, "rb");
#else
    SDL_RWops* fp = SDL_RWFromFile(path, "rb");    
#endif
    if (!fp)
        return luaL_error(L, "Failed to open font: %s", path);
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetIOSize(fp);
#else
    size_t size = SDL_RWsize(fp);
#endif
    void* data = malloc(size);
    if (!data)
        return luaL_error(L, "Failed to alloc memory for font");
#if defined(SELENE_USE_SDL3)
    SDL_ReadIO(fp, data, size);
    SDL_CloseIO(fp);
#else
    SDL_RWread(fp, data, 1, size);
    SDL_RWclose(fp);
#endif

    if (!stbtt_InitFont(&info, data, 0))
        return luaL_error(L, "Failed to init font data");

    int ascent, descent, line_gap;
    float fsize = (float)font_size;
    float scale = stbtt_ScaleForMappingEmToPixels(&info, fsize);
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
    int baseline = (int)(ascent * scale);

    NEW_UDATA(Font, font);

    int tw, th;
    tw = th = 0;

    int i;
    FontGlyph* aux_glyphs = font->glyphs;

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
    font->texture.width = tw;
    font->texture.height = th;
    glGenTextures(1, &(font->texture.handle));

    int x = 0;

    uint32_t* bitmap = (uint32_t*)malloc(tw*th*4);
    memset(bitmap, 0, final_size*4);

    for (int i = 0; i < 256; i++) {
        int ww = font->glyphs[i].bw;
        int hh = font->glyphs[i].bh;
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

        font->glyphs[i].tx = x;

        x += font->glyphs[i].bw;
    }
    glBindTexture(GL_TEXTURE_2D, font->texture.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(bitmap);
    free(data);
    return 1;
}

static int l_Font__destroy(lua_State* L) {
    CHECK_META(Font);
    if (self->texture.handle != 0) {
        glDeleteTextures(1, &(self->texture.handle));
        self->texture.handle = 0;
    }
    return 0;
}

int l_Font_open_meta(lua_State* L) {
    luaL_newmetatable(L, "Font");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_Reg reg[] = {
        REG_FIELD(Font, 8x8),
        REG_FIELD(Font, load),
        REG_META_FIELD(Font, destroy),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
