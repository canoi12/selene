#include <selene.h>
#include <lua_helper.h>

#ifdef SELENE_PLUGINS_PRELOAD
extern int luaopen_plugins(lua_State *L);
#endif

#if USE_JIT
void lua_rawsetp(lua_State* L, int idx, void* p) {
    int value = (int)p;
    lua_rawseti(L, idx, value);
}
int lua_rawgetp(lua_State* L, int idx, const void* p) {
    int value = (int)p;
    lua_rawgeti(L, idx, value);
}
void luaL_requiref (lua_State *L, const char *modname,
                               lua_CFunction openf, int glb) {
  lua_pushcfunction(L, openf);
  lua_pushstring(L, modname);  /* argument to open function */
  lua_call(L, 1, 1);  /* open module */
  lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_pushvalue(L, -2);  /* make copy of module (call result) */
  lua_setfield(L, -2, modname);  /* _LOADED[modname] = module */
  lua_pop(L, 1);  /* remove _LOADED table */
  if (glb) {
    lua_pushvalue(L, -1);  /* copy of 'mod' */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}
#endif

int selene_running = 0;

// Type Modules
extern int luaopen_Data(lua_State *L);

// Library Modules
#ifndef SELENE_NO_AUDIO
extern int luaopen_audio(lua_State* L);
#endif
extern int luaopen_fs(lua_State *L);
extern int luaopen_gl(lua_State *L);
extern int luaopen_linmath(lua_State *L);
#ifndef SELENE_NO_JSON
extern int luaopen_json(lua_State* L);
#endif
#ifndef SELENE_NO_IMAGE
extern int luaopen_image(lua_State* L);
#endif
#ifndef SELENE_NO_FONT
extern int luaopen_font(lua_State* L);
#endif
#ifndef SELENE_NO_SDL
extern int luaopen_sdl(lua_State *L);
#endif


#ifndef SELENE_NO_SDL
static int l_load_from_sdl_rwops(lua_State *L) {
  const char *module_name = luaL_checkstring(L, 1);
  size_t len = strlen(module_name);
  char path[256];
  strcpy(path, module_name);
  for (int i = 0; i < len; i++) {
    if (path[i] == '.')
      path[i] = '/';
  }
  strcat(path, ".lua");
#if DEBUG && defined(OS_ANDROID)
  __android_log_print(ANDROID_LOG_DEBUG, "selene", "[%s] load file: %s",
                      module_name, path);
#endif
  SDL_RWops *rw = SDL_RWFromFile(path, "r");
  if (!rw) {
    strcpy(path, module_name);
    for (int i = 0; i < len; i++) {
      if (path[i] == '.')
        path[i] = '/';
    }
    strcat(path, "/init.lua");
    rw = SDL_RWFromFile(path, "r");
    if (!rw) {
      lua_pushfstring(L, "[selene] RWops: failed to open module: %s", module_name);
      return 1;
    }
  }
  size_t size = SDL_RWsize(rw);
  char *content = malloc(size);
  SDL_RWread(rw, content, 1, size);
  if (luaL_loadbuffer(L, content, size, path) != LUA_OK) {
    free((void *)content);
    lua_pushfstring(L, "[selene] failed to parse Lua file: %s", path);
    return 1;
  }
  free((void *)content);
  return 1;
}
#endif

luaL_Reg _mod_regs[] = {
#ifndef SELENE_NO_AUDIO
  {"audio", luaopen_audio},
#endif
  {"fs", luaopen_fs},
#ifndef SELENE_NO_GL
  {"gl", luaopen_gl},
#endif
  {"linmath", luaopen_linmath},
#ifndef SELENE_NO_JSON
  {"json", luaopen_json},
#endif
#ifndef SELENE_NO_FONT
  {"font", luaopen_font},
#endif
#ifndef SELENE_NO_IMAGE
  {"image", luaopen_image},
#endif
#ifndef SELENE_NO_SDL
  {"sdl", luaopen_sdl},
#endif
  {NULL, NULL}
};

static int l_string_utf8codepoint(lua_State *L) {
  INIT_ARG();
  CHECK_STRING(str);
  CHECK_INTEGER(pos);
  uint8_t *p = (uint8_t *)str + pos - 1;
  if (*p < 0x80) {
    PUSH_INTEGER(*p);
    PUSH_INTEGER(1);
    return 2;
  }
  int codepoint = *p;
  int size = 1;

  switch (codepoint & 0xf0) {
  case 0xf0:
    codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) |
                ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
    size = 4;
    break;
  case 0xe0: {
    codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
    size = 3;
    break;
  }
  case 0xc0:
  case 0xd0: {
    codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
    size = 2;
    break;
  }
  default:
    codepoint = -1;
  }
  if (codepoint > SELENE_MAX_UNICODE)
    codepoint = -1;
  PUSH_INTEGER(codepoint);
  PUSH_INTEGER(size);
  return 2;
}

