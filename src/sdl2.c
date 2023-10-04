#include "SDL_events.h"
#include "SDL_gamecontroller.h"
#include "SDL_keyboard.h"
#include "SDL_messagebox.h"
#include "SDL_mouse.h"
#include "SDL_video.h"
#include "lauxlib.h"
#include "lua.h"
#include "selene.h"

static const Uint8* keys;

static int l_sdl2_GLContext_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {NULL, NULL}
    };
    luaL_newmetatable(L, "SDL_GLContext");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static int l_window__GetSize(lua_State* L) {
    SDL_Window** win = luaL_checkudata(L, 1, "SDL_Window");
    int x, y;
    SDL_GetWindowSize(*win, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int l_sdl2_Window_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"GetSize", l_window__GetSize},
        {"SetSize", NULL},
        {"GetPosition", NULL},
        {"SetPosition", NULL},
        {"Swap", NULL},
        {"SetBordered", NULL},
        {"Maximize", NULL},
        {"Minimize", NULL},
        {"Restore", NULL},
        {"ShowSimpleMessageBox", NULL},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "SDL_Window");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static int l_sdl2_CreateWindow(lua_State* L) {
    const char* title = luaL_checkstring(L, 1);
    int width = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
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
    int attr = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);
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

static int l_Event__type(lua_State* L) {
    SDL_Event* ev = luaL_checkudata(L, 1, "SDL_Event");
    lua_pushinteger(L, ev->type);
    return 1;
}

static int l_Event__WindowEvent(lua_State* L) {
    SDL_Event* ev = luaL_checkudata(L, 1, "SDL_Event");
    lua_pushinteger(L, ev->window.event);
    lua_pushinteger(L, ev->window.windowID);
    lua_pushinteger(L, ev->window.data1);
    lua_pushinteger(L, ev->window.data2);
    return 4;
}

static int l_Event__MouseMotionEvent(lua_State* L) {
    SDL_Event* ev = luaL_checkudata(L, 1, "SDL_Event");
    lua_pushinteger(L, ev->motion.which);
    lua_pushinteger(L, ev->motion.state);
    lua_pushinteger(L, ev->motion.x);
    lua_pushinteger(L, ev->motion.y);
    lua_pushinteger(L, ev->motion.xrel);
    lua_pushinteger(L, ev->motion.yrel);
    return 6;
}

static int l_Event__MouseButtonEvent(lua_State* L) {
    SDL_Event* ev = luaL_checkudata(L, 1, "SDL_Event");
    lua_pushinteger(L, ev->button.which);
    lua_pushinteger(L, ev->button.button);
    lua_pushinteger(L, ev->button.x);
    lua_pushinteger(L, ev->button.y);
    lua_pushinteger(L, ev->button.clicks);
    return 5;
}

static int l_Event__KeyboardEvent(lua_State* L) {
    SDL_Event* ev = luaL_checkudata(L, 1, "SDL_Event");
    lua_pushstring(L, SDL_GetScancodeName(ev->key.keysym.scancode));
    lua_pushboolean(L, ev->key.repeat);
    return 2;
}

static int l_sdl2_Event_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"type", l_Event__type},
        {"WindowEvent", l_Event__WindowEvent},
        {"MouseMotionEvent", l_Event__MouseMotionEvent},
        {"MouseButtonEvent", l_Event__MouseButtonEvent},
        {"KeyboardEvent", l_Event__KeyboardEvent},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "SDL_Event");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

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
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, keys[key]);
    return 1;
}

static int l_sdl2_HasScreenKeyboardSupport(lua_State* L) {
    lua_pushboolean(L, SDL_HasScreenKeyboardSupport());
    return 1;
}

static int l_sdl2_IsScreenKeyboardShown(lua_State* L) {
    SDL_Window** window = luaL_checkudata(L, 1, "SDL_Window");
    lua_pushboolean(L, SDL_IsScreenKeyboardShown(*window));
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
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
    return 1;
}

// Joystick

