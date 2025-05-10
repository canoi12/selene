#if defined(__cplusplus)
extern "C" {
#endif
#include "selene.h"
#include "lua_helper.h"
#if defined(__cplusplus)
}
#endif

#ifndef SELENE_RENDERER_H_
#define SELENE_RENDERER_H_

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

#ifndef SELENE_BATCH2D_PROJECTION_FAR
#define SELENE_BATCH2D_PROJECTION_FAR 1000
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
 * Vertex Types
 */

typedef struct {
    float x, y, z;
    float r, g, b, a;
    float u, v;
} Vertex2D;

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
} Vertex3D;

/**
 * Buffer Types
 */

typedef struct {
    int stride;
    struct {
        int location;
        int stride;
        int type;
    } attrib[16];
} VertexFormat;

enum {
    SELENE_GPU_BUFFER_NO_TARGET = -1,
    SELENE_GPU_BUFFER_VERTEX,
    SELENE_GPU_BUFFER_INDEX,
    SELENE_GPU_BUFFER_UNIFORM
};

#ifndef SELENE_NO_GL
struct GLBuffer {
    int type;
    GLuint handle;
    GLboolean usage;
};
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
enum DX11BufferType {
    DX11_VERTEX_BUFFER,
    DX11_INDEX_BUFFER,
    DX11_CONSTANT_BUFFER,
    DX11_STAGING_BUFFER
};

struct DX11Buffer {
    enum DX11BufferType type;
    ID3D11Buffer* handle;
    D3D11_USAGE usage;
};
#endif
#endif

typedef struct {
    Uint32 type;
    Uint32 stride, size;
    union {
#ifndef SELENE_NO_GL
        struct GLBuffer gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkBuffer handle;
            VkDeviceMemory mem;
        } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
        struct DX11Buffer dx11;
#endif
#endif
    };
} GpuBuffer;

typedef struct {
    int type, usage;
    int stride, size;
    union {
#ifndef SELENE_NO_GL
        struct { GLuint handle; } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct { VkBuffer handle; } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
        struct DX11Buffer dx11;
#endif
#endif
    };
} selene_GpuBuffer;

/**
 * Texture Types
 */

#ifndef SELENE_NO_GL
struct GLTexture {
    GLuint handle;
};
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
struct DX11Texture {
    union {
        ID3D11Texture2D* tex2d;
        ID3D11Texture1D* tex1d;
    };
    ID3D11ShaderResourceView* srv;
    ID3D11SamplerState* sampler;
};
#endif
#endif

typedef struct {
    union {
#ifndef SELENE_NO_GL
        struct GLTexture gl;
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
#ifndef SELENE_NO_DX11
        struct DX11Texture dx11;
#endif
#endif
    };
    int width, height;
    int format;
} Texture2D;

typedef struct {
    int type;
    int width, height;
    int depth;
    int mip_level;
    int pixel_format;
    union {
#ifndef SELENE_NO_GL
        struct { GLuint handle; } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkImage handle;
            VkDeviceMemory mem;
            VkImageView view;
            VkSampler sampler;
        } vk;
#endif
#ifdef OS_WIN
#ifndef SELENE_NO_DX11
        struct {
            void* handle;
            ID3D11ShaderResourceView* srv;
            ID3D11SamplerState* sampler;
        } dx11;
#endif
#endif
    };
} selene_Texture;

typedef struct {
    Texture2D texture;
    FontGlyph glyphs[256];
} Font;

/**
 * Render Target
 */

