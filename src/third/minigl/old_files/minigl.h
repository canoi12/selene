#ifndef SELENE_MINIGL_H_
#define SELENE_MINIGL_H_

#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

#ifdef SELENE_USE_NATIVE_GL
#if defined(OS_WIN)
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#endif
#endif

#if defined(_WIN32)
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

#define SELGL_POINTS 0x0000
#define SELGL_LINES 0x0001
#define SELGL_LINE_LOOP 0x0002
#define SELGL_LINE_STRIP 0x0003
#define SELGL_TRIANGLES 0x0004
#define SELGL_TRIANGLE_STRIP 0x0005
#define SELGL_TRIANGLE_FAN 0x0006

#define SELGL_NEVER 0x0200
#define SELGL_LESS 0x0201
#define SELGL_EQUAL 0x0202
#define SELGL_LEQUAL 0x0203
#define SELGL_GREATER 0x0204
#define SELGL_NOTEQUAL 0x0205
#define SELGL_GEQUAL 0x0206
#define SELGL_ALWAYS 0x0207
#define SELGL_ZERO 0
#define SELGL_ONE 1
#define SELGL_SRC_COLOR 0x0300
#define SELGL_ONE_MINUS_SRC_COLOR 0x0301
#define SELGL_SRC_ALPHA 0x0302
#define SELGL_ONE_MINUS_SRC_ALPHA 0x0303
#define SELGL_DST_ALPHA 0x0304
#define SELGL_ONE_MINUS_DST_ALPHA 0x0305
#define SELGL_DST_COLOR 0x0306
#define SELGL_ONE_MINUS_DST_COLOR 0x0307
#define SELGL_SRC_ALPHA_SATURATE 0x0308
#define SELGL_NONE 0
#define SELGL_FRONT_LEFT 0x0400
#define SELGL_FRONT_RIGHT 0x0401
#define SELGL_BACK_LEFT 0x0402
#define SELGL_BACK_RIGHT 0x0403
#define SELGL_FRONT 0x0404
#define SELGL_BACK 0x0405
#define SELGL_LEFT 0x0406
#define SELGL_RIGHT 0x0407
#define SELGL_FRONT_AND_BACK 0x0408
#define SELGL_NO_ERROR 0
#define SELGL_INVALID_ENUM 0x0500
#define SELGL_INVALID_VALUE 0x0501
#define SELGL_INVALID_OPERATION 0x0502
#define SELGL_OUT_OF_MEMORY 0x0505
#define SELGL_CW 0x0900
#define SELGL_CCW 0x0901
#define SELGL_POINT_SIZE 0x0B11
#define SELGL_POINT_SIZE_RANGE 0x0B12
#define SELGL_POINT_SIZE_GRANULARITY 0x0B13
#define SELGL_LINE_SMOOTH 0x0B20
#define SELGL_LINE_WIDTH 0x0B21
#define SELGL_LINE_WIDTH_RANGE 0x0B22
#define SELGL_LINE_WIDTH_GRANULARITY 0x0B23
#define SELGL_POLYGON_MODE 0x0B40
#define SELGL_POLYGON_SMOOTH 0x0B41
#define SELGL_CULL_FACE 0x0B44
#define SELGL_CULL_FACE_MODE 0x0B45
#define SELGL_FRONT_FACE 0x0B46
#define SELGL_DEPTH_RANGE 0x0B70
#define SELGL_DEPTH_TEST 0x0B71
#define SELGL_DEPTH_WRITEMASK 0x0B72
#define SELGL_DEPTH_CLEAR_VALUE 0x0B73
#define SELGL_DEPTH_FUNC 0x0B74
#define SELGL_STENCIL_TEST 0x0B90
#define SELGL_STENCIL_CLEAR_VALUE 0x0B91
#define SELGL_STENCIL_FUNC 0x0B92
#define SELGL_STENCIL_VALUE_MASK 0x0B93
#define SELGL_STENCIL_FAIL 0x0B94
#define SELGL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define SELGL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define SELGL_STENCIL_REF 0x0B97
#define SELGL_STENCIL_WRITEMASK 0x0B98
#define SELGL_VIEWPORT 0x0BA2
#define SELGL_DITHER 0x0BD0
#define SELGL_BLEND_DST 0x0BE0
#define SELGL_BLEND_SRC 0x0BE1
#define SELGL_BLEND 0x0BE2

