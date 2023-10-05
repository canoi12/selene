#include "selene.h"

static const Uint8* keys;

NEW_REG(SDL_GLContext) {
    {NULL, NULL}
};
NEW_META(SDL_GLContext)

static META_FUNCTION(SDL_Window, GetSize) {
    INIT_GET_UDATA(SDL_Window, *win);
    int width, height;
    SDL_GetWindowSize(*win, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
    return 2;
}

static META_FUNCTION(SDL_Window, SetSize) {
    INIT_GET_UDATA(SDL_Window, *win);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    SDL_SetWindowSize(*win, width, height);
    return 0;
}

static META_FUNCTION(SDL_Window, GetPosition) {
    INIT_GET_UDATA(SDL_Window, *win);
    int x, y;
    SDL_GetWindowPosition(*win, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static META_FUNCTION(SDL_Window, SetPosition) {
    INIT_GET_UDATA(SDL_Window, *win);
    CHECK_INTEGER(x);
    CHECK_INTEGER(y);
    SDL_SetWindowPosition(*win, x, y);
    return 0;
}

static META_FUNCTION(SDL_Window, Swap) {
    INIT_GET_UDATA(SDL_Window, *win);
    SDL_GL_SwapWindow(*win);
    return 0;
}

static META_FUNCTION(SDL_Window, SetBordered) {
    INIT_GET_UDATA(SDL_Window, *win);
    GET_BOOLEAN(value);
    SDL_SetWindowBordered(*win, value);
    return 0;
}

static META_FUNCTION(SDL_Window, Maximize) {
    INIT_GET_UDATA(SDL_Window, *win);
    SDL_MaximizeWindow(*win);
    return 0;
}

static META_FUNCTION(SDL_Window, Minimize) {
    INIT_GET_UDATA(SDL_Window, *win);
    SDL_MinimizeWindow(*win);
    return 0;
}

static META_FUNCTION(SDL_Window, Restore) {
    INIT_GET_UDATA(SDL_Window, *win);
    SDL_RestoreWindow(*win);
    return 0;
}

static META_FUNCTION(SDL_Window, ShowSimpleMessageBox) {
    INIT_GET_UDATA(SDL_Window, *win);
    return 0;
}

static const NEW_REG(SDL_Window) {
    META_FIELD(SDL_Window, GetSize),
    META_FIELD(SDL_Window, SetSize),
    META_FIELD(SDL_Window, GetPosition),
    META_FIELD(SDL_Window, SetPosition),
    META_FIELD(SDL_Window, Swap),
    META_FIELD(SDL_Window, SetBordered),
    META_FIELD(SDL_Window, Maximize),
    META_FIELD(SDL_Window, Minimize),
    META_FIELD(SDL_Window, Restore),
    META_FIELD(SDL_Window, ShowSimpleMessageBox),
    {NULL, NULL}
};

NEW_META(SDL_Window);

static int l_sdl2_CreateWindow(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(title);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    SDL_Window** window = lua_newuserdata(L, sizeof(void*));
    luaL_setmetatable(L, "SDL_Window");
    *window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        flags
    );
    return 1;
}

static int l_sdl2_DestroyWindow(lua_State* L) {
    SDL_Window** window = luaL_checkudata(L, 1, "SDL_Window");
    SDL_DestroyWindow(*window);
    return 0;
}

static int l_sdl2_GL_SetAttribute(lua_State* L) {
    INIT_ARG();
    CHECK_INTEGER(attr);
    CHECK_INTEGER(value);
    SDL_GL_SetAttribute(attr, value);
    return 0;
}

static int l_sdl2_GL_CreateContext(lua_State* L) {
    SDL_Window** window = luaL_checkudata(L, 1, "SDL_Window");
    SDL_GLContext* ctx = lua_newuserdata(L, sizeof(void*));
    luaL_setmetatable(L, "SDL_GLContext");
    *ctx = SDL_GL_CreateContext(*window);
    return 1;
}

static int l_sdl2_GL_DeleteContext(lua_State* L) {
    SDL_GLContext* ctx = luaL_checkudata(L, 1, "SDL_GLContext");
    SDL_GL_DeleteContext(*ctx);
    return 0;
}

static int l_sdl2_GL_MakeCurrent(lua_State* L) {
    SDL_GLContext* ctx = luaL_checkudata(L, 1, "SDL_GLContext");
    SDL_Window** window = luaL_checkudata(L, 2, "SDL_Window");
    SDL_GL_MakeCurrent(*window, *ctx);
    return 0;
}

static int l_sdl2_GL_SwapWindow(lua_State* L) {
    SDL_Window** window = luaL_checkudata(L, 1, "SDL_Window");
    SDL_GL_SwapWindow(*window);
    return 0;
}

static int l_sdl2_GetScancodeFromName(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    lua_pushinteger(L, SDL_GetScancodeFromName(name));
    return 1;
}

// Event

static META_FUNCTION(SDL_Event, Poll) {
    INIT_GET_UDATA(SDL_Event, ev);
    lua_pushboolean(L, SDL_PollEvent(ev));
    return 1;
}

static META_FUNCTION(SDL_Event, GetType) {
    INIT_GET_UDATA(SDL_Event, ev);
    PUSH_INTEGER(ev->type);
    return 1;
}

static META_FUNCTION(SDL_Event, WindowEvent) {
    INIT_GET_UDATA(SDL_Event, ev);
    PUSH_INTEGER(ev->window.event);
    PUSH_INTEGER(ev->window.windowID);
    PUSH_INTEGER(ev->window.data1);
    PUSH_INTEGER(ev->window.data2);
    return 4;
}

static META_FUNCTION(SDL_Event, MouseMotionEvent) {
    INIT_GET_UDATA(SDL_Event, ev);
    PUSH_INTEGER(ev->motion.which);
    PUSH_INTEGER(ev->motion.state);
    PUSH_INTEGER(ev->motion.x);
    PUSH_INTEGER(ev->motion.y);
    PUSH_INTEGER(ev->motion.xrel);
    PUSH_INTEGER(ev->motion.yrel);
    return 6;
}

static META_FUNCTION(SDL_Event, MouseButtonEvent) {
    INIT_GET_UDATA(SDL_Event, ev);
    PUSH_INTEGER(ev->button.which);
    PUSH_INTEGER(ev->button.button);
    PUSH_INTEGER(ev->button.x);
    PUSH_INTEGER(ev->button.y);
    PUSH_INTEGER(ev->button.clicks);
    return 5;
}

static META_FUNCTION(SDL_Event, KeyboardEvent) {
    INIT_GET_UDATA(SDL_Event, ev);
    PUSH_STRING(SDL_GetScancodeName(ev->key.keysym.scancode));
    PUSH_BOOLEAN(ev->key.repeat);
    return 2;
}

const NEW_REG(SDL_Event) {
    META_FIELD(SDL_Event, Poll),
    META_FIELD(SDL_Event, GetType),
    META_FIELD(SDL_Event, WindowEvent),
    META_FIELD(SDL_Event, MouseMotionEvent),
    META_FIELD(SDL_Event, MouseButtonEvent),
    META_FIELD(SDL_Event, KeyboardEvent),
    {NULL, NULL}
};
NEW_META(SDL_Event);

static int l_sdl2_NewEvent(lua_State* L) {
    SDL_Event* ev = lua_newuserdata(L, sizeof(*ev));
    luaL_setmetatable(L, "SDL_Event");
    return 1;
}

static int l_sdl2_PollEvent(lua_State* L) {
    SDL_Event* ev = luaL_checkudata(L, 1, "SDL_Event");
    lua_pushboolean(L, SDL_PollEvent(ev));
    return 1;
}

// Keyboard
static int l_sdl2_CheckKeyState(lua_State* L) {
    INIT_ARG();
    CHECK_INTEGER(key);
    PUSH_BOOLEAN(keys[key]);
    return 1;
}

static int l_sdl2_HasScreenKeyboardSupport(lua_State* L) {
    PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());
    return 1;
}

static int l_sdl2_IsScreenKeyboardShown(lua_State* L) {
    INIT_GET_UDATA(SDL_Window, *win);
    PUSH_BOOLEAN(SDL_IsScreenKeyboardShown(*win));
    return 1;
}

static int l_sdl2_GetMousePosition(lua_State* L) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

static int l_sdl2_IsMouseDown(lua_State* L) {
    INIT_ARG();
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
    return 1;
}

// Joystick

static int l_sdl2_JoystickOpen(lua_State* L) {
    SDL_Joystick* joy = SDL_JoystickOpen((int)luaL_checknumber(L, 1));
    if (joy) {
        SDL_Joystick** j = lua_newuserdata(L, sizeof(void*));
        luaL_setmetatable(L, "SDL_Joystick");
        *j = joy;
    } else lua_pushnil(L);
    return 1;
}

static int l_sdl2_NumJoysticks(lua_State *L) {
    lua_pushinteger(L, SDL_NumJoysticks());
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetName) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_STRING(SDL_JoystickName(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetVendor) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickGetVendor(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetProduct) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickGetProduct(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetProductVersion) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetType) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickGetType(*j));
    return 1;
}

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