typedef struct {
    union {
#ifndef SELENE_NO_GL
        struct {
            GLuint fbo;
        } gl;
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
    Texture2D* color;
    Texture2D* depth;
    int width, height;
} selene_RenderTarget;


/**
 * Shader
 */

#ifndef SELENE_NO_GL
struct GLShader {
    Uint32 handle;
};
#endif

#ifndef SELENE_NO_VULKAN
struct VKShader {
    VkShaderModule handle;
};
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
struct DX11Shader {
    union {
        ID3D11VertexShader* vertex;
        ID3D11PixelShader* pixel;
    };
    ID3DBlob* blob;
};
#endif
#ifndef SELENE_NO_DX12
struct DX12Shader {
    ID3DBlob* blob;
};
#endif
#endif

typedef struct selene_Shader selene_Shader;
struct selene_Shader {
    Uint32 type;
    union {
#ifndef SELENE_NO_GL
        struct { GLuint handle; } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct { VkShaderModule handle; } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
        struct DX11Shader dx11;
#endif
#ifndef SELENE_NO_DX12
        struct DX12Shader dx12;
#endif
#endif
    };
};

/**
 * Material
 */

typedef struct {
    vec3 color;
    float roughness;
    float metallic;
} Material;

/**
 * Mesh
 */

typedef struct {
    Vertex3D* vertices;
    int vertex_count;
    Uint32* indices;
    int index_count;
} Mesh;

/**
 * Light
 */

typedef struct {
    float position[3];
    float color[3];
    float intensity;
    float range;
} Light;

/**
 * RenderPipeline
 */

struct selene_InputLayout {
    int count;
    size_t vertex_stride;
    void* handle;
    struct {
        char name[64];
        int type;
        int size;
        int offset;
        int binding;
        int stride;
    } attributes[8];
};

#ifndef SELENE_NO_GL
struct GLBlendState {
    GLboolean enabled;
    GLenum func0, func1;
    GLenum equation;
};

struct GLDepthStencilState {
    GLboolean depth_enabled;
    GLenum depth_func;
    GLboolean stencil_enabled;
    GLenum stencil_func;
};

struct GLRasterizerState {
    GLboolean cull_enabled;
    GLint front_face;
    GLint cull_face;
};

struct GLPipeline {
    GLuint vao;
    GLuint program;
#if 0
    struct GLBlendState blend_state;
    struct GLDepthStencilState depth_stencil_state;
    struct GLRasterizerState rasterizer_state;
#endif
};
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
struct DX11Pipeline {
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* input_layout;
    ID3D11BlendState* blend_state;
    ID3D11DepthStencilState* depth_stencil_state;
    ID3D11RasterizerState* rasterizer_state;
    D3D11_PRIMITIVE_TOPOLOGY topology;
};
#endif
#ifndef SELENE_NO_DX12
struct DX12Pipeline {
    ID3D12PipelineState* handle;
};
#endif
#endif

struct selene_BlendState {
    int enabled;
    int src, dst;
    int equation;
    void* handle;
};

struct selene_DepthStencilState {
    int depth_enabled, stencil_enabled;
    int depth_func, stencil_func;
    int depth_write_mask;
    void* handle;
};

struct selene_RasterState {
    int fill_mode;
    int scissor_enabled;
    int cull_enabled;
    int cull_face, front_face;
    void* handle;
};

typedef struct selene_RenderPipeline selene_RenderPipeline;
struct selene_RenderPipeline {
    struct selene_InputLayout layout;
    selene_Shader* vs;
    selene_Shader* ps;
    struct selene_BlendState blend_state;
    struct selene_DepthStencilState depth_stencil_state;
    struct selene_RasterState raster_state;
    union {
#ifndef SELENE_NO_GL
        struct GLPipeline gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkPipeline handle;
            VkPipelineLayout layout;
            VkDescriptorSetLayout descriptor_layout;
            VkShaderModule vs;
            VkShaderModule ps;
            // Cache
            VkPipelineVertexInputStateCreateInfo vertex_input_info;
        } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
        struct DX11Pipeline dx11;
#endif
#ifndef SELENE_NO_DX12
        struct DX12Pipeline dx12;
#endif
#endif
    };
    void* handle;
};

/**
 * Renderer
 */

enum RenderCommandType {
    RENDER_COMMAND_CLEAR = 0,
    RENDER_COMMAND_CLEAR_COLOR,
    RENDER_COMMAND_CLEAR_DEPTH,

    RENDER_COMMAND_SET_PROJECTION,
    RENDER_COMMAND_SET_VIEW,

    RENDER_COMMAND_SET_PIPELINE,
    // Buffers
    RENDER_COMMAND_SET_VERTEX_BUFFER,
    RENDER_COMMAND_SET_INDEX_BUFFER,
    RENDER_COMMAND_SET_UNIFORM_BUFFER,
    // Shader (OpenGL and DX11)
    RENDER_COMMAND_SET_SHADER,
    // Texture
    RENDER_COMMAND_SET_TEXTURE,
    // Render Target
    RENDER_COMMAND_SET_RENDER_TARGET,

    RENDER_COMMAND_SET_SCISSOR,
    RENDER_COMMAND_SET_BLEND_MODE,
    RENDER_COMMAND_SET_CULL_FACE,
    RENDER_COMMAND_SET_VIEWPORT,

    RENDER_COMMAND_INTEGER_UNIFORM,
    RENDER_COMMAND_FLOAT_UNIFORM,

    RENDER_COMMAND_MATRIX_UNIFORM,

    RENDER_COMMAND_DRAW_VERTEX,
    RENDER_COMMAND_DRAW_INDEX,
    RENDER_COMMAND_DRAW_VERTEX_INSTANCED,
    RENDER_COMMAND_DRAW_INDEX_INSTANCED,

    RENDER_COMMAND_CUSTOM,

    RENDER_COMMAND_COUNT
};

