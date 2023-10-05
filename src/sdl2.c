#include "SDL_audio.h"
#include "SDL_events.h"
#include "SDL_joystick.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "lauxlib.h"
#include "lua.h"
#include "selene.h"



static const Uint8* keys;
/************************
 #                      #
 #        Audio         #
 #                      #
 ************************/

static BEGIN_FUNCTION(sdl2, OpenAudio)
    int type = lua_type(L, 1);
    if (type != LUA_TTABLE)
        return luaL_argerror(L, 1, "Table expected");
    lua_getfield(L, 1, "sample_rate");
    int freq = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 1, "channels");
    int channels = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 1, "samples");
    int samples = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    SDL_AudioSpec des;
    des.freq = freq;
    des.channels = channels;
    des.samples = samples;
    des.format = AUDIO_S16;

    lua_pushboolean(L, SDL_OpenAudio(&des, NULL) == 0);
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, PauseAudio)
    INIT_ARG();
    GET_BOOLEAN(pause);
    SDL_PauseAudio(pause);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, CloseAudio)
    SDL_CloseAudio();
END_FUNCTION(0)

/************************
 #                      #
 #      GLContext       #
 #                      #
 ************************/

static BEGIN_META(SDL_GLContext)
    BEGIN_REG(SDL_GLContext)
    END_REG()
    NEW_META(SDL_GLContext);
END_META(1)

/************************
 #                      #
 #        Window        #
 #                      #
 ************************/
static BEGIN_META_FUNCTION(SDL_Window, GetSize, *win)
    int width, height;
    SDL_GetWindowSize(*win, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
END_FUNCTION(2)

static BEGIN_META_FUNCTION(SDL_Window, SetSize, *win)
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    SDL_SetWindowSize(*win, width, height);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(SDL_Window, GetPosition, *win)
    int x, y;
    SDL_GetWindowPosition(*win, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
END_FUNCTION(2)

static BEGIN_META_FUNCTION(SDL_Window, SetPosition, *win)
    CHECK_INTEGER(x);
    CHECK_INTEGER(y);
    SDL_SetWindowPosition(*win, x, y);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(SDL_Window, Swap, *win)
    SDL_GL_SwapWindow(*win);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(SDL_Window, SetBordered, *win)
    GET_BOOLEAN(value);
    SDL_SetWindowBordered(*win, value);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(SDL_Window, Maximize, *win)
    SDL_MaximizeWindow(*win);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(SDL_Window, Minimize, *win)
    SDL_MinimizeWindow(*win);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(SDL_Window, Restore, *win)
    SDL_RestoreWindow(*win);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(SDL_Window, ShowSimpleMessageBox, *win)
END_FUNCTION(0)

static BEGIN_META(SDL_Window)
    BEGIN_REG(SDL_Window)
        REG_META_FIELD(SDL_Window, GetSize),
        REG_META_FIELD(SDL_Window, SetSize),
        REG_META_FIELD(SDL_Window, GetPosition),
        REG_META_FIELD(SDL_Window, SetPosition),
        REG_META_FIELD(SDL_Window, Swap),
        REG_META_FIELD(SDL_Window, SetBordered),
        REG_META_FIELD(SDL_Window, Maximize),
        REG_META_FIELD(SDL_Window, Minimize),
        REG_META_FIELD(SDL_Window, Restore),
        REG_META_FIELD(SDL_Window, ShowSimpleMessageBox),
    END_REG()
    NEW_META(SDL_Window);
END_META(1)

static BEGIN_FUNCTION(sdl2, CreateWindow)
    INIT_ARG();
    CHECK_STRING(title);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    NEW_UDATA(SDL_Window, *window, sizeof(void*));
    *window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        flags
    );
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, DestroyWindow)
    INIT_GET_UDATA(SDL_Window, *window);
    SDL_DestroyWindow(*window);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GL_SetAttribute)
    INIT_ARG();
    CHECK_INTEGER(attr);
    CHECK_INTEGER(value);
    SDL_GL_SetAttribute(attr, value);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GL_CreateContext)
    INIT_GET_UDATA(SDL_Window, *win);
    NEW_UDATA(SDL_GLContext, ctx, sizeof(void*));
    *ctx = SDL_GL_CreateContext(*win);
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, GL_DeleteContext)
    SDL_GLContext* ctx = luaL_checkudata(L, 1, "SDL_GLContext");
    SDL_GL_DeleteContext(*ctx);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GL_MakeCurrent)
    INIT_GET_UDATA(SDL_GLContext, ctx);
    GET_UDATA(SDL_Window, *window);
    SDL_GL_MakeCurrent(*window, *ctx);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GetScancodeFromName)
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GetScancodeFromName(name));
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, GetScancodeName)
    INIT_ARG();
    CHECK_INTEGER(scancode);
    PUSH_STRING(SDL_GetScancodeName(scancode));
