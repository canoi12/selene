#include "modules/renderer.h"
#include "modules/window.h"
#if defined(OS_WIN) && !defined(SELENE_NO_D3D11)
#include <SDL_syswm.h>

extern "C" char font8x8_basic[128][8];
extern "C" char font8x8_control[32][8];
extern "C" char font8x8_ext_latin[96][8];

//const int dx11_buffer_target_types_values[] = {DX11_VERTEX_BUFFER, DX11_INDEX_BUFFER, DX11_CONSTANT_BUFFER, DX11_STAGING_BUFFER};
const int dx11_buffer_target_types_values[] = {D3D11_BIND_VERTEX_BUFFER, D3D11_BIND_INDEX_BUFFER, D3D11_BIND_CONSTANT_BUFFER, DX11_STAGING_BUFFER};
const D3D11_PRIMITIVE_TOPOLOGY dx11_primitive_topologies[] = {
        D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
};
const D3D11_COMPARISON_FUNC dx11_comparison_funcs[] = {
        D3D11_COMPARISON_NEVER,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_ALWAYS
};
const D3D11_CULL_MODE dx11_cull_mode[] = {
        D3D11_CULL_NONE,
        D3D11_CULL_FRONT,
        D3D11_CULL_BACK
};

const DXGI_FORMAT dx11_pixel_formats_values[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT };

int l_DX11_Renderer__destroy(lua_State* L) {
    CHECK_META(selene_Renderer);
    luaL_unref(L, LUA_REGISTRYINDEX, self->l_window_ref);
    free(self->command_pool);
    if (self->default_target.dx11.rtv) self->default_target.dx11.rtv->Release();
    if (self->default_target.dx11.dsv) self->default_target.dx11.dsv->Release();
    if (self->default_target.depth) {
        self->default_target.depth->dx11.tex2d->Release();
        free(self->default_target.depth);
    }
    self->dx11.swap_chain->Release();
    self->dx11.context->Release();
    self->dx11.device->Release();
    return 0;
}

/**
 * selene_RenderPipeline
 */

int l_DX11_Renderer__create_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (!lua_istable(L, arg)) return luaL_argerror(L, arg, "must be a table");

    lua_getfield(L, arg, "vs");
    selene_Shader* vertex = (selene_Shader*)luaL_checkudata(L, -1, selene_Shader_METANAME);
    lua_getfield(L, arg, "ps");
    selene_Shader* pixel = (selene_Shader*)luaL_checkudata(L, -1, selene_Shader_METANAME);
    lua_pop(L, 2);

    if (lua_getfield(L, arg, "layout") != LUA_TTABLE) return luaL_error(L, "invalid layout field");
    NEW_UDATA(selene_RenderPipeline, pipe);
    memset(pipe, 0, sizeof(*pipe));
    if (lua_getfield(L, -2, "stride") == LUA_TNUMBER) pipe->layout.vertex_stride = lua_tointeger(L, -1);
    lua_pop(L, 1);

    pipe->dx11.vs = vertex->dx11.vertex;
    pipe->dx11.ps = pixel->dx11.pixel;

    int len = lua_rawlen(L, -2);
    pipe->layout.count = len;
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -2, i+1);
        lua_getfield(L, -1, "name");
        const char* name = luaL_checkstring(L, -1);
        pipe->layout.attributes[i].binding = 0;
        SDL_strlcpy(pipe->layout.attributes[i].name, name, 64);

        lua_pop(L, 1);
        lua_getfield(L, -1, "offset");
        pipe->layout.attributes[i].offset = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "size");
        const int size = lua_tointeger(L, -1);
        pipe->layout.attributes[i].size = size;
        lua_pop(L, 1);

        lua_getfield(L, -1, "type");
        int opt = luaL_checkoption(L, -1, NULL, type_name_options);
        pipe->layout.attributes[i].type = opt;
        lua_pop(L, 1);
        // fprintf(stdout, "name: %s, offset: %d, size: %d, type: %d\n", name, pipe->layout.attributes[i].offset, size, pipe->layout.attributes[i].type);
        switch (opt) {
            case 0:
            case 1:
                pipe->layout.attributes[i].stride = size;
                break;
            case 2:
            case 3:
                pipe->layout.attributes[i].stride = size * 2;
                break;
            default:
                pipe->layout.attributes[i].stride = size * 4;
        }
        lua_pop(L, 1);
    }
    fprintf(stdout, "registered layout\n");

    D3D11_INPUT_ELEMENT_DESC layout_desc[8];
    for (int i = 0; i < len; i++) {
        D3D11_INPUT_ELEMENT_DESC* l = layout_desc + i;
        int binding = pipe->layout.attributes[i].binding;
        int offset = pipe->layout.attributes[i].offset;
        int size = pipe->layout.attributes[i].size;
        int type = pipe->layout.attributes[i].type;
        int stride = pipe->layout.vertex_stride;
        l->SemanticName = pipe->layout.attributes[i].name;
        l->SemanticIndex = 0;
        switch (type) {
            case SELENE_FLOAT: {
                if (size == 1) l->Format = DXGI_FORMAT_R32_FLOAT;
                else if (size == 2) l->Format = DXGI_FORMAT_R32G32_FLOAT;
                else if (size == 3) l->Format = DXGI_FORMAT_R32G32B32_FLOAT;
                else if (size == 4) l->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
            break;
        }
        l->InputSlot = 0;
        l->AlignedByteOffset = offset;
        l->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        l->InstanceDataStepRate = 0;
    }
    ID3D11InputLayout* layout;
    self->dx11.device->CreateInputLayout(
            layout_desc,
            len,
            vertex->dx11.blob->GetBufferPointer(),
            vertex->dx11.blob->GetBufferSize(),
            &layout);
    pipe->dx11.input_layout = layout;
    pipe->dx11.blend_state = NULL;
    D3D11_BLEND_DESC blendDesc = {0};
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    if (lua_getfield(L, arg, "blend") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) blendDesc.RenderTarget[0].BlendEnable = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "func") == LUA_TSTRING) {
            const char* blendmode = luaL_optstring(L, -1, "alpha");
            if (strcmp(blendmode, "alpha") == 0) {
                //blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
                //blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
                blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
                blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
                //fprintf(stdout, "entrando aqui\n");
            }
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    if (blendDesc.RenderTarget[0].BlendEnable) {
        self->dx11.device->CreateBlendState(&blendDesc, &(pipe->dx11.blend_state));
    }

    D3D11_DEPTH_STENCIL_DESC depthDesc = {0};
    depthDesc.DepthEnable = FALSE;
    depthDesc.StencilEnable = FALSE;
    if (lua_getfield(L, arg, "depth") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) depthDesc.DepthEnable = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "func") == LUA_TSTRING) {
            int opt = luaL_checkoption(L, -1, NULL, comparison_func_options);
            // fprintf(stdout, "depth func: %d\n", opt);
            depthDesc.DepthFunc = dx11_comparison_funcs[opt];
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    if (depthDesc.DepthEnable) {
        depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ID3D11DepthStencilState* ds_state;
        self->dx11.device->CreateDepthStencilState(&depthDesc, &ds_state);
        pipe->dx11.depth_stencil_state = ds_state;
        fprintf(stdout, "depth state\n");
    }
    D3D11_RASTERIZER_DESC rasterDesc = {0};
    rasterDesc.ScissorEnable = FALSE;
    rasterDesc.DepthClipEnable = FALSE;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.ScissorEnable = FALSE;
    if (lua_getfield(L, arg, "cull") == LUA_TTABLE) {
        //if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) pipe->gl.rasterizer_state.cull_enabled = lua_toboolean(L, -1);
        //lua_pop(L, 1);
        if (lua_getfield(L, -1, "front_face") == LUA_TSTRING) {
            int opt = luaL_checkoption(L, -1, "cw", front_face_options);
            rasterDesc.FrontCounterClockwise = opt == 1;
        }
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "cull_face") == LUA_TSTRING) {
            int opt = luaL_checkoption(L, -1, "back", cull_face_options);
            rasterDesc.CullMode = dx11_cull_mode[opt];
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    if (lua_getfield(L, arg, "scissor") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) rasterDesc.ScissorEnable = lua_toboolean(L, -1);
        fprintf(stdout, "Enabled scissor: %d\n", rasterDesc.ScissorEnable);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    ID3D11RasterizerState* raster_state = NULL;
    self->dx11.device->CreateRasterizerState(&rasterDesc, &raster_state);
    pipe->dx11.rasterizer_state = raster_state;

    return 1;
}