static int l_sdl2_JoystickOpen(lua_State* L) {
    SDL_Joystick* joy = SDL_JoystickOpen(luaL_checknumber(L, 1));
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

static int l_sdl2_JoystickName(lua_State *L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushstring(L, SDL_JoystickName(*j));
    return 1;
}

static int l_sdl2_JoystickGetVendor(lua_State *L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushinteger(L, SDL_JoystickGetVendor(*j));
    return 1;
}

static int l_sdl2_JoystickGetProduct(lua_State *L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushinteger(L, SDL_JoystickGetProduct(*j));
    return 1;
}

static int l_sdl2_JoystickGetProductVersion(lua_State *L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushnumber(L, SDL_JoystickGetProductVersion(*j));
    return 1;
}

static int l_sdl2_JoystickGetType(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushnumber(L, SDL_JoystickGetType(*j));
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

static int l_sdl2_JoystickGetTypeString(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushstring(L, joy_types[SDL_JoystickGetType(*j)]);
    return 1;
}

static int l_sdl2_JoystickNumAxes(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushnumber(L, SDL_JoystickNumAxes(*j));
    return 1;
}

static int l_sdl2_JoystickNumBalls(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushnumber(L, SDL_JoystickNumBalls(*j));
    return 1;
}

static int l_sdl2_JoystickNumHats(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushnumber(L, SDL_JoystickNumHats(*j));
    return 1;
}

static int l_sdl2_JoystickNumButtons(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushnumber(L, SDL_JoystickNumButtons(*j));
    return 1;
}

static int l_sdl2_JoystickGetAxis(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    int axis = luaL_checkinteger(L, 2);
    lua_pushnumber(L, SDL_JoystickGetAxis(*j, axis));
    return 1;
}

static int l_sdl2_JoystickGetBall(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    int ball = luaL_checkinteger(L, 2);
    int dx, dy;
    SDL_JoystickGetBall(*j, ball, &dx, &dy);
    lua_pushnumber(L, dx);
    lua_pushnumber(L, dy);
    return 2;
}

static int l_sdl2_JoystickGetHat(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    int hat = luaL_checknumber(L, 2);
    lua_pushnumber(L, SDL_JoystickGetHat(*j, hat));
    return 1;
}

static int l_sdl2_JoystickGetButton(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    int button = luaL_checknumber(L, 2);
    lua_pushboolean(L, SDL_JoystickGetButton(*j, button));
    return 1;
}

static int l_sdl2_JoystickRumble(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    Uint16 low = luaL_checknumber(L, 2);
    Uint16 high = luaL_checknumber(L, 3);
    Uint32 freq = luaL_optnumber(L, 4, 100);
    lua_pushboolean(L, SDL_JoystickRumble(*j, low, high, freq) == 0);
    return 1;
}

static int l_sdl2_JoystickCurrentPowerLevel(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    lua_pushinteger(L, SDL_JoystickCurrentPowerLevel(*j));
    return 1;
}

static int l_sdl2_JoystickClose(lua_State* L) {
    SDL_Joystick** j = luaL_checkudata(L, 1, "SDL_Joystick");
    if (SDL_JoystickGetAttached(*j))
        SDL_JoystickClose(*j);
    return 0;
}

// Game Controller

static int l_sdl2_GameControllerOpen(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
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
    lua_pushboolean(L, SDL_IsGameController(luaL_checkinteger(L, 1)));
    return 1;
}

static int l_sdl2_GameControllerName(lua_State* L) {
    SDL_GameController** ctrl = luaL_checkudata(L, 1, "SDL_GameController");
    lua_pushstring(L, SDL_GameControllerName(*ctrl));
    return 1;
}

static int l_sdl2_GameControllerGetVendor(lua_State* L) {
    SDL_GameController** ctrl = luaL_checkudata(L, 1, "SDL_GameController");
    lua_pushinteger(L, SDL_GameControllerGetVendor(*ctrl));
    return 1;
}

static int l_sdl2_GameControllerGetProduct(lua_State *L) {
    SDL_GameController** gc = luaL_checkudata(L, 1, "SDL_GameControllerGetProduct");
    lua_pushnumber(L, SDL_GameControllerGetProduct(*gc));
    return 1;
}

static int l_sdl2_GameControllerGetProductVersion(lua_State *L) {
    SDL_GameController** gc = luaL_checkudata(L, 1, "SDL_GameController");
    lua_pushnumber(L, SDL_GameControllerGetProductVersion(*gc));
    return 1;
}

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static int l_sdl2_GameControllerGetAxisFromString(lua_State* L) {
    const char* str = luaL_checkstring(L, 1);
    luaL_checkinteger(L, SDL_GameControllerGetAxisFromString(str));
    return 1;
}

static int l_sdl2_GameControllerGetAxis(lua_State* L) {
    SDL_GameController** gc = luaL_checkudata(L, 1, "SDL_GameController");
    int axis = luaL_checkinteger(L, 2);
    float value = (float)SDL_GameControllerGetAxis(*gc, axis);
    lua_pushnumber(L, value);
    return 1;
}

static int l_sdl2_GameControllerGetButtonFromString(lua_State* L) {
    const char* str = luaL_checkstring(L, 1);
    lua_pushinteger(L, SDL_GameControllerGetButtonFromString(str));
    return 1;
}

static int l_sdl2_GameControllerGetButton(lua_State* L) {
    SDL_GameController** gc = luaL_checkudata(L, 1, "SDL_GameController");
    int button = luaL_checkinteger(L, 2);
    lua_pushboolean(L, SDL_GameControllerGetButton(*gc, button));
    return 1;
}

static int l_sdl2_GameControllerRumble(lua_State *L) {
    SDL_GameController** gc = luaL_checkudata(L, 1, "SDL_GameController");
    Uint16 low = (Uint16)luaL_checkinteger(L, 2);
    Uint16 high = (Uint16)luaL_checkinteger(L, 3);
    Uint32 freq = (Uint32)luaL_optinteger(L, 4, 100);
    lua_pushboolean(L, SDL_GameControllerRumble(*gc, low, high, freq) == 0);
    return 1;
}

const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};

static int l_sdl2_GameControllerCurrentPowerLevel(lua_State* L) {
	SDL_GameController** gc = luaL_checkudata(L, 1, "SDL_GameController");
	if (!(*gc))
		return luaL_error(L, "Closed joystick");
	SDL_Joystick *j = SDL_GameControllerGetJoystick(*gc);
	lua_pushstring(L, gpad_powerlevels[SDL_JoystickCurrentPowerLevel(j)+1]);
	return 1;
}

static int l_sdl2_GameControllerClose(lua_State* L) {
    SDL_GameController** gc = luaL_checkudata(L, 1, "SDL_GameController");
    if (SDL_GameControllerGetAttached(*gc))
        SDL_GameControllerClose(*gc);
    return 0;
}

// timer

static int l_sdl2_GetTicks(lua_State* L) {
    lua_pushinteger(L, SDL_GetTicks());
    return 1;
}

static int l_sdl2_Delay(lua_State* L) {
    Uint32 ms = luaL_checkinteger(L, 1);
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
        {"JoystickName", l_sdl2_JoystickName},
        {"JoystickGetVendor", l_sdl2_JoystickGetVendor},
        {"JoystickGetProductVersion", l_sdl2_JoystickGetProductVersion},
        {"JoystickGetType", l_sdl2_JoystickGetType},
        {"JoystickGetTypeString", l_sdl2_JoystickGetTypeString},
        {"JoystickNumAxes", l_sdl2_JoystickNumAxes},
        {"JoystickNumBalls", l_sdl2_JoystickNumBalls},
        {"JoystickNumHats", l_sdl2_JoystickNumHats},
        {"JoystickNumButtons", l_sdl2_JoystickNumButtons},
        {"JoystickGetAxis", l_sdl2_JoystickGetAxis},
        {"JoystickGetBall", l_sdl2_JoystickGetBall},
        {"JoystickGetHat", l_sdl2_JoystickGetHat},
        {"JoystickGetButton", l_sdl2_JoystickGetButton},
        {"JoystickRumble", l_sdl2_JoystickRumble},
        {"JoystickCurrentPowerLevel", l_sdl2_JoystickCurrentPowerLevel},
        {"JoystickClose", l_sdl2_JoystickClose},
        // game controller
        {"GameControllerOpen", l_sdl2_GameControllerOpen},
        {"IsGameController", l_sdl2_IsGameController},
        {"GameControllerName", l_sdl2_GameControllerName},
        {"GameControllerGetVendor", l_sdl2_GameControllerGetVendor},
        {"GameControllerGetProduct", l_sdl2_GameControllerGetProduct},
        {"GameControllerGetProductVersion", l_sdl2_GameControllerGetProductVersion},
        {"GameControllerAxisFromString", l_sdl2_GameControllerGetAxisFromString},
        {"GameControllerGetAxis", l_sdl2_GameControllerGetAxis},
        {"GameControllerGetButtonFromString", l_sdl2_GameControllerGetButtonFromString},
        {"GameControllerGetButton", l_sdl2_GameControllerGetButton},
        {"GameControllerRumble", l_sdl2_GameControllerRumble},
        {"GameControllerCurrentPowerLevel", l_sdl2_GameControllerCurrentPowerLevel},
        {"GameControllerClose", l_sdl2_GameControllerClose},
        // timer
        {"GetTicks", l_sdl2_GetTicks},
        {"Delay", l_sdl2_Delay},
        {"GetPerformanceCounter", l_sdl2_GetPerformanceCounter},
        {"GetPerformanceFrequency", l_sdl2_GetPerformanceFrequency},
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    l_sdl2_Window_meta(L);
    lua_setfield(L, -2, "SDL_Window");
    l_sdl2_GLContext_meta(L);
    lua_setfield(L, -2, "SDL_GLContext");
    l_sdl2_Event_meta(L);
    lua_setfield(L, -2, "SDL_Event");
    for (int i = 0; l_sdl2_Enums[i].name != NULL; i++) {
        lua_pushinteger(L, l_sdl2_Enums[i].value);
        lua_setfield(L, -2, l_sdl2_Enums[i].name);
    }
   return 0;
}