#define SELGL_VERSION 0x1F02    
#define SELGL_SHADING_LANGUAGE_VERSION 0x8B8C

#define SELGL_DEPTH_BUFFER_BIT 0x00000100
#define SELGL_STENCIL_BUFFER_BIT 0x00000400
#define SELGL_COLOR_BUFFER_BIT 0x00004000

#define SELGL_TEXTURE_1D 0x0DE0
#define SELGL_TEXTURE_2D 0x0DE1
#define SELGL_TEXTURE_WIDTH 0x1000
#define SELGL_TEXTURE_HEIGHT 0x1001
#define SELGL_TEXTURE_BORDER_COLOR 0x1004
#define SELGL_DONT_CARE 0x1100
#define SELGL_FASTEST 0x1101
#define SELGL_NICEST 0x1102
#define SELGL_BYTE 0x1400
#define SELGL_UNSIGNED_BYTE 0x1401
#define SELGL_SHORT 0x1402
#define SELGL_UNSIGNED_SHORT 0x1403
#define SELGL_INT 0x1404
#define SELGL_UNSIGNED_INT 0x1405
#define SELGL_FLOAT 0x1406
#define SELGL_CLEAR 0x1500
#define SELGL_AND 0x1501
#define SELGL_AND_REVERSE 0x1502
#define SELGL_COPY 0x1503
#define SELGL_AND_INVERTED 0x1504
#define SELGL_NOOP 0x1505
#define SELGL_XOR 0x1506
#define SELGL_OR 0x1507
#define SELGL_NOR 0x1508
#define SELGL_EQUIV 0x1509
#define SELGL_INVERT 0x150A
#define SELGL_OR_REVERSE 0x150B
#define SELGL_COPY_INVERTED 0x150C
#define SELGL_OR_INVERTED 0x150D
#define SELGL_NAND 0x150E
#define SELGL_SET 0x150F
#define SELGL_TEXTURE 0x1702
#define SELGL_COLOR 0x1800
#define SELGL_DEPTH 0x1801
#define SELGL_STENCIL 0x1802
#define SELGL_STENCIL_INDEX 0x1901
#define SELGL_DEPTH_COMPONENT 0x1902
#define SELGL_RED 0x1903
#define SELGL_GREEN 0x1904
#define SELGL_BLUE 0x1905
#define SELGL_ALPHA 0x1906
#define SELGL_RGB 0x1907
#define SELGL_RGBA 0x1908
#define SELGL_POINT 0x1B00
#define SELGL_LINE 0x1B01
#define SELGL_FILL 0x1B02
#define SELGL_KEEP 0x1E00
#define SELGL_REPLACE 0x1E01
#define SELGL_INCR 0x1E02
#define SELGL_DECR 0x1E03
#define SELGL_VENDOR 0x1F00
#define SELGL_RENDERER 0x1F01
#define SELGL_VERSION 0x1F02
#define SELGL_EXTENSIONS 0x1F03
#define SELGL_NEAREST 0x2600
#define SELGL_LINEAR 0x2601
#define SELGL_NEAREST_MIPMAP_NEAREST 0x2700
#define SELGL_LINEAR_MIPMAP_NEAREST 0x2701
#define SELGL_NEAREST_MIPMAP_LINEAR 0x2702
#define SELGL_LINEAR_MIPMAP_LINEAR 0x2703
#define SELGL_TEXTURE_MAG_FILTER 0x2800
#define SELGL_TEXTURE_MIN_FILTER 0x2801
#define SELGL_TEXTURE_WRAP_S 0x2802
#define SELGL_TEXTURE_WRAP_T 0x2803
#define SELGL_REPEAT 0x2901