END_FUNCTION(1)

/************************
 #                      #
 #        Event         #
 #                      #
 ************************/

static BEGIN_META_FUNCTION(SDL_Event, Poll, ev)
    PUSH_BOOLEAN(SDL_PollEvent(ev));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Event, GetType, ev)
    PUSH_INTEGER(ev->type);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Event, WindowEvent, ev)
    PUSH_INTEGER(ev->window.event);
    PUSH_INTEGER(ev->window.windowID);
    PUSH_INTEGER(ev->window.data1);
    PUSH_INTEGER(ev->window.data2);
END_FUNCTION(4)

static BEGIN_META_FUNCTION(SDL_Event, MouseMotionEvent, ev)
    PUSH_INTEGER(ev->motion.which);
    PUSH_INTEGER(ev->motion.state);
    PUSH_INTEGER(ev->motion.x);
    PUSH_INTEGER(ev->motion.y);
    PUSH_INTEGER(ev->motion.xrel);
    PUSH_INTEGER(ev->motion.yrel);
END_FUNCTION(6)

static BEGIN_META_FUNCTION(SDL_Event, MouseButtonEvent, ev)
    PUSH_INTEGER(ev->button.which);
    PUSH_INTEGER(ev->button.button);
    PUSH_INTEGER(ev->button.x);
    PUSH_INTEGER(ev->button.y);
    PUSH_INTEGER(ev->button.clicks);
END_FUNCTION(5)

static BEGIN_META_FUNCTION(SDL_Event, KeyboardEvent, ev)
    PUSH_STRING(SDL_GetScancodeName(ev->key.keysym.scancode));
    PUSH_BOOLEAN(ev->key.repeat);
END_FUNCTION(2)

static BEGIN_META(SDL_Event)
    BEGIN_REG(SDL_Event)
        REG_META_FIELD(SDL_Event, Poll),
        REG_META_FIELD(SDL_Event, GetType),
        REG_META_FIELD(SDL_Event, WindowEvent),
        REG_META_FIELD(SDL_Event, MouseMotionEvent),
        REG_META_FIELD(SDL_Event, MouseButtonEvent),
        REG_META_FIELD(SDL_Event, KeyboardEvent),
    END_REG()
    NEW_META(SDL_Event);
END_META(1)

static int l_sdl2_NewEvent(lua_State* L) {
    SDL_Event* ev = lua_newuserdata(L, sizeof(*ev));
    luaL_setmetatable(L, "SDL_Event");
    return 1;
}

/************************
 #                      #
 #       Keyboard       #
 #                      #
 ************************/
static BEGIN_FUNCTION(sdl2, CheckKeyState)
    INIT_ARG();
    CHECK_INTEGER(key);
    PUSH_BOOLEAN(keys[key]);
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, HasScreenKeyboardSupport)
    PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, IsScreenKeyboardShown)
    INIT_GET_UDATA(SDL_Window, *win);
    PUSH_BOOLEAN(SDL_IsScreenKeyboardShown(*win));
END_FUNCTION(1)

/************************
 #                      #
 #         Mouse        #
 #                      #
 ************************/

static BEGIN_FUNCTION(sdl2, GetMousePosition)
    int x, y;
    SDL_GetMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
END_FUNCTION(2)

static BEGIN_FUNCTION(sdl2, GetRelativeMousePosition)
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
END_FUNCTION(2)

static BEGIN_FUNCTION(sdl2, IsMouseDown)
    INIT_ARG();
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
END_FUNCTION(1)

/************************
 #                      #
 #       Joystick       #
 #                      #
 ************************/

