#ifndef SELENE_FILESYSTEM_H_
#define SELENE_FILESYSTEM_H_

#include "selene.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern int r_exec_path;
extern int r_dir_path;
extern int r_user_path;

SELENE_API int luaopen_filesystem(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_FILESYSTEM_H_ */
