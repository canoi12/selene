#ifndef SELENE_MINIGL_H_
#define SELENE_MINIGL_H_

#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

#if defined(OS_EMSCRIPTEN) || defined(OS_ANDROID)
#define SELENE_USE_NATIVE_GL 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#if defined(OS_WIN)
    #ifndef WINDOWS_LEAN_AND_MEAN
        #define WINDOWS_LEAN_AND_MEAN 1
    #endif
    static HMODULE s_glsym;
#else
    #include <dlfcn.h>
    static void *s_glsym;   
    #ifndef RTLD_LAZY
        #define RTLD_LAZY 0x00001
        #endif
    #ifndef RTLD_GLOBAL
        #define RTLD_GLOBAL 0x00100
    #endif
#endif
#endif

typedef unsigned int glObject;

#define GL_STENCIL_CLEAR_VALUE 0xb91
#define GL_FASTEST 0x1101
#define GL_ONE 0x1
#define GL_UNSIGNED_INT 0x1405
#define GL_EQUAL 0x202
#define GL_SRC_COLOR 0x300
#define GL_BLEND_SRC_RGB 0x80c9
#define GL_BLEND_DST_ALPHA 0x80ca
#define GL_BLEND_EQUATION 0x8009
#define GL_FILL 0x1b02
#define GL_STENCIL_INDEX 0x1901
#define GL_NONE 0x0
#define GL_ONE_MINUS_DST_COLOR 0x307
#define GL_DEPTH_COMPONENT24 0x81a6
#define GL_ALWAYS 0x207
#define GL_FRONT_LEFT 0x400
#define GL_COPY_INVERTED 0x150c
#define GL_LINK_STATUS 0x8b82
#define GL_LINE_WIDTH_GRANULARITY 0xb23
#define GL_INVALID_VALUE 0x501
#define GL_STENCIL_VALUE_MASK 0xb93
#define GL_STENCIL_FUNC 0xb92
#define GL_TEXTURE 0x1702
#define GL_OUT_OF_MEMORY 0x505
#define GL_LINES 0x1
#define GL_NEVER 0x200
#define GL_TEXTURE_2D 0xde1
#define GL_NICEST 0x1102
#define GL_FLOAT 0x1406
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_DONT_CARE 0x1100
#define GL_SRC_ALPHA_SATURATE 0x308
#define GL_TEXTURE_WIDTH 0x1000
#define GL_INCR 0x1e02
#define GL_INVERT 0x150a
#define GL_EQUIV 0x1509
#define GL_STATIC_COPY 0x88e6
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_TRIANGLES 0x4
#define GL_VERTEX_SHADER 0x8b31
#define GL_POLYGON_MODE 0xb40
#define GL_DYNAMIC_DRAW 0x88e8
#define GL_LINE 0x1b01
#define GL_FRONT_FACE 0xb46
#define GL_BACK 0x405
#define GL_OR_INVERTED 0x150d
#define GL_VENDOR 0x1f00
#define GL_LINE_LOOP 0x2
#define GL_SHORT 0x1402
#define GL_FRONT_AND_BACK 0x408
#define GL_RIGHT 0x407
#define GL_STENCIL_WRITEMASK 0xb98
#define GL_MIN 0x8007
#define GL_BLUE 0x1905
#define GL_POINT_SIZE 0xb11
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_MIRRORED_REPEAT 0x8370
#define GL_LEFT 0x406
#define GL_TRIANGLE_FAN 0x6
#define GL_ZERO 0x0
#define GL_POINTS 0x0
#define GL_GREEN 0x1904
#define GL_FRONT 0x404
#define GL_TRIANGLE_STRIP 0x5
#define GL_LINE_SMOOTH 0xb20
#define GL_STENCIL_REF 0xb97
#define GL_CULL_FACE 0xb44
#define GL_POLYGON_SMOOTH 0xb41
#define GL_BLEND_DST_RGB 0x80c8
#define GL_COMPILE_STATUS 0x8b81
#define GL_CULL_FACE_MODE 0xb45
#define GL_AND_REVERSE 0x1502
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_FRAGMENT_SHADER 0x8b30
#define GL_OR_REVERSE 0x150b
#define GL_ONE_MINUS_SRC_COLOR 0x301
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_DYNAMIC_COPY 0x88ea
#define GL_AND 0x1501
#define GL_CLEAR 0x1500
#define GL_LINE_STRIP 0x3
#define GL_CCW 0x901
#define GL_STATIC_DRAW 0x88e4
#define GL_LINEAR 0x2601
#define GL_STREAM_COPY 0x88e2
#define GL_BACK_LEFT 0x402
#define GL_CLAMP_TO_BORDER 0x812d
#define GL_STREAM_DRAW 0x88e0
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_DEPTH_COMPONENT32 0x81a7
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_BLEND 0xbe2
#define GL_MAX 0x8008
#define GL_NOTEQUAL 0x205
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_ARRAY_BUFFER 0x8892
#define GL_FUNC_SUBTRACT 0x800a
#define GL_ONE_MINUS_SRC_ALPHA 0x303
#define GL_FUNC_REVERSE_SUBTRACT 0x800b
#define GL_NOOP 0x1505
#define GL_FUNC_ADD 0x8006
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_COPY 0x1503
#define GL_KEEP 0x1e00
#define GL_ALPHA 0x1906
#define GL_SRC_ALPHA 0x302
#define GL_LESS 0x201
#define GL_UNSIGNED_BYTE 0x1401
#define GL_DEPTH_COMPONENT 0x1902
#define GL_BLEND_COLOR 0x8005
#define GL_LINE_WIDTH 0xb21
#define GL_DEPTH_COMPONENT16 0x81a5
#define GL_TEXTURE_1D 0xde0
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_REPLACE 0x1e01
#define GL_INT 0x1404
#define GL_BLEND_SRC_ALPHA 0x80cb
#define GL_STREAM_READ 0x88e1
#define GL_DEPTH_RANGE 0xb70
#define GL_NO_ERROR 0x0
#define GL_CLAMP_TO_EDGE 0x812f
#define GL_DST_ALPHA 0x304
#define GL_BGRA 0x80e1
#define GL_FRONT_RIGHT 0x401
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_DEPTH_FUNC 0xb74
#define GL_REPEAT 0x2901
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_BACK_RIGHT 0x403
#define GL_AND_INVERTED 0x1504
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_EXTENSIONS 0x1f03
#define GL_NEAREST 0x2600
#define GL_RENDERER 0x1f01
#define GL_LINE_WIDTH_RANGE 0xb22
#define GL_DEPTH 0x1801
#define GL_DECR 0x1e03
#define GL_STATIC_READ 0x88e5
#define GL_SHADING_LANGUAGE_VERSION 0x8b8c
#define GL_RGBA 0x1908
#define GL_DEPTH_WRITEMASK 0xb72
#define GL_INVALID_ENUM 0x500
#define GL_RED 0x1903
#define GL_CW 0x900
#define GL_SET 0x150f
#define GL_STENCIL_TEST 0xb90
#define GL_COLOR 0x1800
#define GL_STENCIL 0x1802
#define GL_BGR 0x80e0
#define GL_ONE_MINUS_DST_ALPHA 0x305
#define GL_NOR 0x1508
#define GL_OR 0x1507
#define GL_POINT_SIZE_GRANULARITY 0xb13
#define GL_XOR 0x1506
#define GL_GREATER 0x204
#define GL_DEPTH_BUFFER_BIT 0x100
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_DYNAMIC_READ 0x88e9
#define GL_LEQUAL 0x203
#define GL_DST_COLOR 0x306
#define GL_POINT 0x1b00
#define GL_BLEND_SRC 0xbe1
#define GL_CONSTANT_COLOR 0x8001
#define GL_BYTE 0x1400
#define GL_STENCIL_BUFFER_BIT 0x400
#define GL_VERSION 0x1f02
#define GL_UNSIGNED_SHORT 0x1403
#define GL_BLEND_DST 0xbe0
#define GL_DITHER 0xbd0
#define GL_GEQUAL 0x206
#define GL_VIEWPORT 0xba2
#define GL_STENCIL_PASS_DEPTH_PASS 0xb96
#define GL_STENCIL_PASS_DEPTH_FAIL 0xb95
#define GL_STENCIL_FAIL 0xb94
#define GL_DEPTH_CLEAR_VALUE 0xb73
#define GL_RGB 0x1907
#define GL_DEPTH_TEST 0xb71
#define GL_POINT_SIZE_RANGE 0xb12
#define GL_INVALID_OPERATION 0x502
#define GL_NAND 0x150e