static META_FUNCTION(SDL_Joystick, GetTypeString) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_STRING(joy_types[SDL_JoystickGetType(*j)]);
    return 1;
}

static META_FUNCTION(SDL_Joystick, NumAxes) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickNumAxes(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, NumBalls) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickNumBalls(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, NumHats) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickNumHats(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, NumButtons) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickNumButtons(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetAxis) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*j, axis));
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetBall) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*j, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
    return 2;
}

static META_FUNCTION(SDL_Joystick, GetHat) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*j, hat));
    return 1;
}

static META_FUNCTION(SDL_Joystick, GetButton) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    CHECK_INTEGER(button);
    PUSH_INTEGER(SDL_JoystickGetButton(*j, button));
    return 1;
}

static META_FUNCTION(SDL_Joystick, Rumble) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*j, low, high, freq) == 0);
    return 1;
}

static META_FUNCTION(SDL_Joystick, CurrentPowerLevel) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*j));
    return 1;
}

static META_FUNCTION(SDL_Joystick, Close) {
    INIT_GET_UDATA(SDL_Joystick, *j);
    if (SDL_JoystickGetAttached(*j))
        SDL_JoystickClose(*j);
    return 0;
}

const NEW_REG(SDL_Joystick) {
    META_FIELD(SDL_Joystick, GetName),
    META_FIELD(SDL_Joystick, GetVendor),
    META_FIELD(SDL_Joystick, GetProduct),
    META_FIELD(SDL_Joystick, GetProductVersion),
    META_FIELD(SDL_Joystick, GetType),
    META_FIELD(SDL_Joystick, GetTypeString),
    META_FIELD(SDL_Joystick, NumAxes),
    META_FIELD(SDL_Joystick, NumBalls),
    META_FIELD(SDL_Joystick, NumHats),
    META_FIELD(SDL_Joystick, NumButtons),
    META_FIELD(SDL_Joystick, GetAxis),
    META_FIELD(SDL_Joystick, GetBall),
    META_FIELD(SDL_Joystick, GetHat),
    META_FIELD(SDL_Joystick, GetButton),
    META_FIELD(SDL_Joystick, Rumble),
    META_FIELD(SDL_Joystick, CurrentPowerLevel),
    META_FIELD(SDL_Joystick, Close),
    {NULL, NULL}
};
NEW_META(SDL_Joystick);

