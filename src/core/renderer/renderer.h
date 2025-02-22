#include "selene.h"
#include "lua_helper.h"

#ifndef SELENE_RENDERER_H_
#define SELENE_RENDERER_H_

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
    float r, g, b, a;
    float u, v;
    float nx, ny, nz;
} Vertex3D;

/**
 * Effect types
 */

typedef struct {
    Uint32 handle;
    int position_location;
    int color_location;
    int texcoord_location;
    int model_view_location;
    int projection_location;
} Effect2D;

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

typedef struct Renderer Renderer;
struct Renderer {
    int gl_context_ref;
    int clear_flags;
    int draw_mode;

    void* internal_data;

    int l_texture_ref;
    int l_effect_ref;
    int l_framebuffer_ref;

    void(*present)(Renderer*, lua_State*);
    void(*on_resize)(Renderer*, lua_State*, int w, int h);
};

#define TEXTURE2D_CLASS LUA_META_CLASS(Texture2D)
#define FRAMEBUFFER_CLASS LUA_META_CLASS(Framebuffer)
#define CANVAS_CLASS LUA_META_CLASS(Canvas)

#define BATCH2D_CLASS LUA_META_CLASS(Batch2D)
#define EFFECT2D_CLASS LUA_META_CLASS(Effect2D)

#endif /* SELENE_RENDERER_H_ */
