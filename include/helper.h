#ifndef HELPER_H_
#define HELPER_H_

#include "selene.h"
#include "common.h"

void char_rect(void* f, const int c, float *x, float *y, int* out_pos, int* out_rect, int width) {
    FontData* font = (FontData*)f;
    if (c == '\n') {
        *x = 0;
        *y += font->image_width;
        return;
    }

    if (c == '\t') {
        *x += font->glyphs[c].bw * 2;
        return;
    }
    if (width != 0 && *x + (font->glyphs[c].bl) > width) {
        *x = 0;
        *y += font->image_height;
    }

    float x2 = *x + (float)font->glyphs[c].bl;
    float y2 = *y + (float)font->glyphs[c].bt;

    float s0 = font->glyphs[c].tx;
    float t0 = 0;
    int s1 = font->glyphs[c].bw;
    int t1 = font->glyphs[c].bh;

    *x += font->glyphs[c].ax;
    *y += font->glyphs[c].ay;

    if (out_pos) {
        out_pos[0] = x2;
        out_pos[1] = y2;
    }
    if (out_rect) {
        out_rect[0] = s0;
        out_rect[1] = t0;
        out_rect[2] = s1;
        out_rect[3] = t1;
    }
}


uint8_t* utf8_codepoint(const uint8_t* p, int* codepoint) {
    uint8_t* n = NULL;
    uint8_t c = *p;
    if (c < 0x80) { // Se for um ascii normal, o codepoint é o próprio caractere
        *codepoint = c;
        n = (uint8_t*)p + 1;
        return n;
    }

    switch (c & 0xf0) { // Checa o primeiro byte para conferir quantos bytes precisam ser usados
    case 0xf0: { // 4 bytes
        *codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
        n = (uint8_t*)p + 4;
        break;
    }
    case 0xe0: { // 3 bytes
        *codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
        n = (uint8_t*)p + 3;
        break;
    }
    case 0xc0:
    case 0xd0: { // 2 bytes
        *codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
        n = (uint8_t*)p + 2;
        break;
    }
    default:
    {
        *codepoint = -1;
        n = n + 1;
    }
    }
    if (*codepoint > SELENE_MAX_UNICODE) *codepoint = -1;
    return n;
}

#endif /* HELPER_H_ */