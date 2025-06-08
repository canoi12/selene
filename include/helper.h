#ifndef HELPER_H_
#define HELPER_H_

#include "selene.h"
#include "common.h"

inline void char_rect(selene_FontGlyph* glyphs, const int c, float *x, float *y, int* out_pos, int* out_rect, int width, int line_height) {
    if (c == '\n') {
        *x = 0;
        *y += line_height;
        return;
    }

    if (c == '\t') {
        *x += glyphs[c].bw * 2;
        return;
    }
    if (width != 0 && *x + (glyphs[c].bl) > width) {
        *x = 0;
        *y += line_height;
    }

    float x2 = *x + (float)glyphs[c].bl;
    float y2 = *y + (float)glyphs[c].bt;

    float s0 = glyphs[c].tx;
    float t0 = 0;
    int s1 = glyphs[c].bw;
    int t1 = glyphs[c].bh;

    *x += glyphs[c].ax;
    *y += glyphs[c].ay;

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


inline int utf8_codepoint(uint8_t* p, int* codepoint) {
    //uint8_t* n = NULL;
    int n = 0;
    uint8_t c = *p;
    if (c < 0x80) { // Se for um ascii normal, o codepoint é o próprio caractere
        *codepoint = c;
        return 1;
    }

    switch (c & 0xf0) { // Checa o primeiro byte para conferir quantos bytes precisam ser usados
    case 0xf0: { // 4 bytes
        *codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
        //n = (uint8_t*)p + 4;
        n = 4;
        break;
    }
    case 0xe0: { // 3 bytes
        *codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
        //n = (uint8_t*)p + 3;
        n = 3;
        break;
    }
    case 0xc0:
    case 0xd0: { // 2 bytes
        *codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
        //n = (uint8_t*)p + 2;
        n = 2;
        break;
    }
    default:
    {
        *codepoint = -1;
        n = 1;
        //n = n + 1;
    }
    }
    if (*codepoint > SELENE_MAX_UNICODE) *codepoint = -1;
    return n;
}

#endif /* HELPER_H_ */
