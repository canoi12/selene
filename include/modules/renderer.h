#include "selene.h"
#include "lua_helper.h"

#ifndef SELENE_RENDERER_H_
#define SELENE_RENDERER_H_

#ifndef SELENE_NO_VULKAN
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
    #include <d3d11.h>
#endif
#ifndef SELENE_NO_D3D12
    #include <d3d12.h>
#endif
#if !defined(SELENE_NO_D3D11) && !defined(SELENE_NO_D3D12)
    #include <dxgi.h>
    #include <dxgi1_2.h>
    #include <dxgi1_3.h>
    #include <d3dcompiler.h>
#endif
#endif

#ifndef SELENE_BATCH2D_PROJECTION_FAR
#define SELENE_BATCH2D_PROJECTION_FAR 1000
#endif

#ifndef SELENE_PROJECTION_MATRIX_NAME
#define SELENE_PROJECTION_MATRIX_NAME "m_projection"
#endif

#ifndef SELENE_VIEW_MATRIX_NAME
#define SELENE_VIEW_MATRIX_NAME "m_view"
#endif

#ifndef SELENE_MODEL_MATRIX_NAME
#define SELENE_MODEL_MATRIX_NAME "m_model"
#endif

#ifndef SELENE_MATRICES_UNIFORM_NAME
#define SELENE_MATRICES_UNIFORM_NAME "Matrices"
#endif

#if defined(__cplusplus)
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif
/* String tables for luaL_checkoption — indices match the enums below. */
EXTERN const char* pixel_formats[];
EXTERN const char* texture_filters[];
EXTERN const char* draw_modes[];
EXTERN const char* shader_type_options[];
EXTERN const char* buffer_target_options[];
EXTERN const char* texture_target_options[];
EXTERN const char* front_face_options[];
EXTERN const char* cull_face_options[];
EXTERN const char* polygon_mode_options[];
EXTERN const char* type_name_options[];
EXTERN const char* comparison_func_options[];
EXTERN const char* vertex_format_options[];
EXTERN const char* renderer_backend_options[];
EXTERN const char* blend_mode_options[];
EXTERN const char* index_type_options[];
EXTERN const char* pipeline_binding_options[];

/**
 * Backend-agnostic GPU enums.
 * Drivers translate these to GL / Vulkan / D3D / Metal / AGC / GNM.
 */
enum selene_GpuValueType {
    SELENE_BYTE = 0,
    SELENE_UBYTE,
    SELENE_SHORT,
    SELENE_USHORT,
    SELENE_INT,
    SELENE_UINT,
    SELENE_FLOAT,
    SELENE_DOUBLE
};

enum selene_GpuBufferTarget {
    SELENE_BUFFER_VERTEX = 0,
    SELENE_BUFFER_INDEX,
    SELENE_BUFFER_UNIFORM,
    SELENE_BUFFER_STORAGE
};

enum selene_ShaderStage {
    SELENE_SHADER_VERTEX = 0,
    SELENE_SHADER_PIXEL,
    SELENE_SHADER_COMPUTE
};

enum selene_DrawTopology {
    SELENE_TOPOLOGY_POINTS = 0,
    SELENE_TOPOLOGY_LINES,
    SELENE_TOPOLOGY_TRIANGLES
};

enum selene_CompareFunc {
    SELENE_COMPARE_NEVER = 0,
    SELENE_COMPARE_LESS,
    SELENE_COMPARE_EQUAL,
    SELENE_COMPARE_LESS_EQUAL,
    SELENE_COMPARE_GREATER,
    SELENE_COMPARE_NOT_EQUAL,
    SELENE_COMPARE_GREATER_EQUAL,
    SELENE_COMPARE_ALWAYS
};

enum selene_TextureFilter {
    SELENE_FILTER_NEAREST = 0,
    SELENE_FILTER_LINEAR
};

enum selene_TextureType {
    SELENE_TEXTURE_2D = 0,
    SELENE_TEXTURE_3D,
    SELENE_TEXTURE_CUBE
};

enum selene_FrontFace {
    SELENE_FRONT_CW = 0,
    SELENE_FRONT_CCW
};

enum selene_CullFace {
    SELENE_CULL_NONE = 0,
    SELENE_CULL_FRONT,
    SELENE_CULL_BACK,
    SELENE_CULL_BOTH
};

