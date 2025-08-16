#ifndef SELENE_H_
#define SELENE_H_

#include "selene_config.h"

#include <platforms.h>
#include <common.h>

#ifndef SELENE_API
    #if defined(OS_WIN)
        #if defined(SELENE_EXPORT_DLL)
            #define SELENE_API __declspec(dllexport)
        #elif defined(SELENE_IMPORT_DLL)
            #define SELENE_API __declspec(dllimport)
        #else
            #define SELENE_API 
        #endif
    #else
        #define SELENE_API extern
    #endif
#endif

// #define NEW_META(type, name)\
// type* name = (type*)lua_newuserdata(L, sizeof(*name));\
// luaL_setmetatable(L, type##_METANAME)

// #define META_SELF(type)\
// int arg = 1;\
// type* self = (type*)luaL_checkudata(L, arg++, type##_METANAME)



#if defined(__cplusplus)
extern "C" {
#endif

#if SELENE_USE_JIT
#define lua_rawlen lua_objlen
extern void lua_rawsetp(lua_State* L, int idx, void* p);
extern int lua_rawgetp(lua_State* L, int idx, const void* p);
extern void luaL_requiref (lua_State *L, const char *modname,
                               lua_CFunction openf, int glb);
#endif

#define SELENE_APP_CONTINUE 0
#define SELENE_APP_SUCCESS  1
#define SELENE_APP_FAILURE  2

typedef unsigned int Data;
#ifndef Data_METANAME
#define Data_METANAME "Data"
#endif

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
#if defined(SELENE_USE_SDL3)
#define SELENE_AUDIO_FORMAT SDL_AUDIO_S16
#else
#define SELENE_AUDIO_FORMAT AUDIO_S16SYS
#endif
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

typedef struct _AudioDecoder selene_AudioDecoder;

typedef struct _AudioInfo AudioInfo;
struct _AudioInfo {
    int sample_rate;
    int channels;
    int format;
    int bit_depth;
    int size;
    int frame_count;
};
#endif/* SELENE_NO_AUDIO */

#ifndef SELENE_NO_FONT
typedef struct _FontGlyph selene_FontGlyph;
struct _FontGlyph {
    int ax, ay;
    int bl, bt;
    int bw, bh;
    int tx;
};
#ifndef selene_FontGlyph_METANAME
#define selene_FontGlyph_METANAME "FontGlyph"
#endif
#endif /* SELENE_NO_FONT */

typedef struct {
    int is_running;

    int l_step_callback_ref;
    int l_quit_callback_ref;
    int l_event_callback_ref;
} SeleneContext;

extern SeleneContext g_selene_context;

typedef struct selene_File selene_File;
struct selene_File {
#if defined(SELENE_USE_SDL3)
    SDL_IOStream* handle;
#else
    SDL_RWops* handle;
#endif
};

typedef struct _Renderer selene_Renderer;
typedef struct _Window selene_Window;

// Mask for the initiated SDL modules
extern int g_sdl_modules;

/**
 * Open selene lib
 */
SELENE_API int luaopen_selene(lua_State* L);
SELENE_API int selene_main(int argc, char** argv);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_H_ */