typedef const char*(*GLPROCGetString)(int pname);
extern GLPROCGetString glGetString;
typedef void(*GLPROCClear)(int mask);
extern GLPROCClear glClear;
typedef void(*GLPROCClearColor)(float r, float g, float b, float a);
extern GLPROCClearColor glClearColor;
typedef void(*GLPROCEnable)(unsigned int pname);
extern GLPROCEnable glEnable;
typedef void(*GLPROCDisable)(unsigned int pname);
extern GLPROCDisable glDisable;

typedef int(*GLPROCGetUniformLocation)(unsigned int program, const char* name);
extern GLPROCGetUniformLocation glGetUniformLocation;
typedef int(*GLPROCGetAttribLocation)(unsigned int program, const char* name);
extern GLPROCGetAttribLocation glGetAttribLocation;
typedef void(*GLPROCBlendFunc)(int a, int b);
extern GLPROCBlendFunc glBlendFunc;
typedef void(*GLPROCBlendEquation)(int a);
extern GLPROCBlendEquation glBlendEquation;
typedef void(*GLPROCUniformMatrix4fv)(int loc, int count, int transpose, const float* v);
extern GLPROCUniformMatrix4fv glUniformMatrix4fv;

typedef void(*GLPROCGenVertexArrays)(int n, unsigned int* vaos);
extern GLPROCGenVertexArrays glGenVertexArrays;
typedef void(*GLPROCDeleteVertexArrays)(int n, unsigned int* vaos);
extern GLPROCDeleteVertexArrays glDeleteVertexArrays;
typedef void(*GLPROCEnableVertexAttribArray)(int attrib);
extern GLPROCEnableVertexAttribArray glEnableVertexAttribArray;
typedef void(*GLPROCDisableVertexAttribArray)(int attrib);
extern GLPROCDisableVertexAttribArray glDisableVertexAttribArray;
typedef void(*GLPROCVertexAttribPointer)(int attrib, int size, int type, int normalized, int stride, void* offset);
extern GLPROCVertexAttribPointer glVertexAttribPointer;
typedef void(*GLPROCBindVertexArray)(unsigned int vao);
extern GLPROCBindVertexArray glBindVertexArray;