#define SELGL_BGR 0x80E0
#define SELGL_BGRA 0x80E1
#define SELGL_CLAMP_TO_EDGE 0x812F
#define SELGL_CLAMP_TO_BORDER 0x812D
#define SELGL_BLEND_DST_RGB 0x80C8
#define SELGL_BLEND_SRC_RGB 0x80C9
#define SELGL_BLEND_DST_ALPHA 0x80CA
#define SELGL_BLEND_SRC_ALPHA 0x80CB
#define SELGL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define SELGL_DEPTH_COMPONENT16 0x81A5
#define SELGL_DEPTH_COMPONENT24 0x81A6
#define SELGL_DEPTH_COMPONENT32 0x81A7
#define SELGL_MIRRORED_REPEAT 0x8370

#define SELGL_BLEND_COLOR 0x8005
#define SELGL_BLEND_EQUATION 0x8009
#define SELGL_CONSTANT_COLOR 0x8001
#define SELGL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define SELGL_CONSTANT_ALPHA 0x8003
#define SELGL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define SELGL_FUNC_ADD 0x8006
#define SELGL_FUNC_REVERSE_SUBTRACT 0x800B
#define SELGL_FUNC_SUBTRACT 0x800A
#define SELGL_MIN 0x8007
#define SELGL_MAX 0x8008

#define SELGL_ARRAY_BUFFER 0x8892
#define SELGL_ELEMENT_ARRAY_BUFFER 0x8893
#define SELGL_ARRAY_BUFFER_BINDING 0x8894
#define SELGL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895

#define SELGL_STREAM_DRAW 0x88E0
#define SELGL_STREAM_READ 0x88E1
#define SELGL_STREAM_COPY 0x88E2
#define SELGL_STATIC_DRAW 0x88E4
#define SELGL_STATIC_READ 0x88E5
#define SELGL_STATIC_COPY 0x88E6
#define SELGL_DYNAMIC_DRAW 0x88E8
#define SELGL_DYNAMIC_READ 0x88E9
#define SELGL_DYNAMIC_COPY 0x88EA

#define SELGL_FRAGMENT_SHADER 0x8B30
#define SELGL_VERTEX_SHADER 0x8B31
#define SELGL_COMPILE_STATUS 0x8B81
#define SELGL_LINK_STATUS 0x8B82