static BEGIN_FUNCTION(sdl2, JoystickOpen)
    SDL_Joystick* joy = SDL_JoystickOpen((int)luaL_checknumber(L, 1));
    if (joy) {
        NEW_UDATA(SDL_Joystick, *j, sizeof(void*));
        *j = joy;
    } else PUSH_NIL();
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, NumJoysticks)
    PUSH_INTEGER(SDL_NumJoysticks());
END_FUNCTION(1)

// Meta functions

static BEGIN_META_FUNCTION(SDL_Joystick, GetName, *j)
    PUSH_STRING(SDL_JoystickName(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, GetVendor, *j)
    PUSH_INTEGER(SDL_JoystickGetVendor(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, GetProduct, *j)
    PUSH_INTEGER(SDL_JoystickGetProduct(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, GetProductVersion, *j)
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, GetType, *j)
    PUSH_INTEGER(SDL_JoystickGetType(*j));
END_FUNCTION(1)

const char *joy_types[] = {
    [SDL_JOYSTICK_TYPE_UNKNOWN] = "Unknown",
    [SDL_JOYSTICK_TYPE_GAMECONTROLLER] = "GameController",
    [SDL_JOYSTICK_TYPE_WHEEL] = "Wheel",
    [SDL_JOYSTICK_TYPE_ARCADE_STICK] = "Arcadestick",
    [SDL_JOYSTICK_TYPE_FLIGHT_STICK] = "FlightStick",
    [SDL_JOYSTICK_TYPE_DANCE_PAD] = "DancePad",
    [SDL_JOYSTICK_TYPE_GUITAR] = "Guitar",
    [SDL_JOYSTICK_TYPE_DRUM_KIT] = "DrumKit",
    [SDL_JOYSTICK_TYPE_ARCADE_PAD] = "ArcadePad",
    [SDL_JOYSTICK_TYPE_THROTTLE] = "Throttle",
};

static BEGIN_META_FUNCTION(SDL_Joystick, GetTypeString, *j)
    PUSH_STRING(joy_types[SDL_JoystickGetType(*j)]);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, NumAxes, *j)
    PUSH_INTEGER(SDL_JoystickNumAxes(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, NumBalls, *j)
    PUSH_INTEGER(SDL_JoystickNumBalls(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, NumHats, *j)
    PUSH_INTEGER(SDL_JoystickNumHats(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, NumButtons, *j)
    PUSH_INTEGER(SDL_JoystickNumButtons(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, GetAxis, *j)
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*j, axis));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, GetBall, *j)
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*j, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
END_FUNCTION(2)

static BEGIN_META_FUNCTION(SDL_Joystick, GetHat, *j)
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*j, hat));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, GetButton, *j)
    CHECK_INTEGER(button);
    PUSH_INTEGER(SDL_JoystickGetButton(*j, button));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, Rumble, *j)
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*j, low, high, freq) == 0);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, CurrentPowerLevel, *j)
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*j));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_Joystick, Close, *j)
    if (SDL_JoystickGetAttached(*j))
        SDL_JoystickClose(*j);
END_FUNCTION(0)

static BEGIN_META(SDL_Joystick)
    BEGIN_REG(SDL_Joystick)
        REG_META_FIELD(SDL_Joystick, GetName),
        REG_META_FIELD(SDL_Joystick, GetVendor),
        REG_META_FIELD(SDL_Joystick, GetProduct),
        REG_META_FIELD(SDL_Joystick, GetProductVersion),
        REG_META_FIELD(SDL_Joystick, GetType),
        REG_META_FIELD(SDL_Joystick, GetTypeString),
        REG_META_FIELD(SDL_Joystick, NumAxes),
        REG_META_FIELD(SDL_Joystick, NumBalls),
        REG_META_FIELD(SDL_Joystick, NumHats),
        REG_META_FIELD(SDL_Joystick, NumButtons),
        REG_META_FIELD(SDL_Joystick, GetAxis),
        REG_META_FIELD(SDL_Joystick, GetBall),
        REG_META_FIELD(SDL_Joystick, GetHat),
        REG_META_FIELD(SDL_Joystick, GetButton),
        REG_META_FIELD(SDL_Joystick, Rumble),
        REG_META_FIELD(SDL_Joystick, CurrentPowerLevel),
        REG_META_FIELD(SDL_Joystick, Close),
    END_REG();
    NEW_META(SDL_Joystick);
END_META(1)

/************************
 #                      #
 #    Game Controller   #
 #                      #
 ************************/
