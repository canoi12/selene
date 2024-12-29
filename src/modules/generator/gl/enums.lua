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
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_FRONT_LEFT"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_FRONT_RIGHT"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_BACK_LEFT"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_BACK_RIGHT"},
    "GL_FRONT",
    "GL_BACK",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_LEFT"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_RIGHT"},
    "GL_FRONT_AND_BACK",
    "GL_NO_ERROR",
    "GL_INVALID_ENUM",
    "GL_INVALID_VALUE",
    "GL_INVALID_OPERATION",
    "GL_OUT_OF_MEMORY",
    "GL_CW",
    "GL_CCW",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_POINT_SIZE"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_POINT_SIZE_RANGE"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_POINT_SIZE_GRANULARITY"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_LINE_SMOOTH"},
    "GL_LINE_WIDTH",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_LINE_WIDTH_RANGE"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_LINE_WIDTH_GRANULARITY"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_POLYGON_MODE"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_POLYGON_SMOOTH"},
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
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_BLEND_DST"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_BLEND_SRC"},
    "GL_BLEND",

    "GL_SHADING_LANGUAGE_VERSION",

    "GL_DEPTH_BUFFER_BIT",
    "GL_STENCIL_BUFFER_BIT",
    "GL_COLOR_BUFFER_BIT",

    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_TEXTURE_1D"},
    "GL_TEXTURE_2D",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_TEXTURE_WIDTH"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_TEXTURE_HEIGHT"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_TEXTURE_BORDER_COLOR"},
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
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_CLEAR"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_AND"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_AND_REVERSE"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_COPY"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_AND_INVERTED"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_NOOP"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_XOR"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_OR"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_NOR"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_EQUIV"},
    "GL_INVERT",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_OR_REVERSE"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_COPY_INVERTED"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_OR_INVERTED"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_NAND"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_SET"},
    "GL_TEXTURE",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_COLOR"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_DEPTH"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_STENCIL"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_STENCIL_INDEX"},
    "GL_DEPTH_COMPONENT",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_RED"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_GREEN"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_BLUE"},
    "GL_ALPHA",
    "GL_RGB",
    "GL_RGBA",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_POINT"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_LINE"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_FILL"},
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

    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_BGR"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_BGRA"},
    "GL_CLAMP_TO_EDGE",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_CLAMP_TO_BORDER"},
    "GL_BLEND_DST_RGB",
    "GL_BLEND_SRC_RGB",
    "GL_BLEND_DST_ALPHA",
    "GL_BLEND_SRC_ALPHA",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_POINT_FADE_THRESHOLD_SIZE"},
    "GL_DEPTH_COMPONENT16",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_DEPTH_COMPONENT24"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_DEPTH_COMPONENT32"},
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
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_STREAM_READ"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_STREAM_COPY"},
    "GL_STATIC_DRAW",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_STATIC_READ"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_STATIC_COPY"},
    "GL_DYNAMIC_DRAW",
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_DYNAMIC_READ"},
    {iif = '!defined(GL_ES_VERSION_2_0)', "GL_DYNAMIC_COPY"},

    "GL_FRAGMENT_SHADER",
    "GL_VERTEX_SHADER",
    "GL_COMPILE_STATUS",
    "GL_LINK_STATUS",
}
return enums