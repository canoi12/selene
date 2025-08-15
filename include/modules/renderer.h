#ifndef SELENE_RENDERER_H_
#define SELENE_RENDERER_H_

#include "selene.h"

#ifndef SELENE_NO_VULKAN
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
    #include <d3d11.h>
#endif
#ifndef SELENE_NO_DX12
    #include <d3d12.h>
#endif
#if !defined(SELENE_NO_DX11) && !defined(SELENE_NO_DX12)
    #include <dxgi.h>
    #include <dxgi1_2.h>
    #include <dxgi1_3.h>
    #include <d3dcompiler.h>
#endif
#endif

#ifndef selene_Renderer_METANAME
    #define selene_Renderer_METANAME "selene.Renderer"
#endif

#if defined(__cplusplus)
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN const char* pixel_formats[];
EXTERN const int pixel_formats_values[];

EXTERN const char* texture_filters[];
EXTERN const int texture_filters_values[];

EXTERN const char* draw_modes[];
EXTERN const int draw_modes_values[];

EXTERN const char* shader_type_options[];

EXTERN const char* buffer_target_options[];

EXTERN const char* texture_target_options[];
EXTERN const char* front_face_options[];
EXTERN const char* cull_face_options[];

EXTERN const char* polygon_mode_options[];

enum {
    SELENE_BYTE = 0,
    SELENE_UBYTE,
    SELENE_SHORT,
    SELENE_USHORT,
    SELENE_INT,
    SELENE_UINT,
    SELENE_FLOAT,
    SELENE_DOUBLE
};
EXTERN const char* type_name_options[];
EXTERN const char* comparison_func_options[];
EXTERN const char* vertex_format_options[];

EXTERN const char* renderer_backend_options[];

#undef EXTERN


/**
 * Buffer
 **/

typedef struct {
    int stride;
    struct {
        int location;
        int stride;
        int type;
    } attrib[16];
} _VertexFormat;

typedef struct _Buffer selene_Buffer;
struct _Buffer {
    int type, usage;
    uint32_t stride, size;
    union {
#ifndef SELENE_NO_OPENGL
        struct { int type; GLuint handle; } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct { VkBuffer handle; VkDeviceMemory mem; } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct {
            enum DX11BufferType type;
            ID3D11Buffer* handle;
            D3D11_USAGE usage;
        } dx11;
#endif
#endif
    };
};

/**
 * Texture
 **/


typedef struct _Texture selene_Texture;
struct _Texture {
    union {
#ifndef SELENE_NO_OPENGL
        struct {
            GLuint handle;
        } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkImage handle;
            VkDeviceMemory mem;
            VkImageView view;
            VkSampler sampler;
        } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct {
            union {
                ID3D11Texture2D* tex2d;
                ID3D11Texture1D* tex1d;
            };
            ID3D11ShaderResourceView* srv;
            ID3D11SamplerState* sampler;
        } dx11;
#endif
#endif
    };
    int width, height;
    int z;
    int format;
};

/**
 * Render Target
 **/
typedef struct _RenderTarget selene_RenderTarget;
struct _RenderTarget {
    union {
#ifndef SELENE_NO_OPENGL
        struct { GLuint fbo; } gl;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
        struct {
            ID3D11RenderTargetView* rtv;
            ID3D11DepthStencilView* dsv;
            ID3D11ShaderResourceView* srv;
        } dx11;
#endif
#endif
    };
};

/**
 * Shader
 **/

typedef struct _Shader selene_Shader;
struct _Shader {
    int type;
    union {
#ifndef SELENE_NO_OPENGL
        struct { GLuint handle; } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct { VkShaderModule handle; } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct {
            union {
                ID3D11VertexShader* vertex;
                ID3D11PixelShader* pixel;
            };
            ID3DBlob* blob;
        } dx11;
#endif
#ifndef SELENE_NO_D3D12
            struct {} dx12;
#endif
#endif
    };
};

/**
 * Render Pipeline
 **/
typedef struct _RenderPipeline selene_RenderPipeline;
struct _RenderPipeline {
};

/**
 * Renderer
 **/
typedef struct _Renderer selene_Renderer;
struct _Renderer {
};

#if defined(__cplusplus)
extern "C" {
#endif

SELENE_API int luaopen_renderer(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_RENDERER_H_ */
