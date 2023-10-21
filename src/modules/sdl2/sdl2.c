#include "sdl2.h"

extern MODULE_FUNCTION(AudioDeviceID, meta);
extern MODULE_FUNCTION(AudioStream, meta);
extern MODULE_FUNCTION(Window, meta);
extern MODULE_FUNCTION(GLContext, meta);
extern MODULE_FUNCTION(Event, meta);
extern MODULE_FUNCTION(Joystick, meta);
extern MODULE_FUNCTION(Gamepad, meta);

static MODULE_FUNCTION(sdl2, Init) {
    int flags = 0;
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        flags |= luaL_checkinteger(L, i+1);
    }
    int error = SDL_Init(flags);
    PUSH_BOOLEAN(!error);
    return 1;
}

static MODULE_FUNCTION(sdl2, Quit) {
    SDL_Quit();
    return 0;
}

/************************
 #                      #
 #          GL          #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, GL_MakeCurrent) {
    INIT_ARG();
    CHECK_UDATA(Window, win);
    CHECK_UDATA(GLContext, ctx);
    SDL_GL_MakeCurrent(*win, *ctx);
    return 0;
}

static MODULE_FUNCTION(sdl2, GL_GetProcAddress) {
    PUSH_LUDATA(SDL_GL_GetProcAddress);
    return 1;
}

static MODULE_FUNCTION(sdl2, GL_SetAttribute) {
    INIT_ARG();
    CHECK_INTEGER(attr);
    CHECK_INTEGER(value);
    SDL_GL_SetAttribute(attr, value);
    return 0;
}

/************************
 #                      #
 #      Clipboard       #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, GetClipboardText) {
    char* text = SDL_GetClipboardText();
    PUSH_STRING(text);
    SDL_free(text);
    return 1;
}

static MODULE_FUNCTION(sdl2, HasClipboardText) {
    PUSH_BOOLEAN(SDL_HasClipboardText());
    return 1;
}

static MODULE_FUNCTION(sdl2, SetClipboardText) {
    INIT_ARG();
    CHECK_STRING(text);
    SDL_SetClipboardText(text);
    return 0;
}

/************************
 #                      #
 #      Filesystem      #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, GetBasePath) {
    char* path = SDL_GetBasePath();
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
}

static MODULE_FUNCTION(sdl2, GetPrefPath) {
    INIT_ARG();
    CHECK_STRING(org);
    CHECK_STRING(app);
    char* path = SDL_GetPrefPath(org, app);
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
}

/************************
 #                      #
 #        Shared        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, LoadObject) {
    INIT_ARG();
    CHECK_STRING(sofile);
    void* obj = SDL_LoadObject(sofile);
    PUSH_LUDATA(obj);
    return 1;
}

static MODULE_FUNCTION(sdl2, UnloadObject) {
    INIT_ARG();
    CHECK_LUDATA(void, handle);
    SDL_UnloadObject(handle);
    return 0;
}

static MODULE_FUNCTION(sdl2, LoadFunction) {
    INIT_ARG();
    GET_LUDATA(void, handle);
    CHECK_STRING(name);
    void* func = SDL_LoadFunction(handle, name);
    PUSH_LUDATA(func);
    return 1;
}

/************************
 #                      #
 #       Keyboard       #
 #                      #
 ************************/

const Uint8* keys;
static MODULE_FUNCTION(sdl2, CheckKeyState) {
    INIT_ARG();
    CHECK_INTEGER(key);
    PUSH_BOOLEAN(keys[key]);
    return 1;
}

static MODULE_FUNCTION(sdl2, GetKeyboardState) {
    PUSH_LUDATA(SDL_GetKeyboardState(NULL));
    return 1;
}

static MODULE_FUNCTION(sdl2, HasScreenKeyboardSupport) {
    PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());
    return 1;
}

static MODULE_FUNCTION(sdl2, IsScreenKeyboardShown) {
    INIT_ARG();
    CHECK_UDATA(Window, win);
    PUSH_BOOLEAN(SDL_IsScreenKeyboardShown(*win));
    return 1;
}

static MODULE_FUNCTION(sdl2, GetScancodeFromName) {
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GetScancodeFromName(name));
    return 1;
}

static MODULE_FUNCTION(sdl2, GetScancodeName) {
    INIT_ARG();
    CHECK_INTEGER(scancode);
    PUSH_STRING(SDL_GetScancodeName(scancode));
    return 1;
}

/************************
 #                      #
 #         Mouse        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, GetMousePosition) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
}

static MODULE_FUNCTION(sdl2, GetRelativeMousePosition) {
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
}

static MODULE_FUNCTION(sdl2, IsMouseDown) {
    INIT_ARG();
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
    return 1;
}

/************************
 #                      #
 #        Timer         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, GetTicks) {
    PUSH_INTEGER(SDL_GetTicks());
    return 1;
}

static MODULE_FUNCTION(sdl2, Delay) {
    Uint32 ms = (Uint32)luaL_checkinteger(L, 1);
    SDL_Delay(ms);
    return 0;
}

static MODULE_FUNCTION(sdl2, GetPerformanceCounter) {
    PUSH_INTEGER(SDL_GetPerformanceCounter());
    return 1;
}

static MODULE_FUNCTION(sdl2, GetPerformanceFrequency) {
    PUSH_INTEGER(SDL_GetPerformanceFrequency());
    return 1;
}

/************************
 #                      #
 #        Error         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, GetError) {
    PUSH_STRING(SDL_GetError());
    return 1;
}

static MODULE_FUNCTION(sdl2, SetError) {
    INIT_ARG();
    CHECK_STRING(msg);
    SDL_SetError(msg);
    return 0;
}

/************************
 #                      #
 #        System        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, GetPlatform) {
    PUSH_STRING(SDL_GetPlatform());
    return 1;
}

static MODULE_FUNCTION(sdl2, GetCPUCacheLineSize) {
    PUSH_INTEGER(SDL_GetCPUCacheLineSize());
    return 1;
}

static MODULE_FUNCTION(sdl2, GetCPUCount) {
    PUSH_INTEGER(SDL_GetCPUCount());
    return 1;
}

static MODULE_FUNCTION(sdl2, GetSystemRAM) {
    PUSH_INTEGER(SDL_GetSystemRAM());
    return 1;
}

/************************
 #                      #
 #        Enums         #
 #                      #
 ************************/
