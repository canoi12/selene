#include "sdl2.h"
#include "lauxlib.h"
#include "lua_helper.h"
#include <SDL_keyboard.h>

extern MODULE_FUNCTION(AudioDeviceID, meta);
extern MODULE_FUNCTION(AudioStream, meta);
extern MODULE_FUNCTION(Window, meta);
extern MODULE_FUNCTION(GLContext, meta);
extern MODULE_FUNCTION(Event, meta);
extern MODULE_FUNCTION(Joystick, meta);
extern MODULE_FUNCTION(Gamepad, meta);

static MODULE_FUNCTION(sdl2, init) {
  int flags = 0;
  int args = lua_gettop(L);
  for (int i = 0; i < args; i++) {
    flags |= luaL_checkinteger(L, i + 1);
  }
  int error = SDL_Init(flags);
  PUSH_BOOLEAN(!error);
  return 1;
}

static MODULE_FUNCTION(sdl2, quit) {
  SDL_Quit();
  return 0;
}

/************************
 #                      #
 #          GL          #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, glMakeCurrent) {
  INIT_ARG();
  CHECK_UDATA(Window, win);
  CHECK_UDATA(GLContext, ctx);
  SDL_GL_MakeCurrent(*win, *ctx);
  return 0;
}

static MODULE_FUNCTION(sdl2, glGetProcAddress) {
  PUSH_LUDATA(SDL_GL_GetProcAddress);
  return 1;
}

static MODULE_FUNCTION(sdl2, glSetAttribute) {
  INIT_ARG();
  CHECK_INTEGER(attr);
  CHECK_INTEGER(value);
  SDL_GL_SetAttribute(attr, value);
  return 0;
}

static MODULE_FUNCTION(sdl2, glSetSwapInterval) {
  INIT_ARG();
  GET_BOOLEAN(value);
  int res = SDL_GL_SetSwapInterval(value);
  PUSH_BOOLEAN(res == 0);
  return 1;
}

static MODULE_FUNCTION(sdl2, glGetSwapInterval) {
  int res = SDL_GL_GetSwapInterval();
  PUSH_INTEGER(res);
  return 1;
}

/************************
 #                      #
 #      Clipboard       #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, getClipboardText) {
  char *text = SDL_GetClipboardText();
  PUSH_STRING(text);
  SDL_free(text);
  return 1;
}

static MODULE_FUNCTION(sdl2, hasClipboardText) {
  PUSH_BOOLEAN(SDL_HasClipboardText());
  return 1;
}

static MODULE_FUNCTION(sdl2, setClipboardText) {
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

static MODULE_FUNCTION(sdl2, getBasePath) {
  char *path = SDL_GetBasePath();
  PUSH_STRING(path);
  SDL_free(path);
  return 1;
}

static MODULE_FUNCTION(sdl2, getPrefPath) {
  INIT_ARG();
  CHECK_STRING(org);
  CHECK_STRING(app);
  char *path = SDL_GetPrefPath(org, app);
  PUSH_STRING(path);
  SDL_free(path);
  return 1;
}

/************************
 #                      #
 #        Shared        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, loadObject) {
  INIT_ARG();
  CHECK_STRING(sofile);
  void *obj = SDL_LoadObject(sofile);
  PUSH_LUDATA(obj);
  return 1;
}

static MODULE_FUNCTION(sdl2, unloadObject) {
  INIT_ARG();
  CHECK_LUDATA(void, handle);
  SDL_UnloadObject(handle);
  return 0;
}

static MODULE_FUNCTION(sdl2, loadFunction) {
  INIT_ARG();
  GET_LUDATA(void, handle);
  CHECK_STRING(name);
  void *func = SDL_LoadFunction(handle, name);
  PUSH_LUDATA(func);
  return 1;
}

/************************
 #                      #
 #       Keyboard       #
 #                      #
 ************************/

const Uint8 *keys;
static MODULE_FUNCTION(sdl2, checkKeyState) {
  INIT_ARG();
  CHECK_INTEGER(key);
  PUSH_BOOLEAN(keys[key]);
  return 1;
}

static MODULE_FUNCTION(sdl2, getKeyboardState) {
  PUSH_LUDATA(SDL_GetKeyboardState(NULL));
  return 1;
}

static MODULE_FUNCTION(sdl2, hasScreenKeyboardSupport) {
  PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());
  return 1;
}