static int l_sdl2_GameControllerOpen(lua_State* L) {
    INIT_ARG();
    CHECK_INTEGER(index);
    SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (!ctrl) lua_pushnil(L);
    else {
        NEW_UDATA(SDL_GameController, *controller, sizeof(void*));
        *controller = ctrl;
    }
    return 1;
}

static int l_sdl2_IsGameController(lua_State* L) {
    PUSH_BOOLEAN(SDL_IsGameController((int)luaL_checkinteger(L, 1)));
    return 1;
}

static BEGIN_META_FUNCTION(SDL_GameController, GetName, *gc)
    PUSH_STRING(SDL_GameControllerName(*gc));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, GetVendor, *gc)
    PUSH_INTEGER(SDL_GameControllerGetVendor(*gc));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, GetProduct, *gc)
    PUSH_INTEGER(SDL_GameControllerGetProduct(*gc));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, GetProductVersion, *gc)
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*gc));
END_FUNCTION(1)

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static BEGIN_FUNCTION(sdl2, GameControllerGetAxisFromString)
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, GetAxisFromString, *gc)
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, GetAxis, *gc)
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*gc, axis));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, GetButtonFromString, *gc)
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, GameControllerGetButtonFromString)
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, GetButton, *gc)
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetButton(*gc, axis));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, Rumble, *gc)
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    OPT_INTEGER(freq, 100);
    PUSH_BOOLEAN(SDL_GameControllerRumble(*gc, low, high, freq) == 0);
END_FUNCTION(1)

const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};

static BEGIN_META_FUNCTION(SDL_GameController, CurrentPowerLevel, *gc)
    SDL_Joystick *j = SDL_GameControllerGetJoystick(*gc);
    PUSH_STRING(gpad_powerlevels[SDL_JoystickCurrentPowerLevel(j)+1]);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(SDL_GameController, Close, *gc)
    if (SDL_GameControllerGetAttached(*gc))
        SDL_GameControllerClose(*gc);
END_FUNCTION(0)

static BEGIN_META(SDL_GameController)
    BEGIN_REG(SDL_GameController)
        REG_META_FIELD(SDL_GameController, GetName),
        REG_META_FIELD(SDL_GameController, GetVendor),
        REG_META_FIELD(SDL_GameController, GetProduct),
        REG_META_FIELD(SDL_GameController, GetProductVersion),
        REG_META_FIELD(SDL_GameController, GetAxisFromString),
        REG_META_FIELD(SDL_GameController, GetAxis),
        REG_META_FIELD(SDL_GameController, GetButtonFromString),
        REG_META_FIELD(SDL_GameController, GetButton),
        REG_META_FIELD(SDL_GameController, Rumble),
        REG_META_FIELD(SDL_GameController, CurrentPowerLevel),
        REG_META_FIELD(SDL_GameController, Close),
    END_REG()
    NEW_META(SDL_GameController);
END_MODULE(1)

/************************
 #                      #
 #        Timer         #
 #                      #
 ************************/
static BEGIN_FUNCTION(sdl2, GetTicks)
    PUSH_INTEGER(SDL_GetTicks());
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, Delay)
    Uint32 ms = (Uint32)luaL_checkinteger(L, 1);
    SDL_Delay(ms);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GetPerformanceCounter)
    PUSH_INTEGER(SDL_GetPerformanceCounter());
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, GetPerformanceFrequency)
    PUSH_INTEGER(SDL_GetPerformanceFrequency());
END_FUNCTION(1)

/************************
 #                      #
 #        Enums         #
 #                      #
 ************************/