enum selene_BlendMode {
    SELENE_BLEND_ALPHA = 0,
    SELENE_BLEND_ADDITIVE,
    SELENE_BLEND_SUBTRACT,
    SELENE_BLEND_MULTIPLY
};

enum selene_GpuPixelFormat {
    SELENE_FORMAT_RGB = 0,
    SELENE_FORMAT_RGBA,
    SELENE_FORMAT_DEPTH,
    SELENE_FORMAT_BGRA
};

enum selene_IndexType {
    SELENE_INDEX_UINT16 = 0,
    SELENE_INDEX_UINT32
};

enum selene_PolygonMode {
    SELENE_POLYGON_FILL = 0,
    SELENE_POLYGON_LINE
};

enum selene_PipelineBindingType {
    SELENE_BINDING_UNIFORM = 0,
    SELENE_BINDING_TEXTURE,
    SELENE_BINDING_STORAGE
};

enum {
    SELENE_STAGE_VERTEX = 1 << 0,
    SELENE_STAGE_PIXEL  = 1 << 1,
    SELENE_STAGE_COMPUTE = 1 << 2
};

#ifndef SELENE_PIPELINE_MAX_BINDINGS
#define SELENE_PIPELINE_MAX_BINDINGS 16
#endif

#undef EXTERN

/**
 * Renderer Datas
 */
typedef struct {
    mat4 projection;
    mat4 view;

    float screen_width;
    float screen_height;
    float time;
    float delta_time;
} selene_FrameData;

typedef struct {
    mat4 model;
    vec4 tint_color;
    vec2 tiling;
    vec2 offset;
} SpriteData;

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

#ifndef SELENE_NO_OPENGL
struct GLBuffer {
    int type;
    GLuint handle;
    GLboolean usage;
};
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
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

#if 0
typedef struct {
    Uint32 type;
    Uint32 stride, size;
    union {
#ifndef SELENE_NO_OPENGL
        struct GLBuffer gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkBuffer handle;
            VkDeviceMemory mem;
        } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct DX11Buffer dx11;
#endif
#endif
    };
} GpuBuffer;
#endif

typedef struct _GpuBuffer selene_GpuBuffer;
struct _GpuBuffer {
    int type, usage;
    Uint32 stride, size;
    int index_type; /* SELENE_INDEX_* when type is SELENE_BUFFER_INDEX */
    union {
#ifndef SELENE_NO_OPENGL
        struct { int type; GLuint handle; } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct { VkBuffer handle; VkDeviceMemory mem; } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct DX11Buffer dx11;
#endif
#endif
    };
};

/**
 * Texture Types
 */

#ifndef SELENE_NO_OPENGL
struct GLTexture {
    GLuint handle;
};
#endif

#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
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

typedef struct _Texture2D selene_Texture2D;
struct _Texture2D {
    int id;
    union {
#ifndef SELENE_NO_OPENGL
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
#ifndef SELENE_NO_D3D11
        struct DX11Texture dx11;
#endif
#endif
    };
    int width, height;
    int format;
};

typedef struct _Texture selene_Texture;
struct _Texture {
    int type;
    int width, height;
    int depth;
    int mip_level;
    int pixel_format;
    union {
#ifndef SELENE_NO_OPENGL
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
#ifndef SELENE_NO_D3D11
        struct {
            void* handle;
            ID3D11ShaderResourceView* srv;
            ID3D11SamplerState* sampler;
        } dx11;
#endif
#endif
    };
};

typedef struct _Font selene_Font;
struct _Font {
    selene_Texture2D texture;
    selene_FontGlyph glyphs[256];
};

/**
 * Render Target
 */

typedef struct _RenderTarget selene_RenderTarget;
struct _RenderTarget {
    union {
#ifndef SELENE_NO_OPENGL
        struct {
            GLuint fbo;
        } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkFramebuffer handle;
        } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct {
            ID3D11RenderTargetView* rtv;
            ID3D11DepthStencilView* dsv;
            ID3D11ShaderResourceView* srv;
        } dx11;
#endif
#endif
    };
    selene_Texture2D* color;
    selene_Texture2D* depth;
    int width, height;
};


/**
 * Shader
 */

