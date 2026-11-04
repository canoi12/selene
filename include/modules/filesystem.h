#ifndef SELENE_FILESYSTEM_H_
#define SELENE_FILESYSTEM_H_

#include "selene.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern int r_exec_path;
extern int r_root_path;
extern int r_user_path;

extern int get_virt_path(lua_State* L, const char* path);

/*
 * Read a whole file through the virtual filesystem (virtual roots, mounted
 * game archive, plain disk). Returns a NUL terminated buffer that must be
 * released with SDL_free, or NULL if the file could not be read.
 * Leaves the Lua stack untouched.
 */
SELENE_API void* selene_fs_load(lua_State* L, const char* path, size_t* out_size);

/* Mount a zip archive as the game root (root://). size 0 = until end of file */
SELENE_API int selene_fs_mount_zip(const char* path, unsigned long long offset, unsigned long long size, const char** err);
/* Mount a zip archive from memory; with take_ownership the buffer is SDL_free'd on unmount */
SELENE_API int selene_fs_mount_zip_mem(void* data, size_t size, int take_ownership);
SELENE_API void selene_fs_unmount(void);
SELENE_API int selene_fs_is_mounted(void);

/* Full path of the running executable */
SELENE_API int selene_fs_get_exe_path(char* out, size_t size);
/* Look for a zip appended to 'path' ([exe][zip]["SELENEPK"][u64 offset]) */
SELENE_API int selene_fs_fused_info(const char* path, unsigned long long* offset, unsigned long long* size);

/* Insert a function into package.searchers (package.loaders on LuaJIT) at pos */
SELENE_API void selene_fs_insert_searcher(lua_State* L, lua_CFunction fn, int pos);

SELENE_API int luaopen_filesystem(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_FILESYSTEM_H_ */
