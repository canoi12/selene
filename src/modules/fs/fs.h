#ifndef FILE_H_
#define FILE_H_

#include "selene.h"
#include "lua_helper.h"

typedef struct {
    FILE* handle;
    size_t size;
} File;

#endif /* FILE_H_ */