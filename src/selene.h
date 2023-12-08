#ifndef SELENE_H_
#define SELENE_H_

#define SELENE_VERSION "0.1.0"
#include "common.h"

#define CORE_DIR "core"
#define STEP_FUNC "step"
#define QUIT_FUNC "quit"

typedef struct Data Data;
struct Data {
    void* data;
    size_t size;
};

int luaopen_selene(lua_State* L);

#endif /* SELENE_H_ */