typedef void(*GLPROCGenBuffers)(int n, unsigned int* buffers);
extern GLPROCGenBuffers glGenBuffers;
typedef void(*GLPROCDeleteBuffers)(int n, unsigned int* buffers);
extern GLPROCDeleteBuffers glDeleteBuffers;
typedef void(*GLPROCBindBuffer)(unsigned int target, unsigned int buffer);
extern GLPROCBindBuffer glBindBuffer;
typedef void(*GLPROCBufferData)(uint32_t target, size_t size, const void* data, uint32_t usage);
extern GLPROCBufferData glBufferData;
typedef void(*GLPROCBufferSubData)(uint32_t target, int offset, size_t size, const void* data);
extern GLPROCBufferSubData glBufferSubData;

typedef void(*GLPROCGenTextures)(int n, unsigned int* textures);
extern GLPROCGenTextures glGenTextures;
typedef void(*GLPROCDeleteTextures)(int n, unsigned int* textures);
extern GLPROCDeleteTextures glDeleteTextures;
typedef void(*GLPROCBindTexture)(unsigned int target, unsigned int texture);
extern GLPROCBindTexture glBindTexture;
typedef void(*GLPROCTexParameteri)(int pname, int param);
extern GLPROCTexParameteri glTexParameteri;
typedef void(*GLPROCTexParameterf)(int pname, float param);
extern GLPROCTexParameterf glTexParameterf;
typedef void(*GLPROCTexImage2D)(int target, int level, int internalf, int w, int h, int border, int format, int type, const void* data);
extern GLPROCTexImage2D glTexImage2D;

typedef void(*GLPROCGenTextures)(int n, unsigned int* framebuffers);
extern GLPROCGenTextures glGenTextures;
typedef void(*GLPROCDeleteFramebuffers)(int n, unsigned int* framebuffers);
extern GLPROCDeleteFramebuffers glDeleteFramebuffers;
typedef void(*GLPROCBindFramebuffer)(unsigned int target, unsigned int framebuffer);
extern GLPROCBindFramebuffer glBindFramebuffer;
typedef void(*GLPROCFramebufferFramebuffer2D)(int target, int attachment, int textarget, int texture, int level);
extern GLPROCFramebufferFramebuffer2D glFramebufferFramebuffer2D;

