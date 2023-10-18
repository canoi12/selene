#ifndef GL_H_
#define GL_H_

#include "selene.h"
#include "common.h"
#include "lua_helper.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef unsigned int Texture;
typedef unsigned int Framebuffer;

typedef unsigned int Program;
typedef unsigned int Shader;

typedef unsigned int Buffer;
typedef unsigned int VertexArray;

int seleneopen_gl(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif // GL_H_