static MODULE_FUNCTION(sdl2, isScreenKeyboardShown) {
  INIT_ARG();
  CHECK_UDATA(Window, win);
  PUSH_BOOLEAN(SDL_IsScreenKeyboardShown(*win));
  return 1;
}

static MODULE_FUNCTION(sdl2, getScancodeFromName) {
  INIT_ARG();
  CHECK_STRING(name);
  PUSH_INTEGER(SDL_GetScancodeFromName(name));
  return 1;
}

static MODULE_FUNCTION(sdl2, getScancodeName) {
  INIT_ARG();
  CHECK_INTEGER(scancode);
  PUSH_STRING(SDL_GetScancodeName(scancode));
  return 1;
}

static MODULE_FUNCTION(sdl2, getKeyFromName) {
  INIT_ARG();
  CHECK_STRING(name);
  PUSH_INTEGER(SDL_GetKeyFromName(name));
  return 1;
}

static MODULE_FUNCTION(sdl2, getKeyName) {
  INIT_ARG();
  CHECK_INTEGER(keycode);
  PUSH_STRING(SDL_GetKeyName(keycode));
  return 1;
}

static MODULE_FUNCTION(sdl2, startTextInput) {
  SDL_StartTextInput();
  return 0;
}

static MODULE_FUNCTION(sdl2, stopTextInput) {
  SDL_StopTextInput();
  return 0;
}

