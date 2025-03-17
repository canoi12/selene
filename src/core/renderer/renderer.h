#include "selene.h"
#include "lua_helper.h"

#ifndef SELENE_RENDERER_H_
#define SELENE_RENDERER_H_

#ifndef SELENE_BATCH2D_PROJECTION_FAR
#define SELENE_BATCH2D_PROJECTION_FAR 1000
#endif

#define RENDERLIST_CLEAR(list)\
lua_pushcfunction(L, (list)->clear);\
lua_pushlightuserdata(L, (list));\
lua_call(L, 1, 0)

#define RENDERLIST_CALL(list)\
lua_pushcfunction(L, (list)->call);\
lua_pushlightuserdata(L, (list));\
lua_call(L, 1, 0)

#define RENDERLIST_PUSH(list, rc)\
lua_pushcfunction(L, (list)->push);\
lua_pushlightuserdata(L, (list));\
lua_pushlightuserdata(L, (rc));\
lua_call(L, 2, 0)

#define RENDERLIST_POP(list)\
lua_pushcfunction(L, (list)->pop);\
lua_pushlightuserdata(L, (list));\
lua_call(L, 1, 1)

extern const char* pixel_formats[];
extern const int pixel_formats_values[];

extern const char* texture_filters[];
extern const int texture_filters_values[];

extern const char* draw_modes[];
extern const int draw_modes_values[];

/**
 * Texture Types
 */

typedef Uint32 Texture; // Generic texture

typedef struct {
    Uint32 handle;
    int width, height;
} Texture2D;

typedef struct {
    Uint32 handle; // Handle to the cube map texture
    int size;      // Size of each face of the cube map
} TextureCube;

typedef struct {
    Texture2D texture;
    FontGlyph glyphs[256];
} Font;

/**
 * Framebuffer
 */

typedef struct {
    Uint32 handle;
} Framebuffer;

typedef struct {
    Texture2D texture;
    Uint32 fbo;
    Uint32 depth;
} Canvas;

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
 * Effect types
 */

typedef struct {
    Uint32 handle;
    // attributes
    int position_location;
    int color_location;
    int texcoord_location;
    // uniforms
    int model_view_location;
    int projection_location;
} Effect2D;

typedef struct {
    Uint32 handle;
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

    RENDER_COMMAND_ENABLE_CLIP_RECT,
    RENDER_COMMAND_DISABLE_CLIP_RECT,

    RENDER_COMMAND_SET_BLEND_MODE,

    RENDER_COMMAND_INTEGER_UNIFORM,
    RENDER_COMMAND_FLOAT_UNIFORM,
    RENDER_COMMAND_MATRIX_UNIFORM,

    RENDER_COMMAND_SET_VIEWPORT,

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
            Uint32 location;
            Sint8 c;
            Uint8 uc;
            Sint16 s;
            Uint16 us;
            Sint32 i;
            Uint32 ui;
            float f;
            double d;
            mat4 m;
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
            Uint32 start, count;
        } draw;

        struct {
            Uint32 mode;
            Uint32 start, count;
            Uint32 instance_count;
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

struct Renderer {
    int l_gl_context_ref;
    int l_window_ref;
    SDL_Window* window_ptr;
#if 0
    ClearRenderListFunc clear;
    PushRenderListFunc push;
    PopRenderListFunc pop;
    CallRenderListFunc call;
#endif
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
    int l_render_list_ref;
    RenderList* list_ptr;

    void(*present)(Renderer*, lua_State*);
#if 0
    struct RenderCommandPool* pool;
    struct RenderCommandPool root;
#endif
};

#define TEXTURE2D_CLASS LUA_META_CLASS(Texture2D)
#define FRAMEBUFFER_CLASS LUA_META_CLASS(Framebuffer)
#define CANVAS_CLASS LUA_META_CLASS(Canvas)

#define EFFECT2D_CLASS LUA_META_CLASS(Effect2D)

#define RENDER_BATCH2D_CLASS LUA_META_CLASS(RenderBatch2D)

#endif /* SELENE_RENDERER_H_ */
