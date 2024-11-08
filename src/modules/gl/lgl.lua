--- @meta
--- @class gl
--- @field POINTS integer
--- @field LINES integer
--- @field LINE_LOOP integer
--- @field LINE_STRIP integer
--- @field TRIANGLES integer
--- @field TRIANGLE_STRIP integer
--- @field TRIANGLE_FAN integer
--- @field NEVER integer
--- @field LESS integer
--- @field EQUAL integer
--- @field LEQUAL integer
--- @field GREATER integer
--- @field NOTEQUAL integer
--- @field GEQUAL integer
--- @field ALWAYS integer
--- @field ZERO integer
--- @field ONE integer
--- @field SRC_COLOR integer
--- @field ONE_MINUS_SRC_COLOR integer
--- @field SRC_ALPHA integer
--- @field ONE_MINUS_SRC_ALPHA integer
--- @field DST_ALPHA integer
--- @field ONE_MINUS_DST_ALPHA integer
--- @field DST_COLOR integer
--- @field ONE_MINUS_DST_COLOR integer
--- @field SRC_ALPHA_SATURATE integer
--- @field NONE integer
--- @field FRONT_LEFT integer
--- @field FRONT_RIGHT integer
--- @field BACK_LEFT integer
--- @field BACK_RIGHT integer
--- @field FRONT integer
--- @field BACK integer
--- @field LEFT integer
--- @field RIGHT integer
--- @field FRONT_AND_BACK integer
--- @field NO_ERROR integer
--- @field INVALID_ENUM integer
--- @field INVALID_VALUE integer
--- @field INVALID_OPERATION integer
--- @field OUT_OF_MEMORY integer
--- @field CW integer
--- @field CCW integer
--- @field POINT_SIZE integer
--- @field POINT_SIZE_RANGE integer
--- @field POINT_SIZE_GRANULARITY integer
--- @field LINE_SMOOTH integer
--- @field LINE_WIDTH integer
--- @field LINE_WIDTH_RANGE integer
--- @field LINE_WIDTH_GRANULARITY integer
--- @field POLYGON_MODE integer
--- @field POLYGON_SMOOTH integer
--- @field CULL_FACE integer
--- @field CULL_FACE_MODE integer
--- @field FRONT_FACE integer
--- @field DEPTH_RANGE integer
--- @field DEPTH_TEST integer
--- @field DEPTH_WRITEMASK integer
--- @field DEPTH_CLEAR_VALUE integer
--- @field DEPTH_FUNC integer
--- @field STENCIL_TEST integer
--- @field STENCIL_CLEAR_VALUE integer
--- @field STENCIL_FUNC integer
--- @field STENCIL_VALUE_MASK integer
--- @field STENCIL_FAIL integer
--- @field STENCIL_PASS_DEPTH_FAIL integer
--- @field STENCIL_PASS_DEPTH_PASS integer
--- @field STENCIL_REF integer
--- @field STENCIL_WRITEMASK integer
--- @field VIEWPORT integer
--- @field DITHER integer
--- @field BLEND_DST integer
--- @field BLEND_SRC integer
--- @field BLEND integer
--- @field SHADING_LANGUAGE_VERSION integer
--- @field DEPTH_BUFFER_BIT integer
--- @field STENCIL_BUFFER_BIT integer
--- @field COLOR_BUFFER_BIT integer
--- @field TEXTURE_1D integer
--- @field TEXTURE_2D integer
--- @field TEXTURE_WIDTH integer
--- @field TEXTURE_HEIGHT integer
--- @field TEXTURE_BORDER_COLOR integer
--- @field DONT_CARE integer
--- @field FASTEST integer
--- @field NICEST integer
--- @field BYTE integer
--- @field UNSIGNED_BYTE integer
--- @field SHORT integer
--- @field UNSIGNED_SHORT integer
--- @field INT integer
--- @field UNSIGNED_INT integer
--- @field FLOAT integer
--- @field CLEAR integer
--- @field AND integer
--- @field AND_REVERSE integer
--- @field COPY integer
--- @field AND_INVERTED integer
--- @field NOOP integer
--- @field XOR integer
--- @field OR integer
--- @field NOR integer
--- @field EQUIV integer
--- @field INVERT integer
--- @field OR_REVERSE integer
--- @field COPY_INVERTED integer
--- @field OR_INVERTED integer
--- @field NAND integer
--- @field SET integer
--- @field TEXTURE integer
--- @field COLOR integer
--- @field DEPTH integer
--- @field STENCIL integer
--- @field STENCIL_INDEX integer
--- @field DEPTH_COMPONENT integer
--- @field RED integer
--- @field GREEN integer
--- @field BLUE integer
--- @field ALPHA integer
--- @field RGB integer
--- @field RGBA integer
--- @field POINT integer
--- @field LINE integer
--- @field FILL integer
--- @field KEEP integer
--- @field REPLACE integer
--- @field INCR integer
--- @field DECR integer
--- @field VENDOR integer
--- @field RENDERER integer
--- @field VERSION integer
--- @field EXTENSIONS integer
--- @field NEAREST integer
--- @field LINEAR integer
--- @field NEAREST_MIPMAP_NEAREST integer
--- @field LINEAR_MIPMAP_NEAREST integer
--- @field NEAREST_MIPMAP_LINEAR integer
--- @field LINEAR_MIPMAP_LINEAR integer
--- @field TEXTURE_MAG_FILTER integer
--- @field TEXTURE_MIN_FILTER integer
--- @field TEXTURE_WRAP_S integer
--- @field TEXTURE_WRAP_T integer
--- @field REPEAT integer
--- @field BGR integer
--- @field BGRA integer
--- @field CLAMP_TO_EDGE integer
--- @field CLAMP_TO_BORDER integer
--- @field BLEND_DST_RGB integer
--- @field BLEND_SRC_RGB integer
--- @field BLEND_DST_ALPHA integer
--- @field BLEND_SRC_ALPHA integer
--- @field POINT_FADE_THRESHOLD_SIZE integer
--- @field DEPTH_COMPONENT16 integer
--- @field DEPTH_COMPONENT24 integer
--- @field DEPTH_COMPONENT32 integer
--- @field MIRRORED_REPEAT integer
--- @field BLEND_COLOR integer
--- @field BLEND_EQUATION integer
--- @field CONSTANT_COLOR integer
--- @field ONE_MINUS_CONSTANT_COLOR integer
--- @field CONSTANT_ALPHA integer
--- @field ONE_MINUS_CONSTANT_ALPHA integer
--- @field FUNC_ADD integer
--- @field FUNC_REVERSE_SUBTRACT integer
--- @field FUNC_SUBTRACT integer
--- @field MIN integer
--- @field MAX integer
--- @field ARRAY_BUFFER integer
--- @field ELEMENT_ARRAY_BUFFER integer
--- @field ARRAY_BUFFER_BINDING integer
--- @field ELEMENT_ARRAY_BUFFER_BINDING integer
--- @field STREAM_DRAW integer
--- @field STREAM_READ integer
--- @field STREAM_COPY integer
--- @field STATIC_DRAW integer
--- @field STATIC_READ integer
--- @field STATIC_COPY integer
--- @field DYNAMIC_DRAW integer
--- @field DYNAMIC_READ integer
--- @field DYNAMIC_COPY integer
--- @field FRAGMENT_SHADER integer
--- @field VERTEX_SHADER integer
--- @field COMPILE_STATUS integer
--- @field LINK_STATUS integer
local gl = {}

