#ifndef SELENE_WINDOW_H_
#define SELENE_WINDOW_H_

#include "selene.h"

typedef struct _Window selene_Window;

struct _WindowConfig {
    char resizable;
    char borderless;
    char fullscreen;
    char opengl;
};

struct _Window {
    int r_table_info; // Lua register for the table of informations
    char title[128];
    int width, height;
#if 1 || defined(SELENE_USE_SDL2) || defined(SELENE_USE_SDL3)
    SDL_Window* handle;
#elif defined(SELENE_WINDOW_GLFW)
    GLFWWindow* handle;
#else
    void* handle;
#endif
    char resizable;
    char borderless;
    char opengl;
    char fullscreen;
};

#ifndef selene_Window_METANAME
    #define selene_Window_METANAME "selene.Window"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

int init_window(selene_Window*, const char* title, int width, int height, struct _WindowConfig* conf);
int quit_window(selene_Window*);

SELENE_API int luaopen_window(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_WINDOW_H_ */