static int l_selene_get_exec_path(lua_State *L) {
  char path[1024];
#ifndef SELENE_NO_SDL
  PUSH_STRING(SDL_GetBasePath());
#else
#if defined(_WIN32)
  GetModuleFileNameA(NULL, path, 1024);
  int index = 0;
  char *p = path + strlen(path);
  while (*p != '\\') {
    p--;
  }
  p[1] = '\0';
  lua_pushstring(L, path);
#elif defined(__unix__) && !defined(__EMSCRIPTEN__)
#if defined(__linux__)
  const char *proc = "/proc/self/exe";
#elif defined(__FreeBSD__)
  const char *proc = "/proc/curproc/file";
#else
  const char *proc = "/proc/self/path/a.out";
#endif
  size_t len = readlink(proc, path, 1024);
  char *p = path + len;
  while (*p != '/') {
    p--;
  }
  p[1] = '\0';
  lua_pushstring(L, path);
#else
  lua_pushnil(L);
#endif
#endif
  return 1;
}

static int l_os_host(lua_State *L) {
#if defined(OS_WIN)
  lua_pushstring(L, "windows");
#elif defined(OS_LINUX)
  lua_pushstring(L, "linux");
#elif defined(OS_OSX)
  lua_pushstring(L, "macosx");
#elif defined(OS_EMSCRIPTEN)
  lua_pushstring(L, "emscripten");
#elif defined(OS_ANDROID)
  lua_pushstring(L, "android");
#elif defined(OS_BSD)
  lua_pushstring(L, "bsd");
#elif defined(OS_NSWITCH)
  lua_pushstring(L, "switch");
#elif defined(OS_PS4)
  lua_pushstring(L, "ps4");
#elif defined(OS_PS5)
  lua_pushstring(L, "ps5");
#elif defined(OS_XBONE)
  lua_pushstring(L, "xbone");
#elif defined(OS_XB360)
  lua_pushstring(L, "xb360");
#else
  lua_pushstring(L, "unknown");
#endif
  return 1;
}

static int l_os_arch(lua_State *L) {
#if defined(ARCH_x86)
  lua_pushstring(L, "x86");
#elif defined(ARCH_X64)
  lua_pushstring(L, "x64");
#elif defined(ARCH_ARM)
  lua_pushstring(L, "arm");
#else
  lua_pushnil(L);
#endif
  return 1;
}

static void setup_extended_libs(lua_State *L) {
  /* String */
  lua_getglobal(L, "string");
  lua_pushcfunction(L, l_string_utf8codepoint);
  lua_setfield(L, -2, "utf8codepoint");
  lua_pop(L, 1);

  /* OS */
  lua_getglobal(L, "os");
  lua_pushcfunction(L, l_os_host);
  lua_setfield(L, -2, "host");
  lua_pushcfunction(L, l_os_arch);
  lua_setfield(L, -2, "arch");
  lua_pop(L, 1);
}

static int l_selene_get_version(lua_State *L) {
  PUSH_STRING(SELENE_VERSION);
  return 1;
}

static int l_selene_create_data(lua_State* L) {
    INIT_ARG();
    size_t size = luaL_checkinteger(L, arg++);
    if (size < 1)
      return luaL_argerror(L, arg-1, "size must be greather than zero");
    TEST_UDATA(Data, copy);
    Data* data = (Data*)lua_newuserdata(L, sizeof(Data)+size);
    luaL_setmetatable(L, "Data");
    *data = size;
    if (copy) {
        int copy_size = size > *copy ? *copy : size;
        memcpy(data+1, copy+1, copy_size);
    }
    return 1;
}