--- @return integer
function gl.create_program() end

--- @param prog integer
function gl.delete_program(prog) end

--- @param prog integer|nil
function gl.use_program(prog) end

--- @param prog integer
--- @param shader integer
function gl.attach_shader(prog, shader) end

--- @param prog integer
function gl.link_program(prog) end

--- @param prog integer
--- @param pname integer
--- @return integer
function gl.get_programiv(prog, pname) end

--- @param prog integer
--- @return string
function gl.get_program_info_log(prog) end

--- @param prog integer
--- @param name string
--- @return integer
function gl.get_uniform_location(prog, name) end

--- @param prog integer
--- @param name string
--- @return integer
function gl.get_attrib_location(prog, name) end

--- @param loc integer
--- @param count integer
--- @param transpose boolean
--- @param v lightuserdata
function gl.uniform_matrix4fv(loc, count, transpose, v) end

--- @param n integer
--- @return ...integer
function gl.gen_buffers(n) end

--- @param buffers ...integer
function gl.delete_buffers(buffers) end

--- @param target integer
--- @param buf integer|nil
function gl.bind_buffer(target, buf) end

--- @param target integer
--- @param size integer
--- @param data lightuserdata
--- @param usage integer
function gl.buffer_data(target, size, data, usage) end

--- @param target integer
--- @param start integer
--- @param size integer
--- @param data lightuserdata
function gl.buffer_sub_data(target, start, size, data) end

--- @param n integer
--- @return ...integer
function gl.gen_framebuffers(n) end

