local enums = {
    "GL_POINTS",
    "GL_LINES",
    "GL_LINE_LOOP",
    "GL_LINE_STRIP",
    "GL_TRIANGLES",
    "GL_TRIANGLE_STRIP",
    "GL_TRIANGLE_FAN",

    "GL_NEVER",
    "GL_LESS",
    "GL_EQUAL",
    "GL_LEQUAL",
    "GL_GREATER",
    "GL_NOTEQUAL",
    "GL_GEQUAL",
    "GL_ALWAYS",
    "GL_ZERO",
    "GL_ONE",
    "GL_SRC_COLOR",
    "GL_ONE_MINUS_SRC_COLOR",
    "GL_SRC_ALPHA",
    "GL_ONE_MINUS_SRC_ALPHA",
    "GL_DST_ALPHA",
    "GL_ONE_MINUS_DST_ALPHA",
    "GL_DST_COLOR",
    "GL_ONE_MINUS_DST_COLOR",
    "GL_SRC_ALPHA_SATURATE",
    "GL_NONE",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_FRONT_LEFT"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_FRONT_RIGHT"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_BACK_LEFT"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_BACK_RIGHT"},
    "GL_FRONT",
    "GL_BACK",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_LEFT"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_RIGHT"},
    "GL_FRONT_AND_BACK",
    "GL_NO_ERROR",
    "GL_INVALID_ENUM",
    "GL_INVALID_VALUE",
    "GL_INVALID_OPERATION",
    "GL_OUT_OF_MEMORY",
    "GL_CW",
    "GL_CCW",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_POINT_SIZE"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_POINT_SIZE_RANGE"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_POINT_SIZE_GRANULARITY"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_LINE_SMOOTH"},
    "GL_LINE_WIDTH",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_LINE_WIDTH_RANGE"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_LINE_WIDTH_GRANULARITY"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_POLYGON_MODE"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_POLYGON_SMOOTH"},
    "GL_CULL_FACE",
    "GL_CULL_FACE_MODE",
    "GL_FRONT_FACE",
    "GL_DEPTH_RANGE",
    "GL_DEPTH_TEST",
    "GL_DEPTH_WRITEMASK",
    "GL_DEPTH_CLEAR_VALUE",
    "GL_DEPTH_FUNC",
    "GL_STENCIL_TEST",
    "GL_STENCIL_CLEAR_VALUE",
    "GL_STENCIL_FUNC",
    "GL_STENCIL_VALUE_MASK",
    "GL_STENCIL_FAIL",
    "GL_STENCIL_PASS_DEPTH_FAIL",
    "GL_STENCIL_PASS_DEPTH_PASS",
    "GL_STENCIL_REF",
    "GL_STENCIL_WRITEMASK",
    "GL_VIEWPORT",
    "GL_DITHER",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_BLEND_DST"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_BLEND_SRC"},
    "GL_BLEND",

    "GL_SHADING_LANGUAGE_VERSION",

    "GL_DEPTH_BUFFER_BIT",
    "GL_STENCIL_BUFFER_BIT",
    "GL_COLOR_BUFFER_BIT",

    {iif = '!defined(OS_EMSCRIPTEN)', "GL_TEXTURE_1D"},
    "GL_TEXTURE_2D",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_TEXTURE_WIDTH"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_TEXTURE_HEIGHT"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_TEXTURE_BORDER_COLOR"},
    "GL_DONT_CARE",
    "GL_FASTEST",
    "GL_NICEST",
    "GL_BYTE",
    "GL_UNSIGNED_BYTE",
    "GL_SHORT",
    "GL_UNSIGNED_SHORT",
    "GL_INT",
    "GL_UNSIGNED_INT",
    "GL_FLOAT",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_CLEAR"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_AND"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_AND_REVERSE"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_COPY"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_AND_INVERTED"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_NOOP"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_XOR"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_OR"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_NOR"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_EQUIV"},
    "GL_INVERT",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_OR_REVERSE"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_COPY_INVERTED"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_OR_INVERTED"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_NAND"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_SET"},
    "GL_TEXTURE",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_COLOR"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_DEPTH"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_STENCIL"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_STENCIL_INDEX"},
    "GL_DEPTH_COMPONENT",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_RED"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_GREEN"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_BLUE"},
    "GL_ALPHA",
    "GL_RGB",
    "GL_RGBA",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_POINT"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_LINE"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_FILL"},
    "GL_KEEP",
    "GL_REPLACE",
    "GL_INCR",
    "GL_DECR",
    "GL_VENDOR",
    "GL_RENDERER",
    "GL_VERSION",
    "GL_EXTENSIONS",
    "GL_NEAREST",
    "GL_LINEAR",
    "GL_NEAREST_MIPMAP_NEAREST",
    "GL_LINEAR_MIPMAP_NEAREST",
    "GL_NEAREST_MIPMAP_LINEAR",
    "GL_LINEAR_MIPMAP_LINEAR",
    "GL_TEXTURE_MAG_FILTER",
    "GL_TEXTURE_MIN_FILTER",
    "GL_TEXTURE_WRAP_S",
    "GL_TEXTURE_WRAP_T",
    "GL_REPEAT",

    {iif = '!defined(OS_EMSCRIPTEN)', "GL_BGR"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_BGRA"},
    "GL_CLAMP_TO_EDGE",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_CLAMP_TO_BORDER"},
    "GL_BLEND_DST_RGB",
    "GL_BLEND_SRC_RGB",
    "GL_BLEND_DST_ALPHA",
    "GL_BLEND_SRC_ALPHA",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_POINT_FADE_THRESHOLD_SIZE"},
    "GL_DEPTH_COMPONENT16",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_DEPTH_COMPONENT24"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_DEPTH_COMPONENT32"},
    "GL_MIRRORED_REPEAT",

    "GL_BLEND_COLOR",
    "GL_BLEND_EQUATION",
    "GL_CONSTANT_COLOR",
    "GL_ONE_MINUS_CONSTANT_COLOR",
    "GL_CONSTANT_ALPHA",
    "GL_ONE_MINUS_CONSTANT_ALPHA",
    "GL_FUNC_ADD",
    "GL_FUNC_REVERSE_SUBTRACT",
    "GL_FUNC_SUBTRACT",
    "GL_MIN",
    "GL_MAX",

    "GL_ARRAY_BUFFER",
    "GL_ELEMENT_ARRAY_BUFFER",
    "GL_ARRAY_BUFFER_BINDING",
    "GL_ELEMENT_ARRAY_BUFFER_BINDING",

    "GL_STREAM_DRAW",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_STREAM_READ"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_STREAM_COPY"},
    "GL_STATIC_DRAW",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_STATIC_READ"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_STATIC_COPY"},
    "GL_DYNAMIC_DRAW",
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_DYNAMIC_READ"},
    {iif = '!defined(OS_EMSCRIPTEN)', "GL_DYNAMIC_COPY"},

    "GL_FRAGMENT_SHADER",
    "GL_VERTEX_SHADER",
    "GL_COMPILE_STATUS",
    "GL_LINK_STATUS",
}
return enums