struct RenderCommand {
    enum RenderCommandType type;
    union {
        struct {
            Uint32 program;
            int location;
            union {
                Sint8 c;
                Uint8 uc;
                Sint16 s;
                Uint16 us;
                Sint32 i;
                Uint32 ui;
                float f;
                double d;
                vec2 v2;
                vec3 v3;
                vec4 v4;
                mat4 m;
            };
        } uniform;
        union { float depth; float color[4]; } clear;
        struct { int x, y, width, height; } clip;

        struct { int width, height; } size;

        struct { Uint32 target; Uint32 handle; GpuBuffer* ptr; } buffer;
        struct { Uint32 handle; } program;
        struct { Uint32 slot; Uint32 target; Uint32 handle; void* ptr; } texture;
        struct { int depth; Uint32 target; Uint32 handle; selene_RenderTarget* ptr; } target;

        struct { int x, y, width, height; } viewport;
        struct { int enabled; int x, y, width, height; } scissor;
        struct {
            int func0, func1;
            int equation;
        } blend;

        selene_RenderPipeline* pipeline;

        struct {
            Uint32 mode;
            int start, count;
        } draw;

        struct {
            Uint32 mode;
            int start, count;
            int instance_count;
        } instanced;
    };
};

typedef struct VertexBatch VertexBatch;
struct VertexBatch {
    int offset, count;
    int stride;
    void* data;
    float z;
    float color[4];
};

typedef struct selene_Renderer selene_Renderer;

enum SeleneRendererBackend {
    SELENE_RENDERER_OPENGL = 0,
    SELENE_RENDERER_VULKAN,
    SELENE_RENDERER_DIRECTX11,
    SELENE_RENDERER_DIRECTX12,
    SELENE_RENDERER_METAL
};

#ifndef SELENE_NO_VULKAN
struct VulkanContext {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice phys_device;
    VkDevice device;
    VkCommandBuffer commands;
    VkRenderPass render_pass;
    // Queues
    VkQueue graphics_queue;
    VkQueue present_queue;
};

struct VulkanSwapchain {
    VkSwapchainKHR handle;
    VkImage* images;
    VkImageView* views;
    VkFormat format;
    VkExtent2D extent;
    uint32_t image_count;
};
#endif

struct selene_Renderer {
    enum SeleneRendererBackend backend;
    LuaReference l_window_ref;
    SDL_Window* window_ptr;

    union {
#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
        struct {
            ID3D11Device* device;
            ID3D11DeviceContext* context;
            IDXGISwapChain* swap_chain;
        } dx11;
#endif
#ifndef SELENE_NO_DX12
        struct {
            ID3D12Device* device;
        } dx12;
#endif
#endif
#ifndef SELENE_NO_GL
        SDL_GLContext gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkInstance instance;
            VkSurfaceKHR surface;
            VkPhysicalDevice phys_device;
            VkDevice device;
            VkRenderPass render_pass;
            VkCommandPool command_pool;
            VkCommandBuffer command_buffer;
            VkFramebuffer* framebuffers;
            int current_framebuffer;
            // Queues
            VkQueue graphics_queue;
            VkQueue present_queue;
            struct VulkanSwapchain swapchain;
            // Sync
            VkSemaphore image_semaphore;
            VkSemaphore render_semaphore;
            VkFence fence;
        } vk;
#endif
    };

    int width, height;
    selene_RenderTarget default_target;

    int command_offset;
    int command_count;
    struct RenderCommand* command_pool;

    lua_CFunction destroy;

    // pipeline
    lua_CFunction create_pipeline;
    lua_CFunction destroy_pipeline;
    // buffers
    lua_CFunction create_buffer;
    lua_CFunction destroy_buffer;
    lua_CFunction send_buffer_data;
    // texture
    lua_CFunction create_texture2d;
    lua_CFunction create_depth_texture;
    lua_CFunction destroy_texture;
    // render target
    lua_CFunction create_render_target;
    lua_CFunction destroy_render_target;
    // shader
    lua_CFunction create_shader;
    lua_CFunction destroy_shader;

    lua_CFunction draw;

    lua_CFunction flush;
    lua_CFunction present;
};

#define RENDERER_CLASS LUA_META_CLASS(Renderer)
#define RENDERCOMMANDLIST_CLASS LUA_META_CLASS(RenderCommandList)

#define TEXTURE2D_CLASS LUA_META_CLASS(Texture2D)
#define FRAMEBUFFER_CLASS LUA_META_CLASS(Framebuffer)
#define CANVAS_CLASS LUA_META_CLASS(Canvas)

#define FONT_CLASS LUA_META_CLASS(Font)

#define EFFECT2D_CLASS LUA_META_CLASS(Effect2D)

#define RENDER_BATCH2D_CLASS LUA_META_CLASS(RenderBatch2D)
#define VERTEX_BATCH2D_CLASS LUA_META_CLASS(VertexBatch2D)

#include "stb_image.h"

#endif /* SELENE_RENDERER_H_ */