int l_DX11_Renderer__destroy_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderPipeline, pipe);
    if (pipe->dx11.blend_state) pipe->dx11.blend_state->Release();
    if (pipe->dx11.depth_stencil_state) pipe->dx11.depth_stencil_state->Release();
    if (pipe->dx11.rasterizer_state) pipe->dx11.rasterizer_state->Release();
    if (pipe->dx11.input_layout) pipe->dx11.input_layout->Release();
    return 0;
}

// create vertex, index or uniform buffer
int l_DX11_Renderer__create_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "vertex", buffer_target_options);
    int size = (int)luaL_checkinteger(L, arg++);
    D3D11_BUFFER_DESC desc = {0};
    desc.ByteWidth = size;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = dx11_buffer_target_types_values[opt];
    fprintf(stdout, "dx11 binding: %d %d\n", opt, dx11_buffer_target_types_values[opt]);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    NEW_UDATA(selene_GpuBuffer, buf);
    buf->type = opt;
    ID3D11Buffer* handle;
    HRESULT hr = self->dx11.device->CreateBuffer(&desc, NULL, &handle);
    if (FAILED(hr)) {
        char buffer[256];
        //DXGetErrorStringA(hr, buffer, sizeof(buffer));
        sprintf(buffer, "failed to create DX11 Buffer: 0x%.8x", hr);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }
    buf->dx11.handle = handle;
    buf->size = size;
    fprintf(stdout, "buffer created: %p\n", buf->dx11.handle);
    return 1;
}

int l_DX11_Renderer__destroy_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    buffer->dx11.handle->Release();
    return 1;
}