static int l_sdl2_JoystickClose(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    if (SDL_JoystickGetAttached(*j))
        SDL_JoystickClose(*j);
    return 0;
}

// Game Controller
static int l_sdl2_GameControllerOpen(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (!ctrl) lua_pushnil(L);
    else {
        SDL_GameController** controller = lua_newuserdata(L, sizeof(void*));
        *controller = ctrl;
        luaL_setmetatable(L, "SDL_GameController");
    }
    return 1;
}

static int l_sdl2_IsGameController(lua_State* L) {
    lua_pushboolean(L, SDL_IsGameController((int)luaL_checkinteger(L, 1)));
    return 1;
}

static META_FUNCTION(SDL_GameController, GetName) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    PUSH_STRING(SDL_GameControllerName(*gc));
    return 1;
}

static META_FUNCTION(SDL_GameController, GetVendor) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    PUSH_INTEGER(SDL_GameControllerGetVendor(*gc));
    return 1;
}

static META_FUNCTION(SDL_GameController, GetProduct) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    PUSH_INTEGER(SDL_GameControllerGetProduct(*gc));
    return 1;
}

static META_FUNCTION(SDL_GameController, GetProductVersion) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*gc));
    return 1;
}

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static META_FUNCTION(SDL_GameController, GetAxisFromString) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
    return 1;
}

static int l_sdl2_GameControllerGetAxisFromString(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
    return 1;
}

static META_FUNCTION(SDL_GameController, GetAxis) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*gc, axis));
    return 1;
}

static META_FUNCTION(SDL_GameController, GetButtonFromString) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
    return 1;
}

static int l_sdl2_GameControllerGetButtonFromString(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
    return 1;
}

static META_FUNCTION(SDL_GameController, GetButton) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetButton(*gc, axis));
    return 1;
}