BEGIN_ENUM(sdl2)
    // Init
    ENUM_FIELD(INIT_EVERYTHING, SDL_),
    ENUM_FIELD(INIT_SENSOR, SDL_),
    ENUM_FIELD(INIT_AUDIO, SDL_),
    ENUM_FIELD(INIT_VIDEO, SDL_),
    ENUM_FIELD(INIT_JOYSTICK, SDL_),
    ENUM_FIELD(INIT_GAMECONTROLLER, SDL_),
    ENUM_FIELD(INIT_TIMER, SDL_),
    ENUM_FIELD(INIT_EVENTS, SDL_),
    // Audio
    {"AUDIO_S16", AUDIO_S16},
    {"AUDIO_S16SYS", AUDIO_S16SYS},
    {"AUDIO_F32", AUDIO_F32},
    // Events
    ENUM_FIELD(QUIT, SDL_),
    ENUM_FIELD(WINDOWEVENT, SDL_),
    ENUM_FIELD(KEYDOWN, SDL_),
    ENUM_FIELD(KEYUP, SDL_),
    ENUM_FIELD(MOUSEMOTION, SDL_),
    ENUM_FIELD(MOUSEBUTTONDOWN, SDL_),
    ENUM_FIELD(MOUSEBUTTONUP, SDL_),
    ENUM_FIELD(AUDIODEVICEADDED, SDL_),
    ENUM_FIELD(AUDIODEVICEREMOVED, SDL_),

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
    ENUM_FIELD(WINDOW_SHOWN, SDL_),
    ENUM_FIELD(WINDOW_OPENGL, SDL_),
    ENUM_FIELD(WINDOW_RESIZABLE, SDL_),
    ENUM_FIELD(WINDOW_FULLSCREEN, SDL_),
    ENUM_FIELD(WINDOW_FULLSCREEN_DESKTOP, SDL_),
    ENUM_FIELD(WINDOW_BORDERLESS, SDL_),
    ENUM_FIELD(WINDOW_ALWAYS_ON_TOP, SDL_),

    ENUM_FIELD(WINDOWPOS_CENTERED, SDL_),
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

BEGIN_MODULE(sdl2) {
    BEGIN_REG()
        REG_FIELD(sdl2, Init),
        REG_FIELD(sdl2, Quit),
        /* GL */
        REG_FIELD(sdl2, GL_MakeCurrent),
        REG_FIELD(sdl2, GL_GetProcAddress),
        REG_FIELD(sdl2, GL_SetAttribute),
        /* Clipboard */
        REG_FIELD(sdl2, GetClipboardText),
        REG_FIELD(sdl2, SetClipboardText),
        REG_FIELD(sdl2, HasClipboardText),
        /* Filesystem  */
        REG_FIELD(sdl2, GetBasePath),
        REG_FIELD(sdl2, GetPrefPath),
        /* Shared  */
        REG_FIELD(sdl2, LoadObject),
        REG_FIELD(sdl2, UnloadObject),
        REG_FIELD(sdl2, LoadFunction),
        /* Keyboard */
        REG_FIELD(sdl2, GetKeyboardState),
        REG_FIELD(sdl2, GetScancodeFromName),
        REG_FIELD(sdl2, HasScreenKeyboardSupport),
        REG_FIELD(sdl2, IsScreenKeyboardShown),
        REG_FIELD(sdl2, CheckKeyState),
        /* Mouse */
        REG_FIELD(sdl2, GetMousePosition),
        REG_FIELD(sdl2, GetRelativeMousePosition),
        REG_FIELD(sdl2, IsMouseDown),
        /* Timer */
        REG_FIELD(sdl2, GetTicks),
        REG_FIELD(sdl2, Delay),
        REG_FIELD(sdl2, GetPerformanceCounter),
        REG_FIELD(sdl2, GetPerformanceFrequency),
        /* System */
        REG_FIELD(sdl2, GetPlatform),
        REG_FIELD(sdl2, GetCPUCount),
        REG_FIELD(sdl2, GetCPUCacheLineSize),
        REG_FIELD(sdl2, GetSystemRAM),
        /* Error */
        REG_FIELD(sdl2, GetError),
        REG_FIELD(sdl2, SetError),
    END_REG()
    luaL_newlib(LUA_STATE_NAME, _reg);
    LOAD_META(AudioDeviceID);
    LOAD_META(AudioStream);
    LOAD_META(AudioStream);
    LOAD_META(Window);
    LOAD_META(GLContext);
    LOAD_META(Event);
    LOAD_META(Joystick);
    LOAD_META(Gamepad);
    LOAD_ENUM(sdl2);
    return 1;
}
