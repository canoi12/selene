#include "selene.h"
#include "renderer/renderer.h"

#if defined(SELENE_USE_SDL3)
int g_selene_process_events(lua_State* L, SDL_Event* event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: {
            lua_pushstring(L, "quit");
            return 1;
        }
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN: {
            lua_pushstring(L, "key");
            lua_pushinteger(L, event->key.scancode);
            lua_pushboolean(L, event->key.down);
            lua_pushboolean(L, event->key.repeat);
            return 4;
        }
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            lua_pushstring(L, "window closed");
            return 1;
        }
        case SDL_EVENT_WINDOW_MOVED: {
            lua_pushstring(L, "window moved");
            lua_pushinteger(L, event->window.data1);
            lua_pushinteger(L, event->window.data2);
            return 3;
        }
    }
    return 0;
}
#else
int g_selene_process_event(lua_State* L, SDL_Event* event) {
    switch (event->type) {
        case SDL_QUIT: {
            lua_pushstring(L, "quit");
            return 1;
        }
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            lua_pushstring(L, "key");
            lua_pushinteger(L, event->key.keysym.scancode);
            // lua_pushstring(L, SDL_GetScancodeName(event->key.keysym.scancode));
            lua_pushboolean(L, event->type == SDL_KEYDOWN);
            lua_pushboolean(L, event->key.repeat);
            return 4;
        }
        case SDL_MOUSEMOTION: {
            lua_pushstring(L, "mouse moved");
            lua_pushinteger(L, event->motion.x);
            lua_pushinteger(L, event->motion.y);
            lua_pushinteger(L, event->motion.xrel);
            lua_pushinteger(L, event->motion.yrel);
            return 5;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            lua_pushstring(L, "mouse button");
            lua_pushinteger(L, event->button.button);
            lua_pushboolean(L, event->type == SDL_MOUSEBUTTONDOWN);
            lua_pushinteger(L, event->button.x);
            lua_pushinteger(L, event->button.y);
            return 5;
        }
        case SDL_MOUSEWHEEL: {
            lua_pushstring(L, "mouse wheel");
            lua_pushinteger(L, event->wheel.x);
            lua_pushinteger(L, event->wheel.y);
            return 3;
        }
        case SDL_CONTROLLERDEVICEADDED: {
            lua_pushstring(L, "gamepad added");
            lua_pushinteger(L, event->cdevice.which);
            return 2;
        }
        case SDL_CONTROLLERDEVICEREMOVED: {
            lua_pushstring(L, "gamepad removed");
            lua_pushinteger(L, event->cdevice.which);
            return 2;
        }
        case SDL_AUDIODEVICEADDED: {
            lua_pushstring(L, "audio device added");
            lua_pushinteger(L, event->adevice.which);
            lua_pushboolean(L, event->adevice.iscapture);
            return 3;
        }
        case SDL_AUDIODEVICEREMOVED: {
            lua_pushstring(L, "audio device removed");
            lua_pushinteger(L, event->adevice.which);
            lua_pushboolean(L, event->adevice.iscapture);
            return 2;
        }
        case SDL_DROPFILE: {
            lua_pushstring(L, "drop file");
            lua_pushstring(L, event->drop.file);
            // free(event->drop.file);
            return 2;
        }
        case SDL_DROPTEXT: {
            lua_pushstring(L, "drop text");
            lua_pushstring(L, event->drop.file);
            // free(event->drop.file);
            return 2;
        }
        case SDL_WINDOWEVENT: {
            switch (event->window.event) {
                case SDL_WINDOWEVENT_CLOSE: {
                    lua_pushstring(L, "window closed");
                    return 1;
                }
                case SDL_WINDOWEVENT_MOVED: {
                    lua_pushstring(L, "window moved");
                    lua_pushinteger(L, event->window.data1);
                    lua_pushinteger(L, event->window.data2);
                    return 3;
                }
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    lua_pushstring(L, "window focused");
                    return 1;
                case SDL_WINDOWEVENT_ENTER:
                    lua_pushstring(L, "window entered");
                    return 1;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    lua_pushstring(L, "window unfocused");
                    return 1;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    lua_pushstring(L, "window maximized");
                    return 1;
                case SDL_WINDOWEVENT_MINIMIZED:
                    lua_pushstring(L, "window minimized");
                    return 1;
                case SDL_WINDOWEVENT_RESTORED:
                    lua_pushstring(L, "window restored");
                    return 1;
                case SDL_WINDOWEVENT_RESIZED: {
                    lua_pushstring(L, "window resized");
                    lua_pushinteger(L, event->window.data1);
                    lua_pushinteger(L, event->window.data2);
                    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_renderer_ref);
                    Renderer* r = (Renderer*) lua_touserdata(L, -1);
                    lua_pop(L, 1);
                    if (r && r->on_resize) r->on_resize(r, L, event->window.data1, event->window.data2);
                    return 3;
                }
            }
        }
    }
    return 0;
}
#endif