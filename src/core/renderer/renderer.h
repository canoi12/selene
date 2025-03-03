#include "selene.h"
#include "lua_helper.h"

#ifndef SELENE_RENDERER_H_
#define SELENE_RENDERER_H_

#ifndef SELENE_BATCH2D_PROJECTION_FAR
#define SELENE_BATCH2D_PROJECTION_FAR 1000
#endif

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

    RENDER_COMMAND_SET_BUFFER,
    RENDER_COMMAND_SET_CLIP_RECT,
    RENDER_COMMAND_SET_EFFECT,
    RENDER_COMMAND_SET_TEXTURE,
    RENDER_COMMAND_SET_TARGET,

    RENDER_COMMAND_FLOAT_UNIFORM,
    RENDER_COMMAND_MATRIX_UNIFORM,

    RENDER_COMMAND_SET_VIEWPORT,

    RENDER_COMMAND_DRAW_POINT,
    RENDER_COMMAND_DRAW_LINE,

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
            float f;
            mat4 m;
        } uniform;
        struct { Uint32 mask; float color[4]; } clear;
        struct { int x, y, width, height; } clip;
        struct { Uint32 attrib; } enable;
        struct { Uint32 attrib; } disable;
        struct { Uint32 target; Uint32 handle; } buffer;

        struct { int width, height; } size;

        struct { Uint32 handle; } effect;
        struct { Uint32 slot; Uint32 target; Uint32 handle; } texture;
        struct { Uint32 target; Uint32 handle; } target;

        struct { Uint32 vao; Uint32 vbo; Uint32 ibo; } model;
        struct { Uint32 vao; Uint32 vbo; } sprite_batch;

        struct { int x, y, width, height; } viewport;

        struct {
            Uint32 vao;
            Uint32 mode;
            Uint32 start, count;
        } draw;
        struct {
            Uint32 vao;
            Uint32 mode;
            Uint32 start, count;
            Uint32 instance_count;
        } instanced;
    };
};

struct RenderCommandPool {
    int current;
    struct RenderCommand commands[512];
    struct RenderCommandPool* prev;
    struct RenderCommandPool* next;
};

typedef struct Renderer Renderer;
struct Renderer {
    int l_gl_context_ref;
    struct RenderCommandPool* pool;
    struct RenderCommandPool root;

    void(*clear)(struct Renderer*);
    void(*push)(struct Renderer*, struct RenderCommand*);
    struct RenderCommand*(*pop)(struct Renderer*);
    void(*call)(struct Renderer*);

    void(*present)(Renderer*, lua_State*);
};

#define TEXTURE2D_CLASS LUA_META_CLASS(Texture2D)
#define FRAMEBUFFER_CLASS LUA_META_CLASS(Framebuffer)
#define CANVAS_CLASS LUA_META_CLASS(Canvas)

#define BATCH2D_CLASS LUA_META_CLASS(Batch2D)
#define EFFECT2D_CLASS LUA_META_CLASS(Effect2D)

#define RENDER_BATCH2D_CLASS LUA_META_CLASS(RenderBatch2D)

#endif /* SELENE_RENDERER_H_ */
