#ifndef SELENE_RUNTIME_H_
#define SELENE_RUNTIME_H_

/*
 * Selene runtime entry points (src/selene_main.c)
 *
 * The runtime boots a game (directory, zip, fused executable or main.lua),
 * and drives the main loop. Front-ends (src/main.c for the plain runtime,
 * src/editor/main.cpp for the editor) are thin wrappers around these.
 */

#include "selene.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Opens the libs, resolves the game from argv and runs boot.lua.
 * *userdata must hold a lua_State* created by the caller. */
SELENE_API int selene_init(void** userdata, int argc, char** argv);
SELENE_API int selene_iterate(void* userdata);
SELENE_API int selene_event(void* userdata, SDL_Event* event);
SELENE_API void selene_quit(void* userdata, int status);
SELENE_API int selene_main_loop(lua_State* L);

/* Full run: create state, init, loop, quit. Returns the process exit code. */
SELENE_API int selene_main(int argc, char** argv);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_RUNTIME_H_ */