int l_DX11_Renderer__send_buffer_data(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(size);
    if (!lua_isuserdata(L, arg)) return luaL_argerror(L, arg, "userdata or lightuserdata expected");
    void* data = lua_touserdata(L, arg++);
    if (buffer->size < size) { // Release and Recreate buffer
        fprintf(stdout, "[DX11] Release and Realloc GpuBuffer, old size: %d, new size: %d\n", buffer->size, size);
        buffer->dx11.handle->Release();
        D3D11_BUFFER_DESC desc = { 0 };
        desc.ByteWidth = size;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = dx11_buffer_target_types_values[buffer->type];
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        ID3D11Buffer* handle;
        HRESULT hr = self->dx11.device->CreateBuffer(&desc, NULL, &handle);
        if (FAILED(hr)) {
            char buffer[256];
            //DXGetErrorStringA(hr, buffer, sizeof(buffer));
            sprintf(buffer, "failed to create DX11 Buffer: 0x%.8x", hr);
            lua_pushstring(L, buffer);
            lua_error(L);
            return 0;
        }
        buffer->dx11.handle = handle;
        buffer->size = size;
    }
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result = self->dx11.context->Map(
            buffer->dx11.handle,
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mappedResource
    );
    memcpy(mappedResource.pData, data, size);
    self->dx11.context->Unmap(buffer->dx11.handle, 0);
    return 0;
}

int l_DX11_Renderer__send_buffer_ortho(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(offset);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, n);
    CHECK_NUMBER(float, f);
    mat4 matrix;
    glm_mat4_identity(matrix);
    glm_ortho(left, right, bottom, top, n, f, matrix);
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT result = self->dx11.context->Map(
        buffer->dx11.handle,
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &mapped
    );
    char* data = (char*)mapped.pData + offset;
    memcpy(data, matrix, sizeof(mat4));
    self->dx11.context->Unmap(buffer->dx11.handle, 0);
    return 0;
}

/**
 * Textures
 */