static META_FUNCTION(SDL_GameController, Rumble) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_GameControllerRumble(*gc, low, high, freq) == 0);
    return 1;
}

const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};

static META_FUNCTION(SDL_GameController, CurrentPowerLevel) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    SDL_Joystick *j = SDL_GameControllerGetJoystick(*gc);
    PUSH_STRING(gpad_powerlevels[SDL_JoystickCurrentPowerLevel(j)+1]);
    return 1;
}

static META_FUNCTION(SDL_GameController, Close) {
    INIT_GET_UDATA(SDL_GameController, *gc);
    if (SDL_GameControllerGetAttached(*gc))
        SDL_GameControllerClose(*gc);
    return 0;
}

const NEW_REG(SDL_GameController) {
    META_FIELD(SDL_GameController, GetName),
    META_FIELD(SDL_GameController, GetVendor),
    META_FIELD(SDL_GameController, GetProduct),
    META_FIELD(SDL_GameController, GetProductVersion),
    META_FIELD(SDL_GameController, GetAxisFromString),
    META_FIELD(SDL_GameController, GetAxis),
    META_FIELD(SDL_GameController, GetButtonFromString),
    META_FIELD(SDL_GameController, GetButton),
    META_FIELD(SDL_GameController, Rumble),
    META_FIELD(SDL_GameController, CurrentPowerLevel),
    META_FIELD(SDL_GameController, Close),
    {NULL, NULL}
};
NEW_META(SDL_GameController);

// timer

static int l_sdl2_GetTicks(lua_State* L) {
    lua_pushinteger(L, SDL_GetTicks());
    return 1;
}

static int l_sdl2_Delay(lua_State* L) {
    Uint32 ms = (Uint32)luaL_checkinteger(L, 1);
    SDL_Delay(ms);
    return 0;
}

static int l_sdl2_GetPerformanceCounter(lua_State* L) {
    lua_pushinteger(L, SDL_GetPerformanceCounter());
    return 1;
}

static int l_sdl2_GetPerformanceFrequency(lua_State* L) {
    lua_pushinteger(L, SDL_GetPerformanceFrequency());
    return 1;
}

static struct {
    const char* name;
    int value;
} l_sdl2_Enums[] = {
    // Events
    {"QUIT", SDL_QUIT},
    {"WINDOWEVENT", SDL_WINDOWEVENT},
    {"KEYDOWN", SDL_KEYDOWN},
    {"KEYUP", SDL_KEYUP},
    {"MOUSEMOTION", SDL_MOUSEMOTION},
    {"MOUSEBUTTONDOWN", SDL_MOUSEBUTTONDOWN},
    {"MOUSEBUTTONUP", SDL_MOUSEBUTTONUP},

    // Window Events
    {"WINDOWEVENT_CLOSE", SDL_WINDOWEVENT_CLOSE},
    {"WINDOWEVENT_SIZE_CHANGED", SDL_WINDOWEVENT_SIZE_CHANGED},
    {"WINDOWEVENT_RESIZED", SDL_WINDOWEVENT_RESIZED},
    {"WINDOWEVENT_MOVED", SDL_WINDOWEVENT_MOVED},
    {"WINDOWEVENT_MINIMIZED", SDL_WINDOWEVENT_MINIMIZED},
    {"WINDOWEVENT_MAXIMIZED", SDL_WINDOWEVENT_MAXIMIZED},
    {"WINDOWEVENT_RESTORED", SDL_WINDOWEVENT_RESTORED},
    {"WINDOWEVENT_SHOWN", SDL_WINDOWEVENT_SHOWN},
    {"WINDOWEVENT_HIDDEN", SDL_WINDOWEVENT_HIDDEN},
    {"WINDOWEVENT_ENTER", SDL_WINDOWEVENT_ENTER},
    {"WINDOWEVENT_LEAVE", SDL_WINDOWEVENT_LEAVE},
    {"WINDOWEVENT_FOCUS_GAINED", SDL_WINDOWEVENT_FOCUS_GAINED},
    {"WINDOWEVENT_FOCUS_LOST", SDL_WINDOWEVENT_FOCUS_LOST},
    {"WINDOWEVENT_TAKE_FOCUS", SDL_WINDOWEVENT_TAKE_FOCUS},
    {"WINDOWEVENT_HIT_TEST", SDL_WINDOWEVENT_HIT_TEST},

    // window flags
    {"WINDOW_RESIZABLE", SDL_WINDOW_RESIZABLE},
    {"WINDOW_FULLSCREEN", SDL_WINDOW_FULLSCREEN},
    {"WINDOW_FULLSCREEN_DESKTOP", SDL_WINDOW_FULLSCREEN_DESKTOP},
    {"WINDOW_BORDERLESS", SDL_WINDOW_BORDERLESS},
    {"WINDOW_ALWAYS_ON_TOP", SDL_WINDOW_ALWAYS_ON_TOP},
    // message box
    {"MESSAGEBOX_ERROR", SDL_MESSAGEBOX_ERROR},
    {"MESSAGEBOX_WARNING", SDL_MESSAGEBOX_WARNING},
    {"MESSAGEBOX_INFORMATION", SDL_MESSAGEBOX_INFORMATION},
    // SDL GL
    {"GL_DOUBLEBUFFER", SDL_GL_DOUBLEBUFFER},
    {"GL_DEPTH_SIZE", SDL_GL_DEPTH_SIZE},
    {"GL_STENCIL_SIZE", SDL_GL_STENCIL_SIZE},
    {"GL_CONTEXT_PROFILE_MASK", SDL_GL_CONTEXT_PROFILE_MASK},
    {"GL_CONTEXT_MAJOR_VERSION", SDL_GL_CONTEXT_MAJOR_VERSION},
    {"GL_CONTEXT_MINOR_VERSION", SDL_GL_CONTEXT_MINOR_VERSION},
    {"GL_CONTEXT_PROFILE_CORE", SDL_GL_CONTEXT_PROFILE_CORE},
    {"GL_CONTEXT_PROFILE_COMPATIBILITY", SDL_GL_CONTEXT_PROFILE_COMPATIBILITY},
    {"GL_CONTEXT_PROFILE_ES", SDL_GL_CONTEXT_PROFILE_ES},
    {NULL, 0}
};

