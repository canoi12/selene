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
// #include <vulkan.h>
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

#if 0
#define RENDERLIST_CLEAR(list)\
lua_pushcfunction(L, (list)->clear);\
lua_pushlightuserdata(L, (list));\
lua_call(L, 1, 0)
#else
#define RENDERLIST_CLEAR(list)\
(list)->clear((list))
#endif

#if 0
#define RENDERLIST_CALL(list)\
lua_pushcfunction(L, (list)->call);\
lua_pushlightuserdata(L, (list));\
lua_call(L, 1, 0)
#else
#define RENDERLIST_CALL(list)\
(list)->call((list))
#endif

#if 0
#define RENDERLIST_PUSH(list, rc)\
lua_pushcfunction(L, (list)->push);\
lua_pushlightuserdata(L, (list));\
lua_pushlightuserdata(L, (rc));\
lua_call(L, 2, 0)
#else
#define RENDERLIST_PUSH(list, rc)\
(list)->push((list), 1, rc)
#endif

#if 0
#define RENDERLIST_POP(list)\
lua_pushcfunction(L, (list)->pop);\
lua_pushlightuserdata(L, (list));\
lua_call(L, 1, 1)
#else
#define RENDERLIST_POP(list)\
(list)->pop((list))
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

typedef Uint32 BufferID;

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

typedef struct {
    Uint32 type;
    Uint32 stride, size;
    union {
#ifndef SELENE_NO_GL
        struct GLBuffer gl;
#endif
#if defined(OS_WIN)
#ifndef SELENE_NO_DX11
        struct DX11Buffer dx11;
#endif
#endif
    };
} GpuBuffer;

typedef struct VertexBatch2D VertexBatch2D;
struct VertexBatch2D {
    GpuBuffer vbo;
    int offset;
    size_t count;
    Vertex2D* data;

    void(*check_size)(VertexBatch2D*,int);
    void(*push)(VertexBatch2D*,int,Vertex2D*);
    void(*flush)(VertexBatch2D*,int,size_t);
};

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
} RenderTarget;


/**
 * Shader
 */

#ifndef SELENE_NO_GL
struct GLShader {
    Uint32 handle;
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

typedef struct SeleneShader SeleneShader;
struct SeleneShader {
    Uint32 type;
    union {
        struct GLShader gl;
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

struct InputLayout {
    int count;
    size_t vertex_stride;
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
    struct GLBlendState blend_state;
    struct GLDepthStencilState depth_stencil_state;
    struct GLRasterizerState rasterizer_state;
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

typedef struct RenderPipeline RenderPipeline;
struct RenderPipeline {
    struct InputLayout layout;
    union {
#ifndef SELENE_NO_GL
        struct GLPipeline gl;
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
        struct { int depth; Uint32 target; Uint32 handle; RenderTarget* ptr; } target;

        struct { int x, y, width, height; } viewport;
        struct { int enabled; int x, y, width, height; } scissor;
        struct {
            int func0, func1;
            int equation;
        } blend;

        RenderPipeline* pipeline;

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
#if 0
struct RenderCommandPool {
    int current;
    struct RenderCommand commands[512];
    struct RenderCommandPool* prev;
    struct RenderCommandPool* next;
};
#endif

typedef struct BatchBuffer BatchBuffer;
struct BatchBuffer {
    int offset, count;
    int stride;
    void* data;
    vec4 color;
    float z;
};

typedef struct VertexBatch VertexBatch;
struct VertexBatch {
    int offset, count;
    int stride;
    void* data;
    vec4 color;
};

typedef struct RenderCommandBlock RenderCommandBlock;
struct RenderCommandBlock {
    int current;
    struct RenderCommand commands[512];
};

typedef struct Renderer Renderer;
typedef struct SeleneRenderer SeleneRenderer;

enum SeleneRendererBackend {
    SELENE_RENDERER_OPENGL = 0,
    SELENE_RENDERER_VULKAN,
#if defined(OS_WIN)
    SELENE_RENDERER_DIRECTX11,
    SELENE_RENDERER_DIRECTX12,
#endif
#if defined(OS_MACOS)
    SELENE_RENDERER_METAL
#endif
};

struct SeleneRenderer {
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
    };

    RenderTarget default_target;

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

#if 0
struct Renderer {
    int l_window_ref;
    SDL_Window* window_ptr;

    int l_gl_context_ref;

    vec4 clear_color;

    int current_pipeline_ref;
    int current_texture_ref;

    // vertex array
    int current_vao_id;
    // buffer
    int current_vbo_id;
    int current_ibo_id;
    // texture
    int current_tex2d_id;
    int current_fbo_id;
    int current_rbo_id;
    // program
    int current_program_id;

    // render list
    int l_command_list_ref;
    RenderCommandList* command_list;

    void(*present)(Renderer*, lua_State*);
};
#endif

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
