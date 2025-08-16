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

SELENE_API int luaopen_filesystem(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_FILESYSTEM_H_ */