int seleneopen_sdl2(lua_State* L) {
    keys = SDL_GetKeyboardState(NULL);
    luaL_Reg reg[] = {
        {"CreateWindow", l_sdl2_CreateWindow},
        {"DestroyWindow", l_sdl2_DestroyWindow},
        {"GL_SwapWindow", l_sdl2_GL_SwapWindow},
        {"GL_SetAttribute", l_sdl2_GL_SetAttribute},
        {"GL_CreateContext", l_sdl2_GL_CreateContext},
        {"GL_MakeCurrent", l_sdl2_GL_MakeCurrent},
        {"GL_DeleteContext", l_sdl2_GL_DeleteContext},
        //Event
        {"NewEvent", l_sdl2_NewEvent},
        {"PollEvent", l_sdl2_PollEvent},
        // keyboard
        {"GetScancodeFromName", l_sdl2_GetScancodeFromName},
        {"HasScreenKeyboardSupport", l_sdl2_HasScreenKeyboardSupport},
        {"IsScreenKeyboardShown", l_sdl2_IsScreenKeyboardShown},
        {"CheckKeyState", l_sdl2_CheckKeyState},
        // mouse
        {"GetMousePosition", l_sdl2_GetMousePosition},
        {"IsMouseDown", l_sdl2_IsMouseDown},
        // joystick
        {"JoystickOpen", l_sdl2_JoystickOpen},
        {"NumJoysticks", l_sdl2_NumJoysticks},
        {"JoystickClose", l_sdl2_JoystickClose},
        // game controller
        {"GameControllerOpen", l_sdl2_GameControllerOpen},
        {"IsGameController", l_sdl2_IsGameController},
        {"GameControllerAxisFromString", l_sdl2_GameControllerGetAxisFromString},
        {"GameControllerGetButtonFromString", l_sdl2_GameControllerGetButtonFromString},
        // timer
        {"GetTicks", l_sdl2_GetTicks},
        {"Delay", l_sdl2_Delay},
        {"GetPerformanceCounter", l_sdl2_GetPerformanceCounter},
        {"GetPerformanceFrequency", l_sdl2_GetPerformanceFrequency},
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    LOAD_META(SDL_Window);
    LOAD_META(SDL_GLContext);
    LOAD_META(SDL_Event);
    LOAD_META(SDL_Joystick);
    LOAD_META(SDL_GameController);
    for (int i = 0; l_sdl2_Enums[i].name != NULL; i++) {
        lua_pushinteger(L, l_sdl2_Enums[i].value);
        lua_setfield(L, -2, l_sdl2_Enums[i].name);
    }
   return 0;
}