typedef unsigned int(*GLPROCCreateProgram)();
extern GLPROCCreateProgram glCreateProgram;
typedef void(*GLPROCDeleteProgram)(unsigned int program);
extern GLPROCDeleteProgram glDeleteProgram;
typedef void(*GLPROCUseProgram)(unsigned int program);
extern GLPROCUseProgram glUseProgram;
typedef void(*GLPROCAttachShader)(unsigned int program, unsigned int shader);
extern GLPROCAttachShader glAttachShader;
typedef void(*GLPROCLinkProgram)(unsigned int program);
extern GLPROCLinkProgram glLinkProgram;
typedef void(*GLPROCGetProgramiv)(unsigned int program, int pname, int* pout);
extern GLPROCGetProgramiv glGetProgramiv;
typedef void(*GLPROCGetProgramInfoLog)(unsigned int program, size_t maxlen, size_t* len, char* log);
extern GLPROCGetProgramInfoLog glGetProgramInfoLog;

typedef unsigned int(*GLPROCCreateShader)(unsigned int shader_type);
extern GLPROCCreateShader glCreateShader;
typedef void(*GLPROCDeleteShader)(unsigned int shader);
extern GLPROCDeleteShader glDeleteShader;
typedef void(*GLPROCShaderSource)(unsigned int shader, int n, const char** sources, const int* out);
extern GLPROCShaderSource glShaderSource;
typedef void(*GLPROCCompileShader)(unsigned int shader);
extern GLPROCCompileShader glCompileShader;
typedef void(*GLPROCGetShaderiv)(unsigned int shader, int pname, int* pout);
extern GLPROCGetShaderiv glGetShaderiv;
typedef void(*GLPROCGetShaderInfoLog)(unsigned int shader, size_t maxlen, size_t* len, char* log);
extern GLPROCGetShaderInfoLog glGetShaderInfoLog;

typedef void(*GLPROCUniform1f)(int loc, float v0);
extern GLPROCUniform1f glUniform1f;
typedef void(*GLPROCUniform1fv)(int loc, int n, float* v);
extern GLPROCUniform1fv glUniform1fv;
typedef void(*GLPROCUniform2f)(int loc, float v0, float v1);
extern GLPROCUniform2f glUniform2f;
typedef void(*GLPROCUniform2fv)(int loc, int n, float* v);
extern GLPROCUniform2fv glUniform2fv;
typedef void(*GLPROCUniform3f)(int loc, float v0, float v1, float v2);
extern GLPROCUniform3f glUniform3f;
typedef void(*GLPROCUniform3fv)(int loc, int n, float* v);
extern GLPROCUniform3fv glUniform3fv;
typedef void(*GLPROCUniform4f)(int loc, float v0, float v1, float v2, float v3);
extern GLPROCUniform4f glUniform4f;
typedef void(*GLPROCUniform4fv)(int loc, int n, float* v);
extern GLPROCUniform4fv glUniform4fv;
typedef void(*GLPROCUniform1i)(int loc, int v0);
extern GLPROCUniform1i glUniform1i;
typedef void(*GLPROCUniform1iv)(int loc, int n, int* v);
extern GLPROCUniform1iv glUniform1iv;
typedef void(*GLPROCUniform2i)(int loc, int v0, int v1);
extern GLPROCUniform2i glUniform2i;
typedef void(*GLPROCUniform2iv)(int loc, int n, int* v);
extern GLPROCUniform2iv glUniform2iv;
typedef void(*GLPROCUniform3i)(int loc, int v0, int v1, int v2);
extern GLPROCUniform3i glUniform3i;
typedef void(*GLPROCUniform3iv)(int loc, int n, int* v);
extern GLPROCUniform3iv glUniform3iv;
typedef void(*GLPROCUniform4i)(int loc, int v0, int v1, int v2, int v3);
extern GLPROCUniform4i glUniform4i;
typedef void(*GLPROCUniform4iv)(int loc, int n, int* v);
extern GLPROCUniform4iv glUniform4iv;

typedef void*(*GetProcFunction)(const char*);

int setup_minigl(GetProcFunction proc);

#endif /* SELENE_MINIGL_H_ */