/************************
 #                      #
 #         Mouse        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, getMousePosition) {
  int x, y;
  SDL_GetMouseState(&x, &y);
  PUSH_NUMBER(x);
  PUSH_NUMBER(y);
  return 2;
}

static MODULE_FUNCTION(sdl2, getRelativeMousePosition) {
  int x, y;
  SDL_GetRelativeMouseState(&x, &y);
  PUSH_NUMBER(x);
  PUSH_NUMBER(y);
  return 2;
}

static MODULE_FUNCTION(sdl2, isMouseDown) {
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

static MODULE_FUNCTION(sdl2, getTicks) {
  PUSH_INTEGER(SDL_GetTicks());
  return 1;
}

static MODULE_FUNCTION(sdl2, delay) {
  Uint32 ms = (Uint32)luaL_checknumber(L, 1);
  SDL_Delay(ms);
  return 0;
}

static MODULE_FUNCTION(sdl2, getPerformanceCounter) {
  PUSH_INTEGER(SDL_GetPerformanceCounter());
  return 1;
}

static MODULE_FUNCTION(sdl2, getPerformanceFrequency) {
  PUSH_INTEGER(SDL_GetPerformanceFrequency());
  return 1;
}

static MODULE_FUNCTION(sdl2, setTime) { return 1; }

/************************
 #                      #
 #        Error         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl2, getError) {
  PUSH_STRING(SDL_GetError());
  return 1;
}

static MODULE_FUNCTION(sdl2, setError) {
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

static MODULE_FUNCTION(sdl2, getPlatform) {
  PUSH_STRING(SDL_GetPlatform());
  return 1;
}

static MODULE_FUNCTION(sdl2, getCPUCacheLineSize) {
  PUSH_INTEGER(SDL_GetCPUCacheLineSize());
  return 1;
}

static MODULE_FUNCTION(sdl2, getCPUCount) {
  PUSH_INTEGER(SDL_GetCPUCount());
  return 1;
}

static MODULE_FUNCTION(sdl2, getSystemRAM) {
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
ENUM_FIELD(INIT_EVERYTHING, SDL_), ENUM_FIELD(INIT_SENSOR, SDL_),
    ENUM_FIELD(INIT_AUDIO, SDL_), ENUM_FIELD(INIT_VIDEO, SDL_),
    ENUM_FIELD(INIT_JOYSTICK, SDL_), ENUM_FIELD(INIT_GAMECONTROLLER, SDL_),
    ENUM_FIELD(INIT_TIMER, SDL_), ENUM_FIELD(INIT_EVENTS, SDL_),
    // Audio
    {"AUDIO_S16", AUDIO_S16}, {"AUDIO_S16SYS", AUDIO_S16SYS},
    {"AUDIO_F32", AUDIO_F32},
    /* Application events */
    ENUM_FIELD(QUIT, SDL_), ENUM_FIELD(APP_TERMINATING, SDL_),
    ENUM_FIELD(APP_LOWMEMORY, SDL_), ENUM_FIELD(APP_WILLENTERBACKGROUND, SDL_),
    ENUM_FIELD(APP_DIDENTERBACKGROUND, SDL_),
    ENUM_FIELD(APP_WILLENTERFOREGROUND, SDL_),
    ENUM_FIELD(APP_DIDENTERFOREGROUND, SDL_), ENUM_FIELD(LOCALECHANGED, SDL_),

    /* Display events */
    ENUM_FIELD(DISPLAYEVENT, SDL_),

    /* Window events */
    ENUM_FIELD(WINDOWEVENT, SDL_),

    /* Keyboard events */
    ENUM_FIELD(KEYDOWN, SDL_), ENUM_FIELD(KEYUP, SDL_),
    ENUM_FIELD(TEXTEDITING, SDL_), ENUM_FIELD(TEXTINPUT, SDL_),
    ENUM_FIELD(KEYMAPCHANGED, SDL_), ENUM_FIELD(TEXTEDITING_EXT, SDL_),

    /* Mouse events */
    ENUM_FIELD(MOUSEMOTION, SDL_), ENUM_FIELD(MOUSEBUTTONDOWN, SDL_),
    ENUM_FIELD(MOUSEBUTTONUP, SDL_), ENUM_FIELD(MOUSEWHEEL, SDL_),

    /* Joystick events */
    ENUM_FIELD(JOYAXISMOTION, SDL_), ENUM_FIELD(JOYBALLMOTION, SDL_),
    ENUM_FIELD(JOYHATMOTION, SDL_), ENUM_FIELD(JOYBUTTONDOWN, SDL_),
    ENUM_FIELD(JOYBUTTONUP, SDL_), ENUM_FIELD(JOYDEVICEADDED, SDL_),
    ENUM_FIELD(JOYDEVICEREMOVED, SDL_), ENUM_FIELD(JOYBATTERYUPDATED, SDL_),

    /* Game controller events */
    ENUM_FIELD(CONTROLLERAXISMOTION, SDL_),
    ENUM_FIELD(CONTROLLERBUTTONDOWN, SDL_),
    ENUM_FIELD(CONTROLLERBUTTONUP, SDL_),
    ENUM_FIELD(CONTROLLERDEVICEADDED, SDL_),
    ENUM_FIELD(CONTROLLERDEVICEREMOVED, SDL_),
    ENUM_FIELD(CONTROLLERDEVICEREMAPPED, SDL_),
    ENUM_FIELD(CONTROLLERTOUCHPADDOWN, SDL_),
    ENUM_FIELD(CONTROLLERTOUCHPADMOTION, SDL_),
    ENUM_FIELD(CONTROLLERTOUCHPADUP, SDL_),
    ENUM_FIELD(CONTROLLERSENSORUPDATE, SDL_),

    /* Touch events */
    ENUM_FIELD(FINGERDOWN, SDL_), ENUM_FIELD(FINGERUP, SDL_),
    ENUM_FIELD(FINGERMOTION, SDL_),

    /* Gesture events */
    ENUM_FIELD(DOLLARGESTURE, SDL_), ENUM_FIELD(DOLLARRECORD, SDL_),
    ENUM_FIELD(MULTIGESTURE, SDL_),

    /* Clipboard events */
    ENUM_FIELD(CLIPBOARDUPDATE, SDL_),

    /* Drag and drop events */
    ENUM_FIELD(DROPFILE, SDL_), ENUM_FIELD(DROPTEXT, SDL_),
    ENUM_FIELD(DROPBEGIN, SDL_), ENUM_FIELD(DROPCOMPLETE, SDL_),

    /* Audio hotplug events */
    ENUM_FIELD(AUDIODEVICEADDED, SDL_), ENUM_FIELD(AUDIODEVICEREMOVED, SDL_),

    /* Sensor events */
    ENUM_FIELD(SENSORUPDATE, SDL_),

    /* Render events */
    ENUM_FIELD(RENDER_TARGETS_RESET, SDL_),
    ENUM_FIELD(RENDER_DEVICE_RESET, SDL_),

    // Window Events
    ENUM_FIELD(WINDOWEVENT_CLOSE, SDL_),
    ENUM_FIELD(WINDOWEVENT_SIZE_CHANGED, SDL_),
    ENUM_FIELD(WINDOWEVENT_RESIZED, SDL_), ENUM_FIELD(WINDOWEVENT_MOVED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MINIMIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MAXIMIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_RESTORED, SDL_), ENUM_FIELD(WINDOWEVENT_SHOWN, SDL_),
    ENUM_FIELD(WINDOWEVENT_HIDDEN, SDL_), ENUM_FIELD(WINDOWEVENT_ENTER, SDL_),
    ENUM_FIELD(WINDOWEVENT_LEAVE, SDL_),
    ENUM_FIELD(WINDOWEVENT_FOCUS_GAINED, SDL_),
    ENUM_FIELD(WINDOWEVENT_FOCUS_LOST, SDL_),
    ENUM_FIELD(WINDOWEVENT_TAKE_FOCUS, SDL_),
    ENUM_FIELD(WINDOWEVENT_HIT_TEST, SDL_),

    // Window Flags
    ENUM_FIELD(WINDOW_SHOWN, SDL_), ENUM_FIELD(WINDOW_OPENGL, SDL_),
    ENUM_FIELD(WINDOW_RESIZABLE, SDL_), ENUM_FIELD(WINDOW_FULLSCREEN, SDL_),
    ENUM_FIELD(WINDOW_FULLSCREEN_DESKTOP, SDL_),
    ENUM_FIELD(WINDOW_BORDERLESS, SDL_), ENUM_FIELD(WINDOW_ALWAYS_ON_TOP, SDL_),

    ENUM_FIELD(WINDOWPOS_CENTERED, SDL_),
    // Message Box
    ENUM_FIELD(MESSAGEBOX_ERROR, SDL_), ENUM_FIELD(MESSAGEBOX_WARNING, SDL_),
    ENUM_FIELD(MESSAGEBOX_INFORMATION, SDL_),
    // GL
    ENUM_FIELD(GL_DOUBLEBUFFER, SDL_), ENUM_FIELD(GL_DEPTH_SIZE, SDL_),
    ENUM_FIELD(GL_STENCIL_SIZE, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_MASK, SDL_),
    ENUM_FIELD(GL_CONTEXT_MAJOR_VERSION, SDL_),
    ENUM_FIELD(GL_CONTEXT_MINOR_VERSION, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_CORE, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_COMPATIBILITY, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_ES, SDL_),
    END_ENUM()

        BEGIN_MODULE(sdl2) {
  keys = SDL_GetKeyboardState(NULL);
  BEGIN_REG()
  REG_FIELD(sdl2, init), REG_FIELD(sdl2, quit),
      /* GL */
      REG_FIELD(sdl2, glMakeCurrent), REG_FIELD(sdl2, glGetProcAddress),
      REG_FIELD(sdl2, glSetAttribute), REG_FIELD(sdl2, glSetSwapInterval),
      REG_FIELD(sdl2, glGetSwapInterval),
      /* Clipboard */
      REG_FIELD(sdl2, getClipboardText), REG_FIELD(sdl2, setClipboardText),
      REG_FIELD(sdl2, hasClipboardText),
      /* Filesystem  */
      REG_FIELD(sdl2, getBasePath), REG_FIELD(sdl2, getPrefPath),
      /* Shared  */
      REG_FIELD(sdl2, loadObject), REG_FIELD(sdl2, unloadObject),
      REG_FIELD(sdl2, loadFunction),
      /* Keyboard */
      REG_FIELD(sdl2, getKeyboardState), REG_FIELD(sdl2, getScancodeFromName),
      REG_FIELD(sdl2, getScancodeName), REG_FIELD(sdl2, getKeyFromName),
      REG_FIELD(sdl2, getKeyName), REG_FIELD(sdl2, hasScreenKeyboardSupport),
      REG_FIELD(sdl2, isScreenKeyboardShown), REG_FIELD(sdl2, checkKeyState),
      REG_FIELD(sdl2, startTextInput), REG_FIELD(sdl2, stopTextInput),
      /* Mouse */
      REG_FIELD(sdl2, getMousePosition),
      REG_FIELD(sdl2, getRelativeMousePosition), REG_FIELD(sdl2, isMouseDown),
      /* Timer */
      REG_FIELD(sdl2, getTicks), REG_FIELD(sdl2, delay),
      REG_FIELD(sdl2, getPerformanceCounter),
      REG_FIELD(sdl2, getPerformanceFrequency),
      /* System */
      REG_FIELD(sdl2, getPlatform), REG_FIELD(sdl2, getCPUCount),
      REG_FIELD(sdl2, getCPUCacheLineSize), REG_FIELD(sdl2, getSystemRAM),
      /* Error */
      REG_FIELD(sdl2, getError), REG_FIELD(sdl2, setError),
      END_REG() luaL_newlib(LUA_STATE_NAME, _reg);
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