static int l_selene_cube_data(lua_State* L) {
    lua_newtable(L);
    float vertices[] = {
        // Face 1 (frontal)
         0.5,  0.5,  0.5, // Vértice 1
        -0.5,  0.5,  0.5, // Vértice 2
        -0.5, -0.5,  0.5, // Vértice 3
         0.5, -0.5,  0.5, // Vértice 4

        // Face 2 (traseira)
         0.5,  0.5, -0.5, // Vértice 5
        -0.5,  0.5, -0.5, // Vértice 6
        -0.5, -0.5, -0.5, // Vértice 7
         0.5, -0.5, -0.5, // Vértice 8

        // Face 3 (esquerda)
        -0.5,  0.5,  0.5, // Vértice 9
        -0.5,  0.5, -0.5, // Vértice 10
        -0.5, -0.5, -0.5, // Vértice 11
        -0.5, -0.5,  0.5, // Vértice 12

        // Face 4 (direita)
         0.5,  0.5,  0.5, // Vértice 13
         0.5,  0.5, -0.5, // Vértice 14
         0.5, -0.5, -0.5, // Vértice 15
         0.5, -0.5,  0.5, // Vértice 16

        // Face 5 (superior)
         0.5,  0.5, -0.5, // Vértice 17
        -0.5,  0.5, -0.5, // Vértice 18
        -0.5,  0.5,  0.5, // Vértice 19
         0.5,  0.5,  0.5, // Vértice 20

        // Face 6 (inferior)
         0.5, -0.5, -0.5, // Vértice 21
        -0.5, -0.5, -0.5, // Vértice 22
        -0.5, -0.5,  0.5, // Vértice 23
         0.5, -0.5,  0.5  // Vértice 24
    };
    float* verts = lua_newuserdata(L, sizeof(vertices));
    memcpy(verts, vertices, sizeof(vertices));
    lua_setfield(L, -2, "vertices");
    lua_pushinteger(L, 24);
    lua_setfield(L, -2, "num_vertices");
    unsigned int indices[] = {
        // Frente
        0, 1, 2,   0, 2, 3,
        // Trás
        4, 5, 6,   4, 6, 7,
        // Esquerda
        8, 9, 10,  8, 10, 11,
        // Direita
        12, 13, 14, 12, 14, 15,
        // Topo
        16, 17, 18, 16, 18, 19,
        // Fundo
        20, 21, 22, 20, 22, 23
    };
    float* inds = lua_newuserdata(L, sizeof(indices));
    memcpy(inds, indices, sizeof(indices));
    lua_setfield(L, -2, "indices");
    lua_pushinteger(L, 36);
    lua_setfield(L, -2, "num_indices");
    return 1;
}

static int l_selene_set_running(lua_State *L) {
  INIT_ARG();
  CHECK_BOOLEAN(running);
  selene_running = running;
  return 0;
}


int luaopen_selene(lua_State *L) {
  luaL_Reg reg[] = {
    {"get_version", l_selene_get_version},
    {"create_data", l_selene_create_data},
    {"cube_data", l_selene_cube_data},
    {"set_running", l_selene_set_running},
    {"get_exec_path", l_selene_get_exec_path},
    {NULL, NULL}
  };
  luaL_newlib(L, reg);
  // LOAD_MODULE(AudioDecoder);
  LOAD_MODULE(Data);

#ifndef SELENE_NO_SDL
  lua_getglobal(L, "package");
#if USE_JIT
  lua_getfield(L, -1, "loaders");
#else
  lua_getfield(L, -1, "searchers");
#endif
  lua_pushcfunction(L, l_load_from_sdl_rwops);
  lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
  lua_pop(L, 2);
#endif

  setup_extended_libs(L);
  int i;
  for (i = 0; _mod_regs[i].name != NULL; i++) {
    luaL_requiref(L, _mod_regs[i].name, _mod_regs[i].func, 1);
    lua_pop(L, 1);
  }

#ifdef SELENE_PLUGINS_PRELOAD
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");
  lua_pushcfunction(L, luaopen_plugins);
  lua_setfield(L, -2, "plugins");
  lua_pop(L, 2);
#endif

  return 1;
}

