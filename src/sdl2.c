#include "SDL_audio.h"
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
    lua_getfield(L, 1, "callback");
    void* callback = NULL;
    if (lua_type(L, -1) == LUA_TLIGHTUSERDATA)
        callback = lua_touserdata(L, -1);
    lua_pop(L, 1);

    SDL_AudioSpec des;
    des.freq = freq;
    des.channels = channels;
    des.samples = samples;
    des.format = AUDIO_S16;
    des.callback = callback;

    lua_pushboolean(L, SDL_OpenAudio(&des, NULL) == 0);
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, PauseAudio)
    GET_BOOLEAN(pause);
    SDL_PauseAudio(pause);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, CloseAudio)
    SDL_CloseAudio();
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, OpenAudioDevice)
    const char* device = NULL;
    if (lua_type(L, arg) == LUA_TSTRING)
        device = lua_tostring(L, arg);
    arg++;
    GET_BOOLEAN(is_capture);
    if (lua_type(L, arg) != LUA_TTABLE)
        return luaL_argerror(L, arg, "Must be a table");
    lua_getfield(L, arg, "sample_rate");
    int freq = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "channels");
    int channels = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "samples");
    int samples = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "callback");
    void* callback = NULL;
    if (lua_type(L, -1) == LUA_TLIGHTUSERDATA)
        callback = lua_touserdata(L, -1);
    lua_pop(L, 1);
    arg++;

    SDL_AudioSpec desired, obtained;
    desired.freq = freq;
    desired.channels = channels;
    desired.samples = samples;
    desired.format = AUDIO_S16;
    desired.callback = callback;

    GET_BOOLEAN(allowed_changes);
    AudioDeviceID d = SDL_OpenAudioDevice(device, is_capture, &desired, &obtained, allowed_changes);
    if (d == 0) {
        lua_pushnil(L);
        return 1;
    }
    else {
        NEW_UDATA(AudioDeviceID, dev);
        *dev = d;
    }
    lua_newtable(L);
    PUSH_INTEGER(obtained.channels);
    lua_setfield(L, -2, "channels");
    PUSH_INTEGER(obtained.freq);
    lua_setfield(L, -2, "sample_rate");
    PUSH_INTEGER(obtained.samples);
    lua_setfield(L, -2, "samples");
END_FUNCTION(2)

static BEGIN_META_FUNCTION(AudioDeviceID, Pause)
    GET_BOOLEAN(pause);
    SDL_PauseAudioDevice(*self, pause);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(AudioDeviceID, Close)
    SDL_CloseAudioDevice(*self);
END_FUNCTION(0)

static BEGIN_META(AudioDeviceID)
    BEGIN_REG(AudioDeviceID)
        REG_META_FIELD(AudioDeviceID, Pause),
        REG_META_FIELD(AudioDeviceID, Close),
    END_REG()
    NEW_META(AudioDeviceID);
END_META(1)

/************************
 #                      #
 #      GLContext       #
 #                      #
 ************************/

static BEGIN_META(GLContext)
    BEGIN_REG(GLContext)
    END_REG()
    NEW_META(GLContext);
END_META(1)

/************************
 #                      #
 #        Window        #
 #                      #
 ************************/
static BEGIN_META_FUNCTION(Window, GetSize)
    int width, height;
    SDL_GetWindowSize(*self, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
END_FUNCTION(2)

static BEGIN_META_FUNCTION(Window, SetSize)
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    SDL_SetWindowSize(*self, width, height);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Window, GetPosition)
    int x, y;
    SDL_GetWindowPosition(*self, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
END_FUNCTION(2)

static BEGIN_META_FUNCTION(Window, SetPosition)
    CHECK_INTEGER(x);
    CHECK_INTEGER(y);
    SDL_SetWindowPosition(*self, x, y);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Window, Swap)
    SDL_GL_SwapWindow(*self);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Window, SetBordered)
    GET_BOOLEAN(value);
    SDL_SetWindowBordered(*self, value);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Window, Maximize)
    SDL_MaximizeWindow(*self);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Window, Minimize)
    SDL_MinimizeWindow(*self);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Window, Restore)
    SDL_RestoreWindow(*self);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Window, ShowSimpleMessageBox)
END_FUNCTION(0)

static BEGIN_META(Window)
    BEGIN_REG(Window)
        REG_META_FIELD(Window, GetSize),
        REG_META_FIELD(Window, SetSize),
        REG_META_FIELD(Window, GetPosition),
        REG_META_FIELD(Window, SetPosition),
        REG_META_FIELD(Window, Swap),
        REG_META_FIELD(Window, SetBordered),
        REG_META_FIELD(Window, Maximize),
        REG_META_FIELD(Window, Minimize),
        REG_META_FIELD(Window, Restore),
        REG_META_FIELD(Window, ShowSimpleMessageBox),
    END_REG()
    NEW_META(Window);
END_META(1)