#if 0
int l_DX11_Renderer__create_font(lua_State* L) {
    CHECK_META(selene_Renderer);
    int w = 2048;
    int h = 8;
    uint8_t* bitmap = (uint8_t*)malloc(w * h * 4);
    int ox = 0;
    int stride = w * sizeof(int);
    for (int i = 0; i < 128; i++) {
        for (int y = 0; y < 8; y++) {
            uint8_t l = font8x8_basic[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    ox = 128 * 8 * sizeof(int);
    for (int i = 0; i < 32; i++) {
        for (int y = 0; y < 8; y++) {
            uint8_t l = font8x8_control[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    ox = 160 * 8 * sizeof(int);
    for (int i = 0; i < 96; i++) {
        for (int y = 0; y < 8; y++) {
            uint8_t l = font8x8_ext_latin[i][y];
            int offset = ox + (i * 8 * sizeof(int)) + (y * stride);
            for (int x = 0; x < 8; x++) {
                char p = (l >> x) & 0x1;

                uint8_t* pixel = bitmap + offset;
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
                pixel[3] = 255 * p;
                offset += 4;
            }
        }
    }
    NEW_UDATA(selene_Font, font);
    font->texture.width = w;
    font->texture.height = h;
    size_t size = w * h * 4;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = font->texture.width;
    desc.Height = font->texture.height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ID3D11SamplerState* sampler_state;
    self->dx11.device->CreateSamplerState(&samplerDesc, &sampler_state);

    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = bitmap;
    subResource.SysMemPitch = bitmap ? font->texture.width * 4 : 0;
    subResource.SysMemSlicePitch = 0;

    // Create texture
    ID3D11Texture2D* handle = nullptr;
    HRESULT hr = self->dx11.device->CreateTexture2D(&desc, bitmap ? &subResource : nullptr, &handle);
    if (FAILED(hr)) {
        return luaL_error(L, "Failed to create texture: 0x%08X", hr);
    }

    // Create shader resource view
    ID3D11ShaderResourceView* srv = nullptr;
    hr = self->dx11.device->CreateShaderResourceView(handle, nullptr, &srv);
    if (FAILED(hr)) {
        handle->Release();
        return luaL_error(L, "Failed to create shader resource view: 0x%08X", hr);
    }
    free(bitmap);

    font->texture.dx11.tex2d = handle;
    font->texture.dx11.srv = srv;
    font->texture.dx11.sampler = sampler_state;

    selene_FontGlyph* glyphs = font->glyphs;
    for (int i = 0; i < 256; i++) {
        glyphs[i].ax = 8 / w;
        glyphs[i].ay = 0;
        glyphs[i].bl = 0;
        glyphs[i].bt = 0;
        glyphs[i].bw = 8;
        glyphs[i].bh = 8;
        glyphs[i].tx = 8 * i;
    }
    return 1;
}
#endif
int l_DX11_Renderer__create_texture2d(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int opt = luaL_checkoption(L, arg++, "rgba", pixel_formats);
    void* data = NULL;
    if (lua_isuserdata(L, arg)) data = lua_touserdata(L, arg++);
    int target = 0;
    if (lua_isboolean(L, arg)) target = lua_toboolean(L, arg++);
    DXGI_FORMAT pixel_format = dx11_pixel_formats_values[opt];

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = pixel_format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    UINT flags = D3D11_BIND_SHADER_RESOURCE;
    if (target) flags |= D3D11_BIND_RENDER_TARGET;
    desc.BindFlags = flags;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ID3D11SamplerState* sampler_state;
    self->dx11.device->CreateSamplerState(&samplerDesc, &sampler_state);

    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = data;
    subResource.SysMemPitch = data ? width * 4 : 0;
    subResource.SysMemSlicePitch = 0;

    // Create texture
    ID3D11Texture2D* handle = nullptr;
    HRESULT hr = self->dx11.device->CreateTexture2D(&desc, data ? &subResource : nullptr, &handle);
    if (FAILED(hr)) {
        return luaL_error(L, "Failed to create texture: 0x%08X", hr);
    }

    // Create shader resource view
    ID3D11ShaderResourceView* srv = nullptr;
    hr = self->dx11.device->CreateShaderResourceView(handle, nullptr, &srv);
    if (FAILED(hr)) {
        handle->Release();
        return luaL_error(L, "Failed to create shader resource view: 0x%08X", hr);
    }

    NEW_UDATA(selene_Texture2D, tex);
    tex->dx11.tex2d = handle;
    tex->dx11.srv = srv;
    tex->dx11.sampler = sampler_state;
    tex->width = width;
    tex->height = height;
    tex->format = opt;
    return 1;
}

int l_DX11_Renderer__create_depth_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;                    // Will be set to window size
    depthDesc.Height = height;                   // Will be set to window size
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    ID3D11Texture2D* handle = NULL;
    ID3D11DepthStencilView* dsv = NULL;

    HRESULT result = self->dx11.device->CreateTexture2D(&depthDesc, nullptr, &handle);
    if (FAILED(result)) {
        char buffer[256];
        sprintf(buffer, "failed to create Depth Texture: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

    result = self->dx11.device->CreateDepthStencilView(handle, nullptr, &dsv);
    if (FAILED(result)) {
        handle->Release();
        char buffer[256];
        sprintf(buffer, "failed to create DepthStencilView: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }
    NEW_UDATA(selene_Texture2D, tex);
    tex->dx11.tex2d = handle;
    tex->dx11.srv = NULL;
    tex->dx11.sampler = NULL;
    tex->width = width;
    tex->height = height;
    tex->format = 2;
    return 1;
}

int l_DX11_Renderer__destroy_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Texture2D, tex);
    if (tex->dx11.srv) tex->dx11.srv->Release();
    if (tex->dx11.sampler) tex->dx11.sampler->Release();
    if (tex->dx11.tex2d) tex->dx11.tex2d->Release();
    return 0;
}

/**
 * Render Targets
 */

int l_DX11_Renderer__create_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Texture2D, color);
    TEST_UDATA(selene_Texture2D, depth);

#if 0
    D3D11_TEXTURE2D_DESC rtDesc = {};
    rtDesc.Width = width;
    rtDesc.Height = height;
    rtDesc.MipLevels = 1;
    rtDesc.ArraySize = 1;
    rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtDesc.SampleDesc.Count = 1;
    rtDesc.SampleDesc.Quality = 0;
    rtDesc.Usage = D3D11_USAGE_DEFAULT;
    rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    rtDesc.CPUAccessFlags = 0;
    rtDesc.MiscFlags = 0;
    ID3D11Texture2D* renderTarget = nullptr;
    HRESULT hr = self->dx11.device->CreateTexture2D(&rtDesc, nullptr, &renderTarget);
    if (FAILED(hr)) {
        return luaL_error(L, "Failed to create render target: 0x%08X", hr);
    }
#endif

    ID3D11RenderTargetView* rtv = nullptr;
    HRESULT hr = self->dx11.device->CreateRenderTargetView(color->dx11.tex2d, nullptr, &rtv);
    if (FAILED(hr)) {
        return luaL_error(L, "Failed to create render target view: 0x%08X", hr);
    }

    // Create shader resource view
    ID3D11ShaderResourceView* srv = nullptr;
    hr = self->dx11.device->CreateShaderResourceView(color->dx11.tex2d, nullptr, &srv);
    if (FAILED(hr)) {
        rtv->Release();
        return luaL_error(L, "Failed to create shader resource view: 0x%08X", hr);
    }

    ID3D11DepthStencilView* dsv = NULL;
    if (depth) {
        hr = self->dx11.device->CreateDepthStencilView(depth->dx11.tex2d, nullptr, &dsv);
        if (FAILED(hr)) {
            srv->Release();
            rtv->Release();
            char buffer[256];
            sprintf(buffer, "failed to create DepthStencilView: 0x%.8x", hr);
            lua_pushstring(L, buffer);
            lua_error(L);
            return 0;
        }
    }

    NEW_UDATA(selene_RenderTarget, target);
    // target->depth = depth ? 1 : 0;
    target->dx11.rtv = rtv;
    target->color = color;
    target->dx11.dsv = dsv;
    target->depth = depth;
    target->dx11.srv = srv;
    return 1;
}

int l_DX11_Renderer__destroy_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderTarget, target);
    if (target->dx11.srv) target->dx11.srv->Release();
    if (target->dx11.rtv) target->dx11.rtv->Release();
    if (target->dx11.dsv) target->dx11.dsv->Release();
    return 0;
}
#if 0
int l_DX11_Renderer__set_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    TEST_UDATA(selene_RenderTarget, target);
    int depth = 0;
    if (lua_isboolean(L, arg)) {
        depth = lua_toboolean(L, arg++);
    }
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_RENDER_TARGET;
    cmd.target.depth = depth;
    cmd.target.ptr = target;
    dx11_push_command(self, &cmd);
    return 0;
}
#endif
/**
 * Shaders
 */
int l_DX11_Renderer__create_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    ID3D11Device* device = self->dx11.device;
    int opt = luaL_checkoption(L, arg++, "vertex", shader_type_options);
    const char* source = luaL_checkstring(L, arg++);
    ID3DBlob* shaderBlob = NULL;
    ID3DBlob* errorBlob = NULL;
    LPCSTR target;
    switch (opt) {
        case 0: // vertex
            target = "vs_5_0";
            break;
        case 1: // fragment (pixel in DirectX)
            target = "ps_5_0";
            break;
            // Add other shader types as needed
        default:
            return luaL_error(L, "unsupported shader type");
    }

    HRESULT hr = D3DCompile(
            source,             // Source code
            strlen(source),     // Source code length
            NULL,               // Source name
            NULL,               // Defines
            NULL,               // Includes
            "main",            // Entry point
            target,            // Target
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // Flags
            0,                  // Effect flags
            &shaderBlob,        // Output blob
            &errorBlob         // Error blob
    );

    if (FAILED(hr)) {
        const char* errorMsg = errorBlob ? (const char*)errorBlob->GetBufferPointer() : "Unknown error";
        if (errorBlob) errorBlob->Release();
        return luaL_error(L, "failed to compile %s shader: %s", shader_type_options[opt], errorMsg);
    }

    void* handle = NULL;
    switch (opt) {
        case 0: // vertex
            hr = device->CreateVertexShader(
                    shaderBlob->GetBufferPointer(),
                    shaderBlob->GetBufferSize(),
                    NULL,
                    (ID3D11VertexShader**)&handle
            );
            break;
        case 1: // pixel
            hr = device->CreatePixelShader(
                    shaderBlob->GetBufferPointer(),
                    shaderBlob->GetBufferSize(),
                    NULL,
                    (ID3D11PixelShader**)&handle
            );
            break;
            // Add other shader types as needed
    }

    if (FAILED(hr)) {
        shaderBlob->Release();
        if (errorBlob) errorBlob->Release();
        return luaL_error(L, "failed to create %s shader", shader_type_options[opt]);
    }

    NEW_UDATA(selene_Shader, shader);
    shader->type = opt;
    shader->dx11.vertex = (ID3D11VertexShader*)handle;
    shader->dx11.blob = shaderBlob;
    if (errorBlob) errorBlob->Release();
    return 1;
}

int l_DX11_Renderer__destroy_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Shader, shd);

    if (shd->dx11.vertex) {
        switch (shd->type) {
            case 0: // vertex
                ((ID3D11VertexShader*)shd->dx11.vertex)->Release();
                break;
            case 1: // pixel
                ((ID3D11PixelShader*)shd->dx11.pixel)->Release();
                break;
        }
        shd->dx11.vertex = NULL;
    }

    if (shd->dx11.blob) {
        shd->dx11.blob->Release();
        shd->dx11.blob = NULL;
    }

    return 0;
}