#define SEL_FRAMEBUFFER_BINDING 0x8CA6
#define SEL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define SEL_RENDERBUFFER_BINDING 0x8CA7
#define SEL_READ_FRAMEBUFFER 0x8CA8
#define SEL_DRAW_FRAMEBUFFER 0x8CA9
#define SEL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define SEL_RENDERBUFFER_SAMPLES 0x8CAB
#define SEL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define SEL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define SEL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define SEL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define SEL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define SEL_FRAMEBUFFER_COMPLETE 0x8CD5
#define SEL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define SEL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define SEL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define SEL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define SEL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#define SEL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define SEL_COLOR_ATTACHMENT0 0x8CE0
#define SEL_COLOR_ATTACHMENT1 0x8CE1
#define SEL_COLOR_ATTACHMENT2 0x8CE2
#define SEL_COLOR_ATTACHMENT3 0x8CE3
#define SEL_COLOR_ATTACHMENT4 0x8CE4
#define SEL_COLOR_ATTACHMENT5 0x8CE5
#define SEL_COLOR_ATTACHMENT6 0x8CE6
#define SEL_COLOR_ATTACHMENT7 0x8CE7
#define SEL_COLOR_ATTACHMENT8 0x8CE8
#define SEL_COLOR_ATTACHMENT9 0x8CE9
#define SEL_COLOR_ATTACHMENT10 0x8CEA
#define SEL_COLOR_ATTACHMENT11 0x8CEB
#define SEL_COLOR_ATTACHMENT12 0x8CEC
#define SEL_COLOR_ATTACHMENT13 0x8CED
#define SEL_COLOR_ATTACHMENT14 0x8CEE
#define SEL_COLOR_ATTACHMENT15 0x8CEF
#define SEL_COLOR_ATTACHMENT16 0x8CF0
#define SEL_COLOR_ATTACHMENT17 0x8CF1
#define SEL_COLOR_ATTACHMENT18 0x8CF2
#define SEL_COLOR_ATTACHMENT19 0x8CF3
#define SEL_COLOR_ATTACHMENT20 0x8CF4
#define SEL_COLOR_ATTACHMENT21 0x8CF5
#define SEL_COLOR_ATTACHMENT22 0x8CF6
#define SEL_COLOR_ATTACHMENT23 0x8CF7
#define SEL_COLOR_ATTACHMENT24 0x8CF8
#define SEL_COLOR_ATTACHMENT25 0x8CF9
#define SEL_COLOR_ATTACHMENT26 0x8CFA
#define SEL_COLOR_ATTACHMENT27 0x8CFB
#define SEL_COLOR_ATTACHMENT28 0x8CFC
#define SEL_COLOR_ATTACHMENT29 0x8CFD
#define SEL_COLOR_ATTACHMENT30 0x8CFE
#define SEL_COLOR_ATTACHMENT31 0x8CFF
#define SEL_DEPTH_ATTACHMENT 0x8D00
#define SEL_STENCIL_ATTACHMENT 0x8D20
#define SEL_FRAMEBUFFER 0x8D40

typedef unsigned int glObject;

typedef const char*(*selGetString)(int);
extern selGetString sel_GetString;

typedef void(*selParameteri)(glObject, int pname, int param);
typedef void(*selParameterf)(glObject, int pname, float param);

typedef void(*selFuncIntParam)(int);
typedef void(*selFuncInt2Param)(int, int);
typedef void(*selFuncInt3Param)(int, int, int);
typedef void(*selFuncInt4Param)(int, int, int, int);
extern selFuncIntParam sel_Clear;

typedef void(*selFuncFloat1Param)(float);
typedef void(*selFuncFloat2Param)(float, float);
typedef void(*selFuncFloat3Param)(float, float, float);
typedef void(*selFuncFloat4Param)(float, float, float, float);
extern selFuncFloat4Param sel_ClearColor;

typedef void(*selEnable)(int);
typedef void(*selDisable)(int);
extern selEnable sel_Enable;
extern selDisable sel_Disable;

typedef void(*selDrawArrays)(int mode, int first, size_t count);
typedef void(*selDrawElements)(int mode, size_t count, int type, const void* indices);
extern selDrawArrays sel_DrawArrays;
extern selDrawElements sel_DrawElements;

typedef int(*selGetLocation)(glObject, const char*);
extern selGetLocation sel_GetUniformLocation;
extern selGetLocation sel_GetAttribLocation;

