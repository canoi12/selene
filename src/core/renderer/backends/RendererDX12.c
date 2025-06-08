#include "selene_renderer.h"
#if 0 && defined(OS_WIN)
#include <SDL_video.h>

const int dx11_buffer_target_types_values[] = {DX11_VERTEX_BUFFER, DX11_INDEX_BUFFER, DX11_CONSTANT_BUFFER, DX11_STAGING_BUFFER};

int l_DX11_Renderer_create(lua_State* L) {
    INIT_ARG();
    selene_Window* win = (selene_Window*)luaL_checkudata(L, arg++, selene_Window_METANAME);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(win->handle, &wmInfo)) {
        return luaL_error(L, "Cannot get WM Info from SDL_Window: %s", SDL_GetError());
    }
    HWND hwnd = wmInfo.info.win.window;

    NEW_UDATA(SeleneRenderer, self);

    return 1;
}

// create vertex, index or uniform buffer
int l_DX11_Renderer__create_buffer(lua_State* L) {
    CHECK_META(SeleneRenderer);
    int opt = luaL_checkoption(L, arg++, "vertex", buffer_target_types);
    D3D11_BUFFER_DESC desc = {0};
    D3D11_SUBRESOURCE_DATA initData = {0};
    NEW_UDATA(SeleneGpuBuffer, buf);
    buf->type = opt;
    buf->dx11.handle = self->dx11->CreateBuffer(&desc, &initData, &(buf->dx11.handle));
    return 1;
}
#endif