/**
 * Draw and flush
 */

int l_DX11_Renderer__flush(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct {
        selene_RenderPipeline* pipe;
        selene_GpuBuffer* vertex;
        selene_GpuBuffer* index;
        selene_GpuBuffer* uniform;
        selene_Texture2D* texture;
        selene_RenderTarget* target;
        UINT stride;
    } state;
    memset(&state, 0, sizeof(state));
    state.stride = sizeof(Vertex2D);
    state.target = &self->default_target;
    for (int i = 0; i < self->command_offset; i++) {
        struct RenderCommand* rc = self->command_pool + i;
        switch (rc->type) {
            case RENDER_COMMAND_CLEAR_COLOR: {
                float* c = rc->clear.color;
                // fprintf(stdout, "Clear Color command\n");
                ID3D11RenderTargetView* rtv = state.target->dx11.rtv;
                self->dx11.context->ClearRenderTargetView(rtv, c);
            }
                break;
            case RENDER_COMMAND_CLEAR_DEPTH: {
                ID3D11DepthStencilView* dsv = state.target->dx11.dsv;
                float depth = rc->clear.depth;
                if (dsv) self->dx11.context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, rc->clear.depth, 0);
            }
                break;
            case RENDER_COMMAND_SET_VIEWPORT: {
                // fprintf(stdout, "Set Viewport command\n");
                D3D11_VIEWPORT viewPort = {0};
                viewPort.TopLeftX = (float) rc->viewport.x;
                viewPort.TopLeftY = (float) rc->viewport.y;
                viewPort.Width = (float) rc->viewport.width;
                viewPort.Height = (float) rc->viewport.height;
                viewPort.MinDepth = 0.0f;
                viewPort.MaxDepth = 1.0f;
                self->dx11.context->RSSetViewports(1, &viewPort);
            }
                break;
            case RENDER_COMMAND_SET_PIPELINE: {
                selene_RenderPipeline* rp = rc->pipeline;
                // fprintf(stdout, "Set Pipeline command\n");
                ID3D11DeviceContext* context = self->dx11.context;
                if (rp == state.pipe) break;
                if (rp == NULL) {
                    // Unbind shaders
                    context->VSSetShader(NULL, NULL, 0);
                    context->PSSetShader(NULL, NULL, 0);

                    // Disable all pipeline states
                    context->OMSetBlendState(NULL, NULL, 0xFFFFFFFF); // Default blend
                    context->OMSetDepthStencilState(NULL, 0); // Default depth
                    context->RSSetState(NULL); // Default rasterizer
                    context->IASetInputLayout(NULL);

                    state.pipe = rp;
                    break;
                }
                state.stride = rp->layout.vertex_stride;
                selene_RenderPipeline pipe;
                memset(&pipe, 0, sizeof(pipe));
                if (!state.pipe) state.pipe = &pipe;

                if (state.pipe->dx11.input_layout != rp->dx11.input_layout) {
                    context->IASetInputLayout(rp->dx11.input_layout);
                }

                if (state.pipe->dx11.vs != rp->dx11.vs) {
                    context->VSSetShader(rp->dx11.vs, NULL, 0);
                }
                if (state.pipe->dx11.ps != rp->dx11.ps) {
                    context->PSSetShader(rp->dx11.ps, NULL, 0);
                }

                if (state.pipe->dx11.blend_state != rp->dx11.blend_state) {
                    float blendFactor[4] = {1,1,1,1}; // Default blend factor
                    context->OMSetBlendState(
                            rp->dx11.blend_state,
                            blendFactor,
                            0xFFFFFFFF // Sample mask
                    );
                    //fprintf(stdout, "testando\n");
                }

                if (state.pipe->dx11.depth_stencil_state != rp->dx11.depth_stencil_state) {
                    context->OMSetDepthStencilState(
                            rp->dx11.depth_stencil_state,
                            0 // Stencil reference
                    );
                }

                if (state.pipe->dx11.rasterizer_state != rp->dx11.rasterizer_state) {
                    context->RSSetState(rp->dx11.rasterizer_state);
                }

                state.pipe = rp;
            }
                break;
            case RENDER_COMMAND_SET_VERTEX_BUFFER: {
                selene_GpuBuffer* b = rc->buffer.ptr;
                selene_RenderPipeline* rp = state.pipe;
                UINT offset = 0;
                UINT stride = state.stride;
                self->dx11.context->IASetVertexBuffers(
                        0,                  // Start slot
                        1,                  // Number of buffers
                        &b->dx11.handle,    // Pointer to buffer
                        &stride,
                        &offset
                );
                
                state.vertex = b;
            }
                break;
            case RENDER_COMMAND_SET_INDEX_BUFFER: {
                selene_GpuBuffer* b = rc->buffer.ptr;
                self->dx11.context->IASetIndexBuffer(b ? b->dx11.handle : NULL, DXGI_FORMAT_R32_UINT, 0);
                state.index = b;
            }
                break;
            case RENDER_COMMAND_SET_UNIFORM_BUFFER: {
                selene_GpuBuffer* b = rc->buffer.ptr;
                self->dx11.context->VSSetConstantBuffers(0, 1, b ? &(b->dx11.handle) : NULL);
                state.uniform = b;
            }
                break;
            case RENDER_COMMAND_SET_TEXTURE: {
                selene_Texture2D* t = (selene_Texture2D*)rc->texture.ptr;
                if (t) {
                    self->dx11.context->PSSetShaderResources(0, 1, &t->dx11.srv);
                    ID3D11SamplerState* sampler = t->dx11.sampler;
                    self->dx11.context->PSSetSamplers(0, 1, &sampler);
                }
                state.texture = t;
            }
                break;
            case RENDER_COMMAND_SET_SHADER:
                //glUseProgram(rc->program.handle);
                break;
            case RENDER_COMMAND_SET_RENDER_TARGET: {
                selene_RenderTarget* t = rc->target.ptr ? rc->target.ptr : &(self->default_target);
                if (rc->target.depth) self->dx11.context->OMSetRenderTargets(1, &(t->dx11.rtv), t->dx11.dsv);
                else self->dx11.context->OMSetRenderTargets(1, &(t->dx11.rtv), NULL);
                state.target = t;
            }
                break;
            case RENDER_COMMAND_SET_SCISSOR: {
                D3D11_RECT scissorRect;
                scissorRect.left = rc->scissor.x;    // Left coordinate (in pixels)
                scissorRect.top = rc->scissor.y;     // Top coordinate (in pixels)
                scissorRect.right = rc->scissor.width;   // Right coordinate (in pixels)
                scissorRect.bottom = rc->scissor.height;  // Bottom coordinate (in pixels)

                // Set the scissor rectangle
                self->dx11.context->RSSetScissorRects(1, &scissorRect);
            }
                break;
#if 0
                case RENDER_COMMAND_SET_BLEND_MODE:
                glBlendFunc(rc->blend.func0, rc->blend.func1);
                glBlendEquation(rc->blend.equation);
                break;
#endif
            case RENDER_COMMAND_FLOAT_UNIFORM:
                // glUniform1f(rc->uniform.location, rc->uniform.f);
                break;
            case RENDER_COMMAND_SET_PROJECTION:
            case RENDER_COMMAND_SET_VIEW:
            case RENDER_COMMAND_MATRIX_UNIFORM:
                //glUniformMatrix4fv(rc->uniform.location, 1, GL_FALSE, rc->uniform.m[0]);
                break;
            case RENDER_COMMAND_DRAW_VERTEX:
                // fprintf(stdout, "Draw Command\n");
                // fprintf(stdout, "draw_vertex: %d %d %d\n", rc->draw.mode, rc->draw.start, rc->draw.count);
                self->dx11.context->IASetPrimitiveTopology(dx11_primitive_topologies[rc->draw.mode]);
                self->dx11.context->Draw(rc->draw.count, rc->draw.start);
                break;
            case RENDER_COMMAND_DRAW_INDEX:
                //glDrawElements(rc->draw.mode, rc->draw.count, GL_UNSIGNED_INT, NULL);
                break;
            case RENDER_COMMAND_DRAW_VERTEX_INSTANCED:
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                //glDrawArraysInstanced(rc->instanced.mode, rc->instanced.start, rc->instanced.count, rc->instanced.instance_count);
#endif
                break;
            case RENDER_COMMAND_DRAW_INDEX_INSTANCED:
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                //glDrawElementsInstanced(rc->instanced.mode, rc->instanced.count, GL_UNSIGNED_INT, NULL, rc->instanced.instance_count);
#endif
                break;
            default:
                break;
        }
    }
    self->command_offset = 0;
    self->dx11.context->VSSetShader(NULL, NULL, 0);
    self->dx11.context->PSSetShader(NULL, NULL, 0);

    // Disable all pipeline states
    self->dx11.context->OMSetBlendState(NULL, NULL, 0xFFFFFFFF); // Default blend
    self->dx11.context->OMSetDepthStencilState(NULL, 0); // Default depth
    self->dx11.context->RSSetState(NULL); // Default rasterizer
    return 0;
}