--- @param framebuffers ...integer
function gl.delete_framebuffers(framebuffers) end

--- @param target integer
--- @param fbo integer|nil
function gl.bind_framebuffer(target, fbo) end

--- @param target integer
--- @param attachment integer
--- @param textarget integer
--- @param texture integer
--- @param level integer
function gl.framebuffer_texture2d(target, attachment, textarget, texture, level) end

--- @param mask integer
function gl.clear(mask) end

--- @param r number
--- @param g number
--- @param b number
--- @param a number
function gl.clear_color(r, g, b, a) end

--- @param val integer
function gl.enable(val) end

--- @param val integer
function gl.disable(val) end

--- @param sfactor integer
--- @param dfactor integer
function gl.blend_func(sfactor, dfactor) end

--- @param eq integer
function gl.blend_equation(eq) end

--- @param mode integer
--- @param start integer
--- @param count integer
function gl.draw_arrays(mode, start, count) end

--- @param mode integer
--- @param start integer
--- @param type integer
--- @param indices integer
function gl.draw_elements(mode, start, type, indices) end

--- @param proc_func lightuserdata|nil
function gl.setup(proc_func) end

--- @param pname integer
--- @return string
function gl.get_string(pname) end

--- @param shader_type integer
--- @return integer
function gl.create_shader(shader_type) end

--- @param shader integer
function gl.delete_shader(shader) end

--- @param shader integer
--- @param source string
function gl.shader_source(shader, source) end

--- @param shader integer
function gl.compile_shader(shader) end

--- @param shader integer
--- @param pname integer
--- @return integer
function gl.get_shaderiv(shader, pname) end

--- @param shader integer
--- @return string
function gl.get_shader_info_log(shader) end

--- @param loc integer
--- @param v number
function gl.uniform1f(loc, v) end

--- @param loc integer
--- @param v0 number
--- @param v1 number
function gl.uniform2f(loc, v0, v1) end

--- @param loc integer
--- @param v0 number
--- @param v1 number
--- @param v2 number
function gl.uniform3f(loc, v0, v1, v2) end

--- @param loc integer
--- @param v0 number
--- @param v1 number
--- @param v2 number
--- @param v3 number
function gl.uniform4f(loc, v0, v1, v2, v3) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform1fv(loc, n, v) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform2fv(loc, n, v) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform3fv(loc, n, v) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform4fv(loc, n, v) end

--- @param loc integer
--- @param v integer
function gl.uniform1i(loc, v) end

--- @param loc integer
--- @param v0 integer
--- @param v1 integer
function gl.uniform2i(loc, v0, v1) end

--- @param loc integer
--- @param v0 integer
--- @param v1 integer
--- @param v2 integer
function gl.uniform3i(loc, v0, v1, v2) end

--- @param loc integer
--- @param v0 integer
--- @param v1 integer
--- @param v2 integer
--- @param v3 integer
function gl.uniform4i(loc, v0, v1, v2, v3) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform1iv(loc, n, v) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform2iv(loc, n, v) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform3iv(loc, n, v) end

--- @param loc integer
--- @param n integer
--- @param v lightuserdata
function gl.uniform4iv(loc, n, v) end

--- @param n integer
--- @return ...integer
function gl.gen_vertex_arrays(n) end

--- @param vaos ...integer
function gl.delete_vertex_arrays(vaos) end

--- @param attrib integer
function gl.enable_vertex_attrib_array(attrib) end

--- @param attrib integer
function gl.disable_vertex_attrib_array(attrib) end

--- @param attrib integer
--- @param size integer
--- @param type integer
--- @param normalized boolean
--- @param stride integer
--- @param offset integer
function gl.vertex_attrib_pointer(attrib, size, type, normalized, stride, offset) end

--- @param vao integer|nil
function gl.bind_vertex_array(vao) end

--- @param n integer
--- @return ...integer
function gl.gen_textures(n) end

--- @param textures ...integer
function gl.delete_textures(textures) end

--- @param target integer
--- @param tex integer|nil
function gl.bind_texture(target, tex) end

--- @param tex integer
--- @param pname integer
--- @param param integer
function gl.tex_parameteri(tex, pname, param) end

--- @param tex integer
--- @param pname integer
--- @param param number
function gl.tex_parameterf(tex, pname, param) end

--- @param target integer
--- @param level integer
--- @param internal integer
--- @param width integer
--- @param height integer
--- @param border integer
--- @param format integer
--- @param type integer
--- @param data lightuserdata
function gl.tex_image2d(target, level, internal, width, height, border, format, type, data) end

