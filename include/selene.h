#ifndef SELENE_H_
#define SELENE_H_

#define SELENE_VERSION "0.1.0"

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

static const char* selene_init_script =
"selene.set_running(true)\n"
#if defined(OS_EMSCRIPTEN)
"selene.__exec = './'\n"
"selene.__dir = './'\n"
#else
"selene.__exec = selene.get_exec_path()\n"
"local path = selene.__exec\n"
"if path then\n"
#if defined(__linux__)
    "package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path\n"
    "package.cpath = path .. '/?.so;' .. package.cpath\n"
#endif
    "selene.__dir = './'\n"
    "if selene.args[2] and selene.args[3] then\n"
    "   if selene.args[2] == '-d' then\n"
    "       package.path = selene.args[3] .. '/?.lua;' .. package.path\n"
    "       selene.__dir = selene.args[3]\n"
    "   end\n"
    "end\n"
"end\n"
#endif
#if 1
"local status, err = pcall(function() require('main') end)\n"
"if not status then\n"
"   selene.set_running(false)\n"
"   error(debug.traceback(err, 1), 2)"
"end";
#else
"require('plugins').setup()\n"
"local status, err = pcall(function() require('cube') end)\n"
"if not status then\n"
"   error(err)\n"
"end";
#endif

extern int selene_running;

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
#endif

#ifndef SELENE_NO_AUDIO
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
#endif

#ifndef SELENE_NO_FONT
typedef struct {
    int ax, ay;
    int bl, bt;
    int bw, bh;
    int tx;
} FontGlyph;
#endif

#if defined(__cplusplus)
extern "C" {
#endif
/**
 * Open selene lib
 */
SELENE_API int luaopen_selene(lua_State* L);

#if !defined(SELENE_NO_STEP_FUNC)
/**
 * Run selene step function callback (thrown an error if none set)
 */
SELENE_API void selene_run_step(lua_State* L);
#endif
#if !defined(SELENE_NO_QUIT_FUNC)
/**
 * Run selene quit function callback
 */
SELENE_API void selene_run_quit(lua_State* L);
#endif

SELENE_API int selene_main(int argc, char** argv);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_H_ */