int l_DX11_Renderer__present(lua_State* L) {
    CHECK_META(selene_Renderer);
    l_DX11_Renderer__flush(L);
    //SDL_GL_SwapWindow(self->window_ptr);
    self->dx11.swap_chain->Present(0, 0);
    return 0;
}

extern "C" int l_DX11_Renderer_create(lua_State * L) {
    INIT_ARG();
    selene_Window* win = (selene_Window*)luaL_checkudata(L, arg++, selene_Window_METANAME);
    int width, height;
    SDL_GetWindowSize(win->handle, &width, &height);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(win->handle, &wmInfo)) {
        return luaL_error(L, "Cannot get WM Info from SDL_Window: %s", SDL_GetError());
    }
    HWND hwnd = wmInfo.info.win.window;
    if (!hwnd) {
        return luaL_error(L, "Failed to get HWND from SDL_Window");
    }

    ID3D11Device* pD3DDevice = NULL;
    D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_0
    };
    ID3D11DeviceContext* pD3DContext = NULL;
    IDXGISwapChain1* swap_chain = nullptr;
#if 1
    HRESULT result = D3D11CreateDevice(
        NULL,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,// Driver type
        NULL,                    // Software rasterizer
        D3D11_CREATE_DEVICE_DEBUG, // Flags
        featureLevels,          // Feature levels
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,      // SDK version
        &pD3DDevice,            // Device pointer
        NULL,                   // Feature level pointer
        &pD3DContext                    // Context pointer
    );
    if (FAILED(result)) {
        if (swap_chain) swap_chain->Release();
        char buffer[256];
        sprintf(buffer, "failed to create DX11 Device: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

    IDXGIFactory2* dxgiFactory;
    result = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(result)) {
        char buffer[256];
        sprintf(buffer, "failed to create DX11 Device: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width = width;                    // Use window size
    sd.Height = height;                   // Use window size
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;         // No multisampling
    sd.SampleDesc.Quality = 0;
    sd.BufferCount = 2;              // Double buffering
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    // sd.OutputWindow = hwnd;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY;

    // Create swap chain
    result = dxgiFactory->CreateSwapChainForHwnd(
        pD3DDevice,
        hwnd,
        &sd,
        nullptr,
        nullptr,
        &swap_chain
    );
    dxgiFactory->Release();
    if (FAILED(result)) {
        char buffer[256];
        sprintf(buffer, "failed to create DX11 SwapChain: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

    ID3D11Texture2D* backBuffer;
    result = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        (LPVOID*)&backBuffer);
    if (FAILED(result)) {
        swap_chain->Release();
        pD3DContext->Release();
        pD3DDevice->Release();
        char buffer[256];
        sprintf(buffer, "failed to get back buffer texture: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }
    ID3D11RenderTargetView* rtv;
    result = pD3DDevice->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();
    if (FAILED(result)) {
        swap_chain->Release();
        pD3DContext->Release();
        pD3DDevice->Release();
        char buffer[256];
        sprintf(buffer, "failed to create Render Target: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;                    // Will be set to window size
    depthDesc.Height = height;                   // Will be set to window size
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    ID3D11Texture2D* depthStencilBuffer;
    result = pD3DDevice->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer);
    if (FAILED(result)) {
        rtv->Release();
        swap_chain->Release();
        pD3DContext->Release();
        pD3DDevice->Release();
        char buffer[256];
        sprintf(buffer, "failed to create Render Target: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

    ID3D11DepthStencilView* dsv;
    result = pD3DDevice->CreateDepthStencilView(depthStencilBuffer, nullptr, &dsv);
    if (FAILED(result)) {
        depthStencilBuffer->Release();
        rtv->Release();
        swap_chain->Release();
        pD3DContext->Release();
        pD3DDevice->Release();
        dxgiFactory->Release();
        char buffer[256];
        sprintf(buffer, "failed to create Render Target: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

#else
    // fprintf(stdout, "%d %d %d\n", sd.BufferDesc.Width, sd.BufferDesc.Height, sd.Flags);
    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    memset(&sd, 0, sizeof(sd));
    sd.BufferDesc.Width = width;                    // Use window size
    sd.BufferDesc.Height = height;                   // Use window size
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;                    // No multisampling
    sd.SampleDesc.Quality = 0;
    sd.BufferCount = 2;                         // Double buffering
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY;
    HRESULT result = D3D11CreateDeviceAndSwapChain(
        NULL,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,// Driver type
        NULL,                    // Software rasterizer
        D3D11_CREATE_DEVICE_DEBUG, // Flags
        featureLevels,          // Feature levels
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,      // SDK version
        &sd,
        &swap_chain,
        &pD3DDevice,            // Device pointer
        NULL,                   // Feature level pointer
        &pD3DContext                    // Context pointer
    );

    if (FAILED(result)) {
        if (swap_chain) swap_chain->Release();
        char buffer[256];
        //DXGetErrorStringA(hr, buffer, sizeof(buffer));
        sprintf(buffer, "failed to create DX11 Device and SwapChain: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }
#endif

    ID3D11Buffer* const_buffer = NULL;
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(mat4);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    result = pD3DDevice->CreateBuffer(&bufferDesc, NULL, &const_buffer);
    if (FAILED(result)) {
        if (swap_chain) swap_chain->Release();
        char buffer[256];
        //DXGetErrorStringA(hr, buffer, sizeof(buffer));
        sprintf(buffer, "failed to create Constant Buffer: 0x%.8x", result);
        lua_pushstring(L, buffer);
        lua_error(L);
        return 0;
    }

    NEW_UDATA(selene_Renderer, self);
    self->dx11.device = pD3DDevice;
    self->dx11.context = pD3DContext;
    self->dx11.swap_chain = swap_chain;

    self->default_target.dx11.rtv = rtv;
    self->default_target.dx11.dsv = dsv;
    self->default_target.depth = (selene_Texture2D*)malloc(sizeof(selene_Texture2D));
    self->default_target.depth->dx11.tex2d = depthStencilBuffer;

    self->command_offset = 0;
    self->command_count = 512;
    self->command_pool = (struct RenderCommand*)malloc(sizeof(struct RenderCommand) * self->command_count);
    lua_pushvalue(L, 1);
    self->l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    self->backend = SELENE_RENDERER_DIRECTX11;
    fprintf(stdout, "create DirectX11 Renderer\n");

    self->destroy = l_DX11_Renderer__destroy;

    self->create_buffer = l_DX11_Renderer__create_buffer;
    self->destroy_buffer = l_DX11_Renderer__destroy_buffer;
    self->send_buffer_data = l_DX11_Renderer__send_buffer_data;

    self->create_pipeline = l_DX11_Renderer__create_pipeline;
    self->destroy_pipeline = l_DX11_Renderer__destroy_pipeline;

    self->create_render_target = l_DX11_Renderer__create_render_target;
    self->destroy_render_target = l_DX11_Renderer__destroy_render_target;

    self->create_shader = l_DX11_Renderer__create_shader;
    self->destroy_shader = l_DX11_Renderer__destroy_shader;

    self->create_texture2d = l_DX11_Renderer__create_texture2d;
    self->create_depth_texture = l_DX11_Renderer__create_depth_texture;
    self->destroy_texture = l_DX11_Renderer__destroy_texture;

    self->flush = l_DX11_Renderer__flush;
    self->present = l_DX11_Renderer__present;

    return 1;
}

#if 0
extern "C" int l_DX11_Renderer_meta(lua_State* L) {
    lua_newtable(L);
    const luaL_Reg _reg[] = {
            REG_META_FIELD(DX11_Renderer, clear_color),
            REG_META_FIELD(DX11_Renderer, clear_depth),
            // Buffers
            REG_META_FIELD(DX11_Renderer, create_buffer),
            REG_META_FIELD(DX11_Renderer, destroy_buffer),
            REG_META_FIELD(DX11_Renderer, set_vertex_buffer),
            REG_META_FIELD(DX11_Renderer, set_index_buffer),
            REG_META_FIELD(DX11_Renderer, set_uniform_buffer),
            REG_META_FIELD(DX11_Renderer, send_buffer_data),
            REG_META_FIELD(DX11_Renderer, send_buffer_ortho),
#if 1
            // Shaders
            REG_META_FIELD(DX11_Renderer, create_shader),
            REG_META_FIELD(DX11_Renderer, destroy_shader),
            // Textures
            REG_META_FIELD(DX11_Renderer, create_font),
            REG_META_FIELD(DX11_Renderer, create_texture2d),
            REG_META_FIELD(DX11_Renderer, create_depth_texture),
            REG_META_FIELD(DX11_Renderer, destroy_texture),
            REG_META_FIELD(DX11_Renderer, set_texture),
            // Render Targets
            REG_META_FIELD(DX11_Renderer, create_render_target),
            REG_META_FIELD(DX11_Renderer, destroy_render_target),
            REG_META_FIELD(DX11_Renderer, set_render_target),
            // Pipeline
            REG_META_FIELD(DX11_Renderer, create_pipeline),
            REG_META_FIELD(DX11_Renderer, destroy_pipeline),
            REG_META_FIELD(DX11_Renderer, set_pipeline),
            // Rasterizer states
            // REG_META_FIELD(DX11_Renderer, set_scissor),
            REG_META_FIELD(DX11_Renderer, set_viewport),
#endif
            // Draw and Flush
            REG_META_FIELD(DX11_Renderer, draw),
            REG_META_FIELD(DX11_Renderer, flush),
            REG_META_FIELD(DX11_Renderer, present),
            // Destroy
            REG_META_FIELD(DX11_Renderer, destroy),
            {NULL, NULL}
    };
    luaL_setfuncs(L, _reg, 0);
    return 1;
}
#endif
#endif