#define DECLARE_UNIFORM(type, suffix)\
typedef void(*selUniform1##suffix)(int loc, type);\
typedef void(*selUniform2##suffix)(int loc, type, type);\
typedef void(*selUniform3##suffix)(int loc, type, type, type);\
typedef void(*selUniform4##suffix)(int loc, type, type, type, type);\
typedef void(*selUniform##suffix##v)(int loc, int n, type*);\
extern selUniform1##suffix sel_Uniform1##suffix;\
extern selUniform2##suffix sel_Uniform2##suffix;\
extern selUniform3##suffix sel_Uniform3##suffix;\
extern selUniform4##suffix sel_Uniform4##suffix;\
extern selUniform##suffix##v sel_Uniform1##suffix##v;\
extern selUniform##suffix##v sel_Uniform2##suffix##v;\
extern selUniform##suffix##v sel_Uniform3##suffix##v;\
extern selUniform##suffix##v sel_Uniform4##suffix##v;

DECLARE_UNIFORM(float, f)
DECLARE_UNIFORM(int, i)

typedef void(*selUniformMatrix4fv)(int loc, int count, int transp, const float* v);

typedef void(*selBindObject)(glObject);
typedef void(*selBindObjectTarget)(int, glObject);

typedef glObject(*selGenObjects)(int, glObject*);
typedef glObject(*selCreateProgram)(void);
typedef glObject(*selCreateShader)(int);

typedef void(*selDeleteObjects)(int, glObject*);
typedef void(*selDeleteObject)(glObject);

#define DEFINE_GLOBJECT(name, bind)\
extern selGenObjects sel_Gen##name##s;\
extern bind sel_Bind##name;\
extern selDeleteObjects sel_Delete##name##s

#define DECLARE_GLOBJECT(name, bind)\
selGenObjects sel_Gen##name##s = NULL;\
bind sel_Bind##name = NULL;\
selDeleteObjects sel_Delete##name##s = NULL

extern selFuncInt2Param sel_BlendFunc;
extern selFuncIntParam sel_BlendEquation;

/*=========================================*
 * Vertex Array
 *=========================================*/

DEFINE_GLOBJECT(VertexArray, selBindObject);

typedef void(*selEnableVertexAttribArray)(int);
typedef void(*selDisableVertexAttribArray)(int);
typedef void(*selVertexAttribPointer)(int attrib, int size, int type, int normalized, int stride, void* offset);

extern selEnableVertexAttribArray sel_EnableVertexAttribArray;
extern selDisableVertexAttribArray sel_DisableVertexAttribArray;
extern selVertexAttribPointer sel_VertexAttribPointer;

/*=========================================*
 * Buffer
 *=========================================*/

DEFINE_GLOBJECT(Buffer, selBindObjectTarget);

typedef void(*selBufferData)(uint32_t target, size_t size, const void* data, uint32_t usage);
extern selBufferData sel_BufferData;

typedef void(*selBufferSubData)(uint32_t target, int offset, size_t size, const void* data);
extern selBufferSubData sel_BufferSubData;

/*=========================================*
 * Texture
 *=========================================*/

DEFINE_GLOBJECT(Texture, selBindObjectTarget);

typedef void(*selTexImage2D)(int target, int level, int internalf, int w, int h, int border, int format, int type, const void* data);
extern selTexImage2D sel_TexImage2D;

extern selParameteri sel_TexParameteri;
extern selParameterf sel_TexParameterf;

/*=========================================*
 * Framebuffer
 *=========================================*/

DEFINE_GLOBJECT(Framebuffer, selBindObjectTarget);

typedef void(*selFramebufferTexture2D)(int target, int attachment, int textarget, int texture, int level);
extern selFramebufferTexture2D sel_FramebufferTexture2D;

/*=========================================*
 * Shader and Program
 *=========================================*/

extern selCreateProgram sel_CreateProgram;
extern selBindObject sel_UseProgram;
extern selDeleteObject sel_DeleteProgram;
extern selCreateShader sel_CreateShader;
extern selDeleteObject sel_DeleteShader;

typedef void(*selShaderSource)(glObject, int, const char**, const int*);
typedef void(*selCompileShader)(glObject);
typedef void(*selGetObjectiv)(glObject, int, int*);

typedef void(*selAttachShader)(glObject, glObject);
typedef void(*selLinkProgram)(glObject);

extern selShaderSource sel_ShaderSource;
extern selCompileShader sel_CompileShader;
extern selGetObjectiv sel_GetShaderiv;

extern selAttachShader sel_AttachShader;
extern selLinkProgram sel_LinkProgram;
extern selGetObjectiv sel_GetProgramiv;

typedef void(*selGetInfoLog)(glObject, size_t max_len, size_t* len, char* log);

extern selGetInfoLog sel_GetShaderInfoLog;
extern selGetInfoLog sel_GetProgramInfoLog;

int luaopen_minigl(lua_State* L);

#endif /* SELENE_MINIGL_H_ */