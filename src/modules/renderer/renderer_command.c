#include "modules/renderer.h"

int renderer_init_command_pool(selene_Renderer* r, int count) {
    if (!r) return 0;
    r->command_offset = 0;
    r->command_count = count > 0 ? count : 512;
    r->command_pool = (struct RenderCommand*)malloc(sizeof(struct RenderCommand) * (size_t)r->command_count);
    return r->command_pool != NULL;
}

void renderer_push_commands(selene_Renderer* r, int count, const struct RenderCommand* cmd) {
    if (!r || !cmd || count <= 0) return;
    if (!r->command_pool || r->command_count <= 0) {
        if (!renderer_init_command_pool(r, 512)) return;
    }
    if (r->command_offset + count > r->command_count) {
        int new_count = r->command_count;
        while (r->command_offset + count > new_count) new_count *= 2;
        struct RenderCommand* pool = (struct RenderCommand*)realloc(
            r->command_pool, sizeof(struct RenderCommand) * (size_t)new_count);
        if (!pool) {
            DEBUG_ERROR("[renderer] failed to grow command pool to %d\n", new_count);
            return;
        }
        r->command_pool = pool;
        r->command_count = new_count;
    }
    memcpy(r->command_pool + r->command_offset, cmd, sizeof(*cmd) * (size_t)count);
    r->command_offset += count;
}
