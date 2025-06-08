#ifndef HELPER_H_
#define HELPER_H_

#include "selene.h"
#include "common.h"

extern void char_rect(selene_FontGlyph* glyphs, const int c, float *x, float *y, int* out_pos, int* out_rect, int width, int line_height);
extern int utf8_codepoint(uint8_t* p, int* codepoint);

#endif /* HELPER_H_ */
