#include "selene.h"
#include "lua_helper.h"

typedef struct Vertex2D Vertex2D;
struct Vertex2D {
    float x, y;
    float r, g, b, a;
    float u, v;
};

typedef struct Batch2D Batch2D;
struct Batch2D {
    Vertex2D* vertices;
    unsigned int count;
    unsigned int capacity;
};

typedef struct Renderer Renderer;
struct Renderer {
    unsigned int vao;
    unsigned int vbo;
    Batch2D batch;
};

MODULE_FUNCTION(selene, create_renderer) {
    NEW_UDATA(Renderer, r);

    r->batch.count = 0;
    r->batch.capacity = 1024;
    r->batch.vertices = malloc(r->batch.capacity * sizeof(Vertex2D));

    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, r->batch.capacity*sizeof(Vertex2D), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return 1;
}

static int s_push_vertex(Renderer* self, float x, float y, float r, float g, float b, float a, float u, float v) {
    if (self->batch.count >= self->batch.capacity) {
        self->batch.capacity *= 2;
        self->batch.vertices = realloc(self->batch.vertices, self->batch.capacity * sizeof(Vertex2D));
    }
    Vertex2D* vertex = &self->batch.vertices[self->batch.count];
    vertex->x = x;
    vertex->y = y;
    vertex->r = r;
    vertex->g = g;
    vertex->b = b;
    vertex->a = a;
    vertex->u = u;
    vertex->v = v;
    self->batch.count++;
    return 0;
}

static MODULE_FUNCTION(Renderer, push_vertex) {
    CHECK_META(Renderer);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    CHECK_NUMBER(float, r);
    CHECK_NUMBER(float, g);
    CHECK_NUMBER(float, b);
    CHECK_NUMBER(float, a);
    CHECK_NUMBER(float, u);
    CHECK_NUMBER(float, v);
    s_push_vertex(self, x, y, r, g, b, a, u, v);
    return 0;
}

static MODULE_FUNCTION(Renderer, push_point) {
    CHECK_META(Renderer);
    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);
    s_push_vertex(self, x, y, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
    return 0;
}

static MODULE_FUNCTION(Renderer, push_line) {
    CHECK_META(Renderer);
    CHECK_NUMBER(float, x1);
    CHECK_NUMBER(float, y1);
    CHECK_NUMBER(float, x2);
    CHECK_NUMBER(float, y2);
    s_push_vertex(self, x1, y1, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
    s_push_vertex(self, x2, y2, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
    return 0;
}

static MODULE_FUNCTION(Renderer, push_triangle) {
    CHECK_META(Renderer);
    CHECK_NUMBER(float, x1);
    CHECK_NUMBER(float, y1);
    CHECK_NUMBER(float, x2);
    CHECK_NUMBER(float, y2);
    CHECK_NUMBER(float, x3);
    CHECK_NUMBER(float, y3);
    s_push_vertex(self, x1, y1, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    s_push_vertex(self, x2, y2, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    s_push_vertex(self, x3, y3, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f);
    return 0;
}

MODULE_FUNCTION(Renderer, meta) {
    BEGIN_REG(reg)
        REG_FIELD(Renderer, push_vertex),
    END_REG()
    luaL_newlib(L, reg);
    return 1;
}