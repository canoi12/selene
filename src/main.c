/* Runtime front-end: boots the game found by selene_init (see selene_main.c) */
#include "selene_runtime.h"

#if 1 // !defined(SELENE_USE_SDL3) || defined(OS_ANDROID)

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
