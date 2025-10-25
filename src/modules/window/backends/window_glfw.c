#if defined(SELENE_USE_GLFW)

int l_create_GLFW_window(lua_State* L) {
    return 1;
}

int l_selene_Window__destroy(lua_State* L) {
    return 0;
}

int l_selene_Window__get_size(lua_State* L) {
    return 2;
}

int l_selene_Window__set_size(lua_State* L) {
    return 0;
}

int l_selene_Window__get_title(lua_State* L) {
    return 1;
}

int l_selene_Window__set_title(lua_State* L) {
    return 0;
}

#endif /* SELENE_USE_GLFW */
