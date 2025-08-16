#include "selene.h"
#include "lua_helper.h"

/*extern int selene_init(void** userdata, int argc, char** argv);
extern int selene_event(void* userdata, SDL_Event* event);
extern int selene_iterate(void* userdata);
extern void selene_quit(void* userdata, int result);*/

static int main_symbol_test(int a) { return -1; }

#if 1 // !defined(SELENE_USE_SDL3) || defined(OS_ANDROID)

SELENE_API int selene_main(int argc, char** argv);

#if !defined(OS_ANDROID)
    int main(int argc, char** argv) { return selene_main(argc, argv); }
#endif /* OS_ANDROID */

#elif defined(SELENE_USE_SDL3)
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
    lua_State* L = luaL_newstate();
    *appstate = (void*)L;
    return selene_init(appstate, argc, argv);
}
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    selene_quit(appstate, result);
    lua_close((lua_State*)appstate);
}
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    return selene_event(appstate, event);
}
SDL_AppResult SDL_AppIterate(void* appstate) {
    return selene_iterate(appstate);
}
#endif
