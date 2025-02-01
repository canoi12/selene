#ifndef SELENE_H_
#define SELENE_H_

#define SELENE_VERSION "0.2.1"

#include <platforms.h>
#include <common.h>

#if defined(BUILD_LIB_AS_DLL)
    #if defined(OS_WIN)
        #define SELENE_API __declspec(dllexport)
    #else
        #define SELENE_API extern
    #endif
#else
    #define SELENE_API
#endif

#if SELENE_USE_JIT
#define lua_rawlen lua_objlen
extern void lua_rawsetp(lua_State* L, int idx, void* p);
extern int lua_rawgetp(lua_State* L, int idx, const void* p);
extern void luaL_requiref (lua_State *L, const char *modname,
                               lua_CFunction openf, int glb);
#endif

typedef unsigned int Data;

#ifndef SELENE_NO_IMAGE
enum {
    SELENE_PIXEL_NONE = -1,
    SELENE_PIXEL_FANTASY,
    SELENE_PIXEL_GREY,
    SELENE_PIXEL_RG,
    SELENE_PIXEL_RGB,
    SELENE_PIXEL_RGBA,
    SELENE_PIXEL_BGR,
    SELENE_PIXEL_BGRA
};
#endif/* SELENE_NO_IMAGE */

#ifndef SELENE_NO_AUDIO

#ifndef SELENE_AUDIO_SAMPLE_RATE
#define SELENE_AUDIO_SAMPLE_RATE 44100
#endif

#ifndef SELENE_AUDIO_FORMAT
#define SELENE_AUDIO_FORMAT AUDIO_S16SYS
#endif

#ifndef SELENE_AUDIO_CHANNELS
#define SELENE_AUDIO_CHANNELS 2
#endif

#ifndef SELENE_AUDIO_SAMPLES
#define SELENE_AUDIO_SAMPLES 4096
#endif

enum {
    SELENE_UNKNOWN_AUDIO_FORMAT = 0,
    SELENE_WAV_FORMAT,
    SELENE_OGG_FORMAT,
    SELENE_MP3_FORMAT,
    SELENE_FLAC_FORMAT,
};

typedef struct AudioDecoder AudioDecoder;

typedef struct AudioInfo AudioInfo;
struct AudioInfo {
    int sample_rate;
    int channels;
    int format;
    int bit_depth;
    int size;
    int frame_count;
};
#endif/* SELENE_NO_AUDIO */

#ifndef SELENE_NO_FONT
typedef struct {
    int ax, ay;
    int bl, bt;
    int bw, bh;
    int tx;
} FontGlyph;
#endif /* SELENE_NO_FONT */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Open selene lib
 */
SELENE_API int luaopen_selene(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_H_ */