#ifndef SELENE_NO_OPENGL
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
#ifndef SELENE_NO_D3D11
struct DX11Shader {
    union {
        ID3D11VertexShader* vertex;
        ID3D11PixelShader* pixel;
    };
    ID3DBlob* blob;
};
#endif
#ifndef SELENE_NO_D3D12
struct DX12Shader {
    ID3DBlob* blob;
};
#endif
#endif

typedef struct _Shader selene_Shader;
struct _Shader {
    int id;
    Uint32 type;
    union {
#ifndef SELENE_NO_OPENGL
        struct { GLuint handle; } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct { VkShaderModule handle; } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct DX11Shader dx11;
#endif
#ifndef SELENE_NO_D3D12
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

#ifndef SELENE_NO_OPENGL
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
#ifndef SELENE_NO_D3D11
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
#ifndef SELENE_NO_D3D12
struct DX12Pipeline {
    ID3D12PipelineState* handle;
};
#endif
#endif

struct _BlendState {
    int enabled;
    int mode; /* SELENE_BLEND_* */
    void* handle;
};

struct _DepthStencilState {
    int depth_enabled, stencil_enabled;
    int depth_func, stencil_func;
    int depth_write_mask;
    void* handle;
};

struct _RasterState {
    int fill_mode;
    int scissor_enabled;
    int cull_enabled;
    int cull_face, front_face;
    void* handle;
};

struct selene_PipelineBinding {
    int slot;        /* Lua slot: set_texture(slot) / set_uniform_buffer(slot) */
    int type;        /* SELENE_BINDING_* */
    int stage_flags; /* SELENE_STAGE_* bits */
    int count;
};

/* CPU-side pipeline description. Drivers only translate this to native PSOs.
 * Viewport/scissor/clip convention: top-left origin, Y-down, Z in [0, 1]. */
struct selene_PipelineDesc {
    selene_Shader* vs;
    selene_Shader* ps;
    int topology; /* SELENE_TOPOLOGY_* */
    struct selene_InputLayout layout;
    struct _BlendState blend;
    struct _DepthStencilState depth;
    struct _RasterState raster;
    int binding_count;
    struct selene_PipelineBinding bindings[SELENE_PIPELINE_MAX_BINDINGS];
};

typedef struct _RenderPipeline selene_RenderPipeline;
struct _RenderPipeline {
    int id;
    int topology; /* SELENE_TOPOLOGY_* — baked into PSO on VK/D3D12/Metal/AGC */
    struct selene_InputLayout layout;
    selene_Shader* vs;
    selene_Shader* ps;
    struct _BlendState blend_state;
    struct _DepthStencilState depth_stencil_state;
    struct _RasterState raster_state;
    int binding_count;
    struct selene_PipelineBinding bindings[SELENE_PIPELINE_MAX_BINDINGS];
    union {
#ifndef SELENE_NO_OPENGL
        struct GLPipeline gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            VkPipeline handle;
            VkPipelineLayout layout;
            VkDescriptorSetLayout descriptor_layout;
            VkDescriptorSet* descriptor_set;
            VkShaderModule vs;
            VkShaderModule ps;
            // Cache
            VkPipelineVertexInputStateCreateInfo vertex_input_info;
        } vk;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct DX11Pipeline dx11;
#endif
#ifndef SELENE_NO_D3D12
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
    RENDER_COMMAND_BEGIN_PASS = 0,
    RENDER_COMMAND_END_PASS,

    RENDER_COMMAND_CLEAR_COLOR,
    RENDER_COMMAND_CLEAR_DEPTH,

    RENDER_COMMAND_SET_PIPELINE,
    RENDER_COMMAND_SET_VERTEX_BUFFER,
    RENDER_COMMAND_SET_INDEX_BUFFER,
    RENDER_COMMAND_SET_UNIFORM_BUFFER,
    RENDER_COMMAND_SET_TEXTURE,
    RENDER_COMMAND_SET_RENDER_TARGET,

    RENDER_COMMAND_SET_SCISSOR,
    RENDER_COMMAND_SET_VIEWPORT,

    /* GL / D3D11 leftovers — explicit APIs ignore these (state lives in the PSO). */
    RENDER_COMMAND_SET_SHADER,
    RENDER_COMMAND_SET_BLEND_MODE,
    RENDER_COMMAND_SET_CULL_FACE,

