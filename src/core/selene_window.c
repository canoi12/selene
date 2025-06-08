#include "selene.h"
#include "lua_helper.h"

#if defined(SELENE_USE_GLFW)
static int l_create_GLFW_window(lua_State* L) {
    return 1;
}
#else
static int l_create_SDL_window(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(title);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int x = SDL_WINDOWPOS_CENTERED;
    int y = SDL_WINDOWPOS_CENTERED;
    int flags = SDL_WINDOW_SHOWN;
    if (lua_istable(L, arg)) {
        lua_getfield(L, arg, "x");
        x = (int)luaL_optinteger(L, -1, x);
        lua_pop(L, 1);

        lua_getfield(L, arg, "y");
        y = (int)luaL_optinteger(L, -1, y);
        lua_pop(L, 1);

        if (lua_getfield(L, arg, "resizable") == LUA_TBOOLEAN) flags |= SDL_WINDOW_RESIZABLE * lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "always_on_top") == LUA_TBOOLEAN) flags |= SDL_WINDOW_ALWAYS_ON_TOP * lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "borderless") == LUA_TBOOLEAN) flags |= SDL_WINDOW_BORDERLESS * lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "opengl") == LUA_TBOOLEAN) flags |= SDL_WINDOW_OPENGL * lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "vulkan") == LUA_TBOOLEAN) flags |= SDL_WINDOW_VULKAN * lua_toboolean(L, -1);
        lua_pop(L, 2);
    }
    if (flags & SDL_WINDOW_OPENGL) {
#if defined(OS_EMSCRIPTEN) || defined(OS_ANDROID)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    }
#ifndef SELENE_NO_VULKAN
    if (flags & SDL_WINDOW_VULKAN) {
        if (SDL_Vulkan_LoadLibrary(NULL) != 0) {
            return luaL_error(L, "failed to load Vulkan library: %s", SDL_GetError());
        }
    }
#endif
    SDL_Window *win = SDL_CreateWindow(
        title,
#if !defined(SELENE_USE_SDL3)
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
#endif
        width, height, flags
    );
    if (win == NULL)
        return luaL_error(L, "failed to create window: %s", SDL_GetError());
    selene_Window *win_ptr = (selene_Window*)lua_newuserdata(L, sizeof(SDL_Window *));
    luaL_setmetatable(L, selene_Window_METANAME);
    win_ptr->handle = win;
    return 1;
}
#endif

int l_create_window(lua_State* L) {
#if defined(SELENE_USE_GLFW)
    return l_create_GLFW_window(L);
#else
    return l_create_SDL_window(L);
#endif
}

static int l_selene_Window__destroy(lua_State* L) {
    CHECK_META(selene_Window);
#if defined(SELENE_USE_GLFW)
#else
    SDL_DestroyWindow(self->handle);
#endif
    return 0;
}

static int l_selene_Window__get_size(lua_State* L) {
    CHECK_META(selene_Window);
    int width, height;
#if defined(SELENE_USE_GLFW)
#else
    SDL_GetWindowSize(self->handle, &width, &height);
#endif
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int l_selene_Window__set_size(lua_State* L) {
    CHECK_META(selene_Window);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
#if defined(SELENE_USE_GLFW)
#else
    SDL_SetWindowSize(self->handle, width, height);
#endif
    return 0;
}

int luaopen_window(lua_State* L) {
    luaL_newmetatable(L, selene_Window_METANAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg _reg[] = {
        REG_META_FIELD(selene_Window, destroy),
        REG_META_FIELD(selene_Window, get_size),
        REG_META_FIELD(selene_Window, set_size),
        {NULL, NULL}
    };
    luaL_setfuncs(L, _reg, 0);
    return 1;
}