static BEGIN_FUNCTION(sdl2, CreateWindow)
    CHECK_STRING(title);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    NEW_UDATA(Window, window);
    *window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        flags
    );
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, DestroyWindow)
    CHECK_UDATA(Window, window);
    SDL_DestroyWindow(*window);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GL_SetAttribute)
    CHECK_INTEGER(attr);
    CHECK_INTEGER(value);
    SDL_GL_SetAttribute(attr, value);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GL_CreateContext)
    CHECK_UDATA(Window, win);
    NEW_UDATA(GLContext, ctx);
    *ctx = SDL_GL_CreateContext(*win);
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, GL_DeleteContext)
    CHECK_UDATA(GLContext, ctx);
    SDL_GL_DeleteContext(*ctx);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GL_MakeCurrent)
    GET_UDATA(GLContext, ctx);
    GET_UDATA(Window, window);
    SDL_GL_MakeCurrent(*window, *ctx);
END_FUNCTION(0)

static BEGIN_FUNCTION(sdl2, GetScancodeFromName)
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GetScancodeFromName(name));
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, GetScancodeName)
    CHECK_INTEGER(scancode);
    PUSH_STRING(SDL_GetScancodeName(scancode));
END_FUNCTION(1)

/************************
 #                      #
 #        Event         #
 #                      #
 ************************/

static BEGIN_META_FUNCTION(Event, Poll)
    PUSH_BOOLEAN(SDL_PollEvent(self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Event, GetType)
    PUSH_INTEGER(self->type);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Event, WindowEvent)
    PUSH_INTEGER(self->window.event);
    PUSH_INTEGER(self->window.windowID);
    PUSH_INTEGER(self->window.data1);
    PUSH_INTEGER(self->window.data2);
END_FUNCTION(4)

static BEGIN_META_FUNCTION(Event, MouseMotionEvent)
    PUSH_INTEGER(self->motion.which);
    PUSH_INTEGER(self->motion.state);
    PUSH_INTEGER(self->motion.x);
    PUSH_INTEGER(self->motion.y);
    PUSH_INTEGER(self->motion.xrel);
    PUSH_INTEGER(self->motion.yrel);
END_FUNCTION(6)

static BEGIN_META_FUNCTION(Event, MouseButtonEvent)
    PUSH_INTEGER(self->button.which);
    PUSH_INTEGER(self->button.button);
    PUSH_INTEGER(self->button.x);
    PUSH_INTEGER(self->button.y);
    PUSH_INTEGER(self->button.clicks);
END_FUNCTION(5)

static BEGIN_META_FUNCTION(Event, MouseWheelEvent)
PUSH_INTEGER(self->wheel.which);
PUSH_INTEGER(self->wheel.x);
PUSH_INTEGER(self->wheel.y);
END_FUNCTION(4)

static BEGIN_META_FUNCTION(Event, KeyboardEvent)
    PUSH_STRING(SDL_GetScancodeName(self->key.keysym.scancode));
    PUSH_BOOLEAN(self->key.repeat);
END_FUNCTION(2)

static BEGIN_META(Event)
    BEGIN_REG(Event)
        REG_META_FIELD(Event, Poll),
        REG_META_FIELD(Event, GetType),
        REG_META_FIELD(Event, WindowEvent),
        REG_META_FIELD(Event, MouseMotionEvent),
        REG_META_FIELD(Event, MouseButtonEvent),
        REG_META_FIELD(Event, KeyboardEvent),
    END_REG()
    NEW_META(Event);
END_META(1)

static BEGIN_FUNCTION(sdl2, NewEvent)
    NEW_UDATA(Event, ev);
END_FUNCTION(1)

/************************
 #                      #
 #       Keyboard       #
 #                      #
 ************************/
static BEGIN_FUNCTION(sdl2, CheckKeyState)
    CHECK_INTEGER(key);
    PUSH_BOOLEAN(keys[key]);
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, HasScreenKeyboardSupport)
    PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, IsScreenKeyboardShown)
    GET_UDATA(Window, win);
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
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
END_FUNCTION(1)

/************************
 #                      #
 #       Joystick       #
 #                      #
 ************************/

static BEGIN_FUNCTION(sdl2, JoystickOpen)
    CHECK_INTEGER(id);
    SDL_Joystick* joy = SDL_JoystickOpen(id);
    if (joy) {
        NEW_UDATA(Joystick, j);
        *j = joy;
    } else PUSH_NIL();
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, NumJoysticks)
    PUSH_INTEGER(SDL_NumJoysticks());
END_FUNCTION(1)

// Meta functions