    RENDER_COMMAND_UPDATE_UNIFORM_BUFFER,

    RENDER_COMMAND_INTEGER_UNIFORM,
    RENDER_COMMAND_FLOAT_UNIFORM,
    RENDER_COMMAND_MATRIX_UNIFORM,
    RENDER_COMMAND_SET_PROJECTION,
    RENDER_COMMAND_SET_VIEW,

    RENDER_COMMAND_DRAW,
    RENDER_COMMAND_DRAW_INDEXED,

    RENDER_COMMAND_PUSH_CONSTANTS,
    RENDER_COMMAND_CUSTOM,
    RENDER_COMMAND_COUNT
};

typedef struct {
    unsigned int binding;
    unsigned int slot;
    unsigned int target;
    selene_Texture2D* ptr;
} SetTextureCommand;

typedef struct {
    unsigned int binding;
    selene_GpuBuffer* ptr;
} SetBufferCommand;

typedef struct {
    int has_depth;
    unsigned int target;
    selene_RenderTarget* ptr;
} SetRenderTargetCommand;

enum {
    UNIFORM_TYPE_INT,
    UNIFORM_TYPE_INT2,
    UNIFORM_TYPE_INT3,
    UNIFORM_TYPE_INT4,

    UNIFORM_TYPE_FLOAT,
    UNIFORM_TYPE_FLOAT2,
    UNIFORM_TYPE_FLOAT3,
    UNIFORM_TYPE_FLOAT4
};

struct RenderCommand {
    enum RenderCommandType type;
    union {
        struct {
            selene_RenderTarget* target; /* NULL = swapchain / default target */
            int clear_color;
            float color[4];
            int clear_depth;
            float depth;
            int has_viewport;
            struct { int x, y, width, height; } viewport;
            int has_scissor;
            struct { int enabled; int x, y, width, height; } scissor;
        } pass;

        struct {
            selene_GpuBuffer* buffer; /* RENDER_COMMAND_UPDATE_UNIFORM_BUFFER */
            int offset;
            int size;
            int location;
            int type;
            union {
                Sint32 i;
                Uint32 ui;
                float f;
                vec2 v2;
                vec3 v3;
                vec4 v4;
                mat4 m;
                selene_FrameData frame;
            };
        } uniform;

        union { float depth; float color[4]; } clear;

        struct {
            int slot;
            int offset;
            int index_type;
            selene_GpuBuffer* ptr;
        } buffer;

        struct { Uint32 handle; } program;

        struct {
            int slot;
            int target; /* SELENE_TEXTURE_* */
            void* ptr;
        } texture;

        struct {
            int depth;
            selene_RenderTarget* ptr;
        } target;

        struct { int x, y, width, height; } viewport;
        struct { int enabled; int x, y, width, height; } scissor;
        struct { int func0, func1, equation; } blend;

        selene_RenderPipeline* pipeline;

        struct {
            int topology;       /* SELENE_TOPOLOGY_* */
            int start, count;
            int instance_count; /* 1 = non-instanced */
            int base_vertex;
        } draw;

        struct {
            int stage_flags;
            int offset, size;
            void* data;
        } push_constants;
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

#ifndef VertexBatch_METANAME
#define VertexBatch_METANAME "VertexBatch"
#endif

enum _RendererBackend {
    SELENE_RENDERER_OPENGL = 0,
    SELENE_RENDERER_VULKAN,
    SELENE_RENDERER_DIRECTX11,
    SELENE_RENDERER_DIRECTX12,
    SELENE_RENDERER_METAL,
    SELENE_RENDERER_AGC,
    SELENE_RENDERER_GNM
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

struct selene_GLCapabilities {
    int is_gles;         /* OpenGL ES context */
    int major, minor;    /* context version */
    int has_vao;         /* GL 3.0+ / GLES 3.0+ / OES_vertex_array_object */
    int has_instancing;  /* GL 3.0+ / GLES 3.0+ / ANGLE_instanced_arrays */
    int has_ubo;         /* GL 3.1+ / GLES 3.0+ */
    int has_blend_equation; /* GL 1.4+ / GLES 3.0+ / EXT_blend_subtract */
    int has_depth_texture;  /* GL_ARB_depth_texture / OES_depth_texture */
    int has_bgra;        /* desktop / EXT_texture_format_BGRA8888 */
    int has_unpack_flip_y; /* desktop / EXT_unpack_subimage */
};

struct _Renderer {
    enum _RendererBackend backend;
    LuaReference l_window_ref;
    selene_Window* window_ptr;

