#include "selene.h"
#include "lua_helper.h"

#ifndef SELENE_RENDERER_H_
#define SELENE_RENDERER_H_

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

extern const char* pixel_formats[];
extern const int pixel_formats_values[];

extern const char* texture_filters[];
extern const int texture_filters_values[];

extern const char* draw_modes[];
extern const int draw_modes_values[];

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
typedef Uint32 VertexArrayID;

typedef struct {
    int vertex_stride;
    struct {
        int location;
        int stride;
        int type;
    } attrib[16];
} VertexFormat;

typedef struct {
    VertexArrayID handle;
#if defined(OS_EMSCRIPTEN) || defined(OS_ANDROID)
    BufferID vertex_id;
    BufferID index_id;
#endif
} VertexArray;

typedef struct {
    BufferID handle;
} Buffer;

typedef struct VertexBatch2D VertexBatch2D;
struct VertexBatch2D {
    VertexArray vao;
    Buffer vbo;
    int offset;
    size_t count;
    Vertex2D* data;

    void(*check_size)(VertexBatch2D*,int);
    void(*push)(VertexBatch2D*,int,Vertex2D*);
    void(*send)(VertexBatch2D*,int,size_t);
};

/**
 * Texture Types
 */

typedef Uint32 TextureID; // OpenGL TextureID

typedef struct {
    TextureID handle;
    int width, height;
} Texture2D;

typedef struct {
    TextureID handle; // Handle to the cube map texture
    int size;      // Size of each face of the cube map
} TextureCube;

typedef struct {
    Texture2D texture;
    FontGlyph glyphs[256];
} Font;

/**
 * Framebuffer
 */

// OpenGL IDs
typedef Uint32 FramebufferID;
typedef Uint32 RenderbufferID;

typedef struct {
    FramebufferID handle;
} Framebuffer;

typedef struct {
    Texture2D texture;
    FramebufferID fbo;
    FramebufferID depth;
} Canvas;


/**
 * Effect types
 */

typedef Uint32 ShaderID;
typedef Uint32 ProgramID;

typedef struct {
    ProgramID handle;
    // attributes
    int position_location;
    int color_location;
    int texcoord_location;
    // uniforms
    int model_view_location;
    int projection_location;
} Effect2D;

typedef struct {
    ProgramID handle;
    // attributes
    int position_location;
    int normal_position;
    int texcoord_position;
    // uniforms
    int model_location;
    int view_location;
    int projection_location;
} Effect3D;

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
 * Renderer
 */

enum RenderCommandType {
    RENDER_COMMAND_CLEAR = 0,
    RENDER_COMMAND_ENABLE,
    RENDER_COMMAND_DISABLE,

    RENDER_COMMAND_SET_PROJECTION,
    RENDER_COMMAND_SET_VIEW,

    RENDER_COMMAND_SET_VERTEX_ARRAY,
    RENDER_COMMAND_SET_BUFFER,
    RENDER_COMMAND_SET_PROGRAM,
    RENDER_COMMAND_SET_TEXTURE,
    RENDER_COMMAND_SET_FRAMEBUFFER,
    RENDER_COMMAND_SET_RENDERBUFFER,

    RENDER_COMMAND_SET_SCISSOR,

    RENDER_COMMAND_SET_BLEND_MODE,

    RENDER_COMMAND_INTEGER_UNIFORM,
    RENDER_COMMAND_FLOAT_UNIFORM,
    RENDER_COMMAND_MATRIX_UNIFORM,

    RENDER_COMMAND_SET_VIEWPORT,

    RENDER_COMMAND_DRAW_VERTEX,
    RENDER_COMMAND_DRAW_INDEX,
    RENDER_COMMAND_DRAW_VERTEX_INSTANCED,
    RENDER_COMMAND_DRAW_INDEX_INSTANCED,
    
    RENDER_COMMAND_ENABLE_ATTRIB,
    RENDER_COMMAND_DISABLE_ATTRIB,
    RENDER_COMMAND_DISABLE_ALL_ATTRIBS,

    RENDER_COMMAND_VERTEX_ATTRIB_POINTER,

    RENDER_COMMAND_ENABLE_CLIP_RECT,
    RENDER_COMMAND_DISABLE_CLIP_RECT,

    RENDER_COMMAND_CUSTOM,

    RENDER_COMMAND_COUNT
};

struct RenderCommand {
    enum RenderCommandType type;
    union {
        struct {
            int index;
            int size;
            int type;
            int norm;
            int stride;
            int offset;
        } attrib;
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
                mat4 m;
            };
        } uniform;
        struct { Uint32 mask; float color[4]; } clear;
        struct { int x, y, width, height; } clip;
        struct { Uint32 attrib; } enable;
        struct { Uint32 attrib; } disable;

        struct { int width, height; } size;

        struct { Uint32 handle; } vao;
        struct { Uint32 target; Uint32 handle; } buffer;
        struct { Uint32 handle; } program;
        struct { Uint32 slot; Uint32 target; Uint32 handle; } texture;
        struct { Uint32 target; Uint32 handle; } target;

        struct { int x, y, width, height; } viewport;
        struct {
            int func0, func1;
            int equation;
        } blend;

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

typedef struct RenderCommandBlock RenderCommandBlock;
struct RenderCommandBlock {
    int current;
    struct RenderCommand commands[512];
};

typedef struct Renderer Renderer;
typedef void(*ClearRenderListFunc)(Renderer*);
typedef void(*PushRenderListFunc)(Renderer*, struct RenderCommand*);
typedef struct RenderCommand*(*PopRenderListFunc)(Renderer*);
typedef void(*CallRenderListFunc)(Renderer*);

typedef struct RenderList RenderList;
struct RenderList {
#if 0
    ClearRenderListFunc clear;
    PushRenderListFunc push;
    PopRenderListFunc pop;
    CallRenderListFunc call;
#endif
    lua_CFunction clear;
    lua_CFunction push;
    lua_CFunction pop;
    lua_CFunction call;

    int l_render_blocks_table_ref;
    int block_index;
    struct RenderCommandBlock* block_ptr;
};

typedef struct RenderCommandList RenderCommandList;
struct RenderCommandList {
    void(*clear)(RenderCommandList*);
    void(*push)(RenderCommandList*, int count, struct RenderCommand*);
    const struct RenderCommand*(*pop)(RenderCommandList*);
    void(*call)(RenderCommandList*);

    int offset, count;
    struct RenderCommand* commands;
};

struct Renderer {
    int l_gl_context_ref;
    int l_window_ref;
    SDL_Window* window_ptr;

    vec4 clear_color;

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
