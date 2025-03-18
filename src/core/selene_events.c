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
            // lua_pushinteger(L, event->key.scancode);
            lua_pushstring(L, SDL_GetScancodeName(event->key.scancode));
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
        /* Keyboard events */
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            lua_pushstring(L, "key");
            //lua_pushinteger(L, event->key.keysym.scancode);
            lua_pushstring(L, SDL_GetScancodeName(event->key.keysym.scancode));
            lua_pushboolean(L, event->type == SDL_KEYDOWN);
            lua_pushboolean(L, event->key.repeat);
            return 4;
        }
        case SDL_TEXTEDITING: {
            lua_pushstring(L, "text edit");
            lua_pushstring(L, event->edit.text);
            lua_pushinteger(L, event->edit.start);
            lua_pushinteger(L, event->edit.length);
            return 4;
        }
        case SDL_TEXTINPUT: {
            lua_pushstring(L, "text");
            lua_pushstring(L, event->text.text);
            return 2;
        }
        /* Mouse events */
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
        /* Joystick events */
        case SDL_JOYDEVICEADDED: {
            lua_pushstring(L, "joystick added");
            lua_pushinteger(L, event->jdevice.which);
            return 2;
        }
        case SDL_JOYDEVICEREMOVED: {
            lua_pushstring(L, "joystick removed");
            lua_pushinteger(L, event->jdevice.which);
            return 2;
        }
        case SDL_JOYAXISMOTION: {
            lua_pushstring(L, "joystick axis");
            lua_pushinteger(L, event->jaxis.which);
            lua_pushinteger(L, event->jaxis.axis);
            lua_pushinteger(L, event->jaxis.value);
            return 4;
        }
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN: {
            lua_pushstring(L, "joystick button");
            lua_pushinteger(L, event->jbutton.which);
            lua_pushinteger(L, event->jbutton.button);
            lua_pushboolean(L, event->jbutton.state == SDL_PRESSED);
            return 4;
        }
        case SDL_JOYHATMOTION: {
            lua_pushstring(L, "joystick hat");
            lua_pushinteger(L, event->jhat.which);
            lua_pushinteger(L, event->jhat.hat);
            lua_pushinteger(L, event->jhat.value);
            return 4;
        }
        case SDL_JOYBALLMOTION: {
            lua_pushstring(L, "joystick ball");
            lua_pushinteger(L, event->jball.which);
            lua_pushinteger(L, event->jball.ball);
            lua_pushinteger(L, event->jball.xrel);
            lua_pushinteger(L, event->jball.yrel);
            return 5;
        }
        case SDL_JOYBATTERYUPDATED: {
            lua_pushstring(L, "joystick battery");
            lua_pushinteger(L, event->jbattery.which);
            const char* battery_level[] = {"unknown", "empty", "low", "medium", "full", "wired"};
            lua_pushstring(L, battery_level[event->jbattery.level+1]);
            return 3;
        }
        /* Gamepad events */
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
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERBUTTONDOWN: {
            lua_pushstring(L, "gamepad button");
            lua_pushinteger(L, event->cbutton.which);
            lua_pushstring(L, SDL_GameControllerGetStringForButton(event->cbutton.button));
            lua_pushboolean(L, event->cbutton.state == SDL_PRESSED);
            return 4;
        }
        case SDL_CONTROLLERAXISMOTION: {
            lua_pushstring(L, "gamepad axis");
            lua_pushinteger(L, event->caxis.which);
            lua_pushstring(L, SDL_GameControllerGetStringForAxis(event->caxis.axis));
            lua_pushinteger(L, event->caxis.value);
            return 4;
        }
        /* Audio events */
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
        /* Finger events */
        case SDL_FINGERUP:
        case SDL_FINGERDOWN: {
            lua_pushstring(L, "touch finger");
            lua_pushinteger(L, event->tfinger.touchId);
            lua_pushinteger(L, event->tfinger.fingerId);
            lua_pushinteger(L, event->tfinger.x);
            lua_pushinteger(L, event->tfinger.y);
            lua_pushinteger(L, event->tfinger.pressure);
            return 6;
        }
        case SDL_FINGERMOTION: {
            lua_pushstring(L, "touch finger motion");
            lua_pushinteger(L, event->tfinger.touchId);
            lua_pushinteger(L, event->tfinger.fingerId);
            lua_pushinteger(L, event->tfinger.x);
            lua_pushinteger(L, event->tfinger.y);
            lua_pushinteger(L, event->tfinger.dx);
            lua_pushinteger(L, event->tfinger.dy);
            lua_pushinteger(L, event->tfinger.pressure);
            return 8;
        }
        /* Gesture events */
        case SDL_DOLLARGESTURE: {
            lua_pushstring(L, "dollar gesture");
            lua_pushinteger(L, event->dgesture.touchId);
            lua_pushinteger(L, event->dgesture.gestureId);
            lua_pushinteger(L, event->dgesture.numFingers);
            lua_pushnumber(L, event->dgesture.error);
            lua_pushinteger(L, event->dgesture.x);
            lua_pushinteger(L, event->dgesture.y);
            return 7;
        }
        case SDL_DOLLARRECORD: {
            lua_pushstring(L, "dollar record");
            lua_pushinteger(L, event->dgesture.touchId);
            lua_pushinteger(L, event->dgesture.gestureId);
            lua_pushinteger(L, event->dgesture.numFingers);
            lua_pushnumber(L, event->dgesture.error);
            lua_pushinteger(L, event->dgesture.x);
            lua_pushinteger(L, event->dgesture.y);
            return 7;
        }
        /* Drop events */
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
        case SDL_DROPBEGIN:
            lua_pushstring(L, "drop begin");
            return 1;
        case SDL_DROPCOMPLETE:
            lua_pushstring(L, "drop complete");
            return 1;
        /* Clipboard events */
        case SDL_CLIPBOARDUPDATE: {
            lua_pushstring(L, "clipboard update");
            return 1;
        }
        /* Window events */
        case SDL_WINDOWEVENT: {
            switch (event->window.event) {
                case SDL_WINDOWEVENT_SHOWN:
                    lua_pushstring(L, "window shown");
                    return 1;
                case SDL_WINDOWEVENT_HIDDEN:
                    lua_pushstring(L, "window hidden");
                    return 1;
                case SDL_WINDOWEVENT_EXPOSED:
                    lua_pushstring(L, "window exposed");
                    return 1;
                case SDL_WINDOWEVENT_MOVED: {
                    lua_pushstring(L, "window moved");
                    lua_pushinteger(L, event->window.data1);
                    lua_pushinteger(L, event->window.data2);
                    return 3;
                }
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                case SDL_WINDOWEVENT_RESIZED: {
                    lua_pushstring(L, "window resized");
                    lua_pushinteger(L, event->window.data1);
                    lua_pushinteger(L, event->window.data2);
                    return 3;
                }
                case SDL_WINDOWEVENT_MINIMIZED:
                    lua_pushstring(L, "window minimized");
                    return 1;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    lua_pushstring(L, "window maximized");
                    return 1;
                case SDL_WINDOWEVENT_RESTORED:
                    lua_pushstring(L, "window restored");
                    return 1;
                case SDL_WINDOWEVENT_ENTER:
                    lua_pushstring(L, "mouse entered");
                    return 1;
                case SDL_WINDOWEVENT_LEAVE:
                    lua_pushstring(L, "mouse leaved");
                    return 1;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    lua_pushstring(L, "focus gained");
                    return 1;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    lua_pushstring(L, "focus lost");
                    return 1;
                case SDL_WINDOWEVENT_CLOSE: {
                    lua_pushstring(L, "window closed");
                    return 1;
                }
            }
        }
    }
    return 0;
}
#endif