    union {
#if defined(OS_WIN)
#ifndef SELENE_NO_D3D11
        struct {
            ID3D11Device* device;
            ID3D11DeviceContext* context;
            IDXGISwapChain* swap_chain;
        } dx11;
#endif
#ifndef SELENE_NO_D3D12
        struct {
            ID3D12Device* device;
        } dx12;
#endif
#endif
#ifndef SELENE_NO_OPENGL
        struct {
            SDL_GLContext ctx;
            struct selene_GLCapabilities caps;
        } gl;
#endif
#ifndef SELENE_NO_VULKAN
        struct {
            int framebuffer_count;
            VkInstance instance;
            VkSurfaceKHR surface;
            VkPhysicalDevice phys_device;
            VkDevice device;
            VkRenderPass render_pass;
            VkCommandPool command_pool;
            VkCommandBuffer command_buffer;
            VkFramebuffer* framebuffers;
            unsigned int current_framebuffer;
            VkDescriptorSetLayout descriptor_set_layout;
            VkDescriptorPool descriptor_pool;
            VkDescriptorSet* descriptor_sets;
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
    int in_frame;
    int in_pass;
    selene_RenderTarget default_target;

    int command_offset;
    int command_count;
    struct RenderCommand* command_pool;

    lua_CFunction destroy;

    /* Frame / pass — explicit APIs (VK, D3D12, Metal, AGC, GNM) override these.
     * Defaults record BEGIN/END_PASS commands for GL / D3D11. */
    lua_CFunction begin_frame;
    lua_CFunction end_frame;
    lua_CFunction begin_pass;
    lua_CFunction end_pass;

    lua_CFunction create_pipeline;
    lua_CFunction destroy_pipeline;
    lua_CFunction create_buffer;
    lua_CFunction destroy_buffer;
    lua_CFunction send_buffer_data;
    lua_CFunction create_texture2d;
    lua_CFunction create_depth_texture;
    lua_CFunction destroy_texture;
    lua_CFunction create_render_target;
    lua_CFunction destroy_render_target;
    lua_CFunction create_shader;
    lua_CFunction destroy_shader;

    lua_CFunction flush;
    lua_CFunction present;
};

#ifdef __cplusplus
extern "C" {
#endif
void renderer_push_commands(selene_Renderer* r, int count, const struct RenderCommand* cmd);
#define renderer_push_command(r, cmd) renderer_push_commands((r), 1, (cmd))
int renderer_init_command_pool(selene_Renderer* r, int count);

int renderer_parse_pipeline_desc(lua_State* L, int tbl, struct selene_PipelineDesc* desc);
void renderer_apply_pipeline_desc(selene_RenderPipeline* pipe, const struct selene_PipelineDesc* desc);
int renderer_attrib_byte_stride(int type, int size);
int renderer_pipeline_binding_index(const selene_RenderPipeline* pipe, int binding_type, int slot);
void renderer_clip_correct(const selene_Renderer* r, mat4 proj);
void renderer_clip_ortho(const selene_Renderer* r,
                         float left, float right, float bottom, float top,
                         float zn, float zf, mat4 dest);

/* Id counters shared by the renderer drivers. They are defined in the C
 * translation unit renderer_meta.c, so they need C linkage here as well:
 * MSVC mangles C++ globals (e.g. ?next_pipeline_id@@3HA) while the definition
 * keeps the unmangled C name, which fails at link time with LNK2019. */
extern int next_texture_id;
extern int next_pipeline_id;
extern int next_shader_id;
#ifdef __cplusplus
}
#endif

static inline void renderer_get_target_size(const selene_Renderer* r, const selene_RenderTarget* t, int* w, int* h) {
    if (t && t->color) {
        *w = t->color->width;
        *h = t->color->height;
    } else if (t && t->width > 0 && t->height > 0) {
        *w = t->width;
        *h = t->height;
    } else {
        *w = r->width;
        *h = r->height;
    }
}

#endif /* SELENE_RENDERER_H_ */