static BEGIN_META_FUNCTION(Joystick, GetName)
    PUSH_STRING(SDL_JoystickName(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, GetVendor)
    PUSH_INTEGER(SDL_JoystickGetVendor(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, GetProduct)
    PUSH_INTEGER(SDL_JoystickGetProduct(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, GetProductVersion)
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, GetType)
    PUSH_INTEGER(SDL_JoystickGetType(*self));
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

static BEGIN_META_FUNCTION(Joystick, GetTypeString)
    PUSH_STRING(joy_types[SDL_JoystickGetType(*self)]);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, NumAxes)
    PUSH_INTEGER(SDL_JoystickNumAxes(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, NumBalls)
    PUSH_INTEGER(SDL_JoystickNumBalls(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, NumHats)
    PUSH_INTEGER(SDL_JoystickNumHats(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, NumButtons)
    PUSH_INTEGER(SDL_JoystickNumButtons(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, GetAxis)
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*self, axis));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, GetBall)
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*self, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
END_FUNCTION(2)

static BEGIN_META_FUNCTION(Joystick, GetHat)
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*self, hat));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, GetButton)
    CHECK_INTEGER(button);
    PUSH_INTEGER(SDL_JoystickGetButton(*self, button));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, Rumble)
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*self, low, high, freq) == 0);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, CurrentPowerLevel)
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Joystick, Close)
    if (SDL_JoystickGetAttached(*self))
        SDL_JoystickClose(*self);
END_FUNCTION(0)

static BEGIN_META(Joystick)
    BEGIN_REG(Joystick)
        REG_META_FIELD(Joystick, GetName),
        REG_META_FIELD(Joystick, GetVendor),
        REG_META_FIELD(Joystick, GetProduct),
        REG_META_FIELD(Joystick, GetProductVersion),
        REG_META_FIELD(Joystick, GetType),
        REG_META_FIELD(Joystick, GetTypeString),
        REG_META_FIELD(Joystick, NumAxes),
        REG_META_FIELD(Joystick, NumBalls),
        REG_META_FIELD(Joystick, NumHats),
        REG_META_FIELD(Joystick, NumButtons),
        REG_META_FIELD(Joystick, GetAxis),
        REG_META_FIELD(Joystick, GetBall),
        REG_META_FIELD(Joystick, GetHat),
        REG_META_FIELD(Joystick, GetButton),
        REG_META_FIELD(Joystick, Rumble),
        REG_META_FIELD(Joystick, CurrentPowerLevel),
        REG_META_FIELD(Joystick, Close),
    END_REG();
    NEW_META(Joystick);
END_META(1)

/************************
 #                      #
 #    Game Controller   #
 #                      #
 ************************/
static BEGIN_FUNCTION(sdl2, GameControllerOpen)
    CHECK_INTEGER(index);
    SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (!ctrl) lua_pushnil(L);
    else {
        NEW_UDATA(GameController, controller);
        *controller = ctrl;
    }
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, IsGameController)
    CHECK_INTEGER(joy);
    PUSH_BOOLEAN(SDL_IsGameController(joy));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetName)
    PUSH_STRING(SDL_GameControllerName(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetVendor)
    PUSH_INTEGER(SDL_GameControllerGetVendor(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetProduct)
    PUSH_INTEGER(SDL_GameControllerGetProduct(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetProductVersion)
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*self));
END_FUNCTION(1)

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static BEGIN_FUNCTION(sdl2, GameControllerGetAxisFromString)
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetAxisFromString)
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetAxis)
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*self, axis));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetButtonFromString)
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
END_FUNCTION(1)

static BEGIN_FUNCTION(sdl2, GameControllerGetButtonFromString)
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, GetButton)
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetButton(*self, axis));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, Rumble)
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    OPT_INTEGER(freq, 100);
    PUSH_BOOLEAN(SDL_GameControllerRumble(*self, low, high, freq) == 0);
END_FUNCTION(1)

const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};

static BEGIN_META_FUNCTION(GameController, CurrentPowerLevel)
    SDL_Joystick *j = SDL_GameControllerGetJoystick(*self);
    PUSH_STRING(gpad_powerlevels[SDL_JoystickCurrentPowerLevel(j)+1]);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(GameController, Close)
    if (SDL_GameControllerGetAttached(*self))
        SDL_GameControllerClose(*self);
END_FUNCTION(0)

static BEGIN_META(GameController)
    BEGIN_REG(GameController)
        REG_META_FIELD(GameController, GetName),
        REG_META_FIELD(GameController, GetVendor),
        REG_META_FIELD(GameController, GetProduct),
        REG_META_FIELD(GameController, GetProductVersion),
        REG_META_FIELD(GameController, GetAxisFromString),
        REG_META_FIELD(GameController, GetAxis),
        REG_META_FIELD(GameController, GetButtonFromString),
        REG_META_FIELD(GameController, GetButton),
        REG_META_FIELD(GameController, Rumble),
        REG_META_FIELD(GameController, CurrentPowerLevel),
        REG_META_FIELD(GameController, Close),
    END_REG()
    NEW_META(GameController);
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
 #        Utils         #
 #                      #
 ************************/

static BEGIN_FUNCTION(sdl2, GetError)
    PUSH_STRING(SDL_GetError());
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
        REG_FIELD(sdl2, OpenAudioDevice),
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
        // Utils
        REG_FIELD(sdl2, GetError),
    END_REG()
    NEW_MODULE(sdl2);
    LOAD_META(AudioDeviceID);
    LOAD_META(Window);
    LOAD_META(GLContext);
    LOAD_META(Event);
    LOAD_META(Joystick);
    LOAD_META(GameController);
    LOAD_ENUMS(sdl2);
END_MODULE(1)