BEGIN_ENUM(sdl2)
    // Events
    ENUM_FIELD(QUIT, SDL_),
    ENUM_FIELD(WINDOWEVENT, SDL_),
    ENUM_FIELD(KEYDOWN, SDL_),
    ENUM_FIELD(KEYUP, SDL_),
    ENUM_FIELD(MOUSEMOTION, SDL_),
    ENUM_FIELD(MOUSEBUTTONDOWN, SDL_),
    ENUM_FIELD(MOUSEBUTTONUP, SDL_),

    // Window Events
    ENUM_FIELD(WINDOWEVENT_CLOSE, SDL_),
    ENUM_FIELD(WINDOWEVENT_SIZE_CHANGED, SDL_),
    ENUM_FIELD(WINDOWEVENT_RESIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MOVED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MINIMIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MAXIMIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_RESTORED, SDL_),
    ENUM_FIELD(WINDOWEVENT_SHOWN, SDL_),
    ENUM_FIELD(WINDOWEVENT_HIDDEN, SDL_),
    ENUM_FIELD(WINDOWEVENT_ENTER, SDL_),
    ENUM_FIELD(WINDOWEVENT_LEAVE, SDL_),
    ENUM_FIELD(WINDOWEVENT_FOCUS_GAINED, SDL_),
    ENUM_FIELD(WINDOWEVENT_FOCUS_LOST, SDL_),
    ENUM_FIELD(WINDOWEVENT_TAKE_FOCUS, SDL_),
    ENUM_FIELD(WINDOWEVENT_HIT_TEST, SDL_),

    // Window Flags
    ENUM_FIELD(WINDOW_RESIZABLE, SDL_),
    ENUM_FIELD(WINDOW_FULLSCREEN, SDL_),
    ENUM_FIELD(WINDOW_FULLSCREEN_DESKTOP, SDL_),
    ENUM_FIELD(WINDOW_BORDERLESS, SDL_),
    ENUM_FIELD(WINDOW_ALWAYS_ON_TOP, SDL_),
    // Message Box
    ENUM_FIELD(MESSAGEBOX_ERROR, SDL_),
    ENUM_FIELD(MESSAGEBOX_WARNING, SDL_),
    ENUM_FIELD(MESSAGEBOX_INFORMATION, SDL_),
    // GL
    ENUM_FIELD(GL_DOUBLEBUFFER, SDL_),
    ENUM_FIELD(GL_DEPTH_SIZE, SDL_),
    ENUM_FIELD(GL_STENCIL_SIZE, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_MASK, SDL_),
    ENUM_FIELD(GL_CONTEXT_MAJOR_VERSION, SDL_),
    ENUM_FIELD(GL_CONTEXT_MINOR_VERSION, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_CORE, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_COMPATIBILITY, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_ES, SDL_),
END_ENUM()

BEGIN_MODULE(sdl2)
    keys = SDL_GetKeyboardState(NULL);
    BEGIN_REG(sdl2)
        // Audio
        REG_FIELD(sdl2, OpenAudio),
        REG_FIELD(sdl2, PauseAudio),
        REG_FIELD(sdl2, CloseAudio),
        // Window
        REG_FIELD(sdl2, CreateWindow),
        REG_FIELD(sdl2, DestroyWindow),
        REG_FIELD(sdl2, GL_SetAttribute),
        REG_FIELD(sdl2, GL_CreateContext),
        REG_FIELD(sdl2, GL_MakeCurrent),
        REG_FIELD(sdl2, GL_DeleteContext),
        // Event
        REG_FIELD(sdl2, NewEvent),
        // Keyboard
        REG_FIELD(sdl2, GetScancodeFromName),
        REG_FIELD(sdl2, HasScreenKeyboardSupport),
        REG_FIELD(sdl2, IsScreenKeyboardShown),
        REG_FIELD(sdl2, CheckKeyState),
        // Mouse
        REG_FIELD(sdl2, GetMousePosition),
        REG_FIELD(sdl2, IsMouseDown),
        // Joystick
        REG_FIELD(sdl2, JoystickOpen),
        REG_FIELD(sdl2, NumJoysticks),
        // Game Controller
        REG_FIELD(sdl2, GameControllerOpen),
        REG_FIELD(sdl2, IsGameController),
        REG_FIELD(sdl2, GameControllerGetAxisFromString),
        REG_FIELD(sdl2, GameControllerGetButtonFromString),
        // Timer
        REG_FIELD(sdl2, GetTicks),
        REG_FIELD(sdl2, Delay),
        REG_FIELD(sdl2, GetPerformanceCounter),
        REG_FIELD(sdl2, GetPerformanceFrequency),
    END_REG()
    NEW_MODULE(sdl2);
    LOAD_META(SDL_Window);
    LOAD_META(SDL_GLContext);
    LOAD_META(SDL_Event);
    LOAD_META(SDL_Joystick);
    LOAD_META(SDL_GameController);
    LOAD_ENUMS(sdl2);
END_MODULE(1)
