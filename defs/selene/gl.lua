--- @meta

---------------- Buffer

--- @class selene.gl.Buffer
local Buffer = {}

--- Creates a new OpenGL Buffer
--- @return selene.gl.Buffer
function Buffer.create() end

--- Bind buffer to the target
--- @param target integer
--- @param buffer selene.gl.Buffer | nil
function Buffer.bind(target, buffer) end

--- Alloc memory for buffer in the GPU
--- @param target integer
--- @param size integer
--- @param usage integer
--- @param data selene.Data | nil
function Buffer.data(target, size, usage, data) end

--- Set part of the buffer data in the GPU
--- @param target integer
--- @param start integer
--- @param size integer
--- @param data selene.Data | nil
function Buffer.subData(target, start, size, data) end


--- Destroy buffer
function Buffer:destroy() end

---------------- Framebuffer

--- @class selene.gl.Framebuffer
local Framebuffer = {}

--- Creates a new OpenGL Framebuffer
--- @return selene.gl.Framebuffer
function Framebuffer.create() end

--- Bind framebuffer to the target
--- @param target integer
--- @param buffer selene.gl.Framebuffer | nil
function Framebuffer.bind(target, buffer) end

--- Attach texture to framebuffer
--- @param target integer
--- @param attachment integer
--- @param tex_target integer
--- @param texture selene.gl.Texture
function Framebuffer.attachTexture2D(target, attachment, tex_target, texture) end

--- Destroy framebuffer
function Framebuffer:destroy() end

---------------- Program

--- @class selene.gl.Program
local Program = {}

--- Creates a new program
--- @return selene.gl.Program
function Program.create() end

--- Use program
--- @param prog selene.gl.Program | nil
function Program.use(prog) end

--- Destroy program
function Program:destroy() end

--- Attach shader to the program
--- @param ... selene.gl.Shader
function Program:attachShader(...) end

--- Link current program
function Program:link() end

--- Get the attribute location of the program by name
--- @param name string
--- @return integer
function Program:getAttribLocation(name) end

--- Get the uniform location of the program by name
--- @param name string
--- @return integer
function Program:getUniformLocation(name) end

---------------- Shader

--- @class selene.gl.Shader
local Shader = {}

--- Creates a new shader
--- @return selene.gl.Shader
function Shader.create() end

--- Destroy shader
function Shader:destroy() end

--- Set shader current source
--- @param src string
function Shader:source(src) end

--- Compile shader
function Shader:compile() end

---------------- Texture

--- @class selene.gl.Texture
local Texture = {}

--- Creates a new OpenGL Texture
--- @return selene.gl.Texture
function Texture.create() end

--- Bind texture to the target
--- @param target integer
--- @param texture selene.gl.Texture | nil
function Texture.bind(target, texture) end

--- Alloc memory for buffer in the GPU
--- @param target integer
--- @param internal integer
--- @param width integer
--- @param height integer
--- @param format integer
--- @param type_ integer
--- @param data selene.Data | nil
function Texture.image2D(target, internal, width, height, format, type_, data) end

--- Set part of the buffer data in the GPU
--- @param target integer
--- @param xoffset integer
--- @param yoffset integer
--- @param width integer
--- @param height integer
--- @param format integer
--- @param type_ integer
--- @param data selene.Data | nil
function Texture.subImage2D(target, xoffset, yoffset, width, height, format, type_, data) end

--- Set current parameter
--- @param target integer
--- @param pname integer
--- @param param integer
function Texture.parameteri(target, pname, param) end

--- Destroy texture
function Texture:destroy() end

---------------- VertexArray

--- @class selene.gl.VertexArray
local VertexArray = {}

function VertexArray.unbind() end
function VertexArray.enable(attrib) end
function VertexArray.disable(attrib) end
function VertexArray.attribPointer(attrib, size, type_, normalized, stride, ptr) end

function VertexArray:bind() end
function VertexArray:destroy() end

---------------- OpenGL

--- @class selene.gl
--- @field COLOR_BUFFER_BIT integer
--- @field DEPTH_BUFFER_BIT integer
--- @field STENCIL_BUFFER_BIT integer
--- @field BYTE integer
--- @field UNSIGNED_BYTE integer
--- @field SHORT integer
--- @field UNSIGNED_SHORT integer
--- @field INT integer
--- @field UNSIGNED_INT integer
--- @field FLOAT integer
--- @field DOUBLE integer
--- @field DEPTH_TEST integer
--- @field STENCIL_TEST integer
--- @field SCISSOR_TEST integer
--- @field BLEND integer
--- @field CULL_FACE integer
--- @field RED integer
--- @field RG integer
--- @field BGR integer
--- @field BGRA integer
--- @field RGB integer
--- @field RGBA integer
--- @field DEPTH_COMPONENT integer
--- @field DEPTH_COMPONENT16 integer
--- @field DEPTH_COMPONENT24 integer
--- @field DEPTH_COMPONENT32 integer
--- @field DEPTH_COMPONENT32F integer
--- @field DEPTH32F_STENCIL8 integer
--- @field DEPTH24_STENCIL8 integer
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
--- @field CONSTANT_COLOR integer
--- @field ONE_MINUS_CONSTANT_COLOR integer
--- @field CONSTANT_ALPHA integer
--- @field ONE_MINUS_CONSTANT_ALPHA integer
--- @field POINTS integer
--- @field LINES integer
--- @field TRIANGLES integer
--- @field TEXTURE_2D integer
--- @field TEXTURE_CUBE_MAP integer
--- @field TEXTURE_CUBE_MAP_NEGATIVE_X integer
--- @field TEXTURE_CUBE_MAP_POSITIVE_X integer
--- @field TEXTURE_CUBE_MAP_NEGATIVE_Y integer
--- @field TEXTURE_CUBE_MAP_POSITIVE_Y integer
--- @field TEXTURE_CUBE_MAP_NEGATIVE_Z integer
--- @field TEXTURE_CUBE_MAP_POSITIVE_Z integer
--- @field TEXTURE_MIN_FILTER integer
--- @field TEXTURE_MAG_FILTER integer
--- @field TEXTURE_WRAP_S integer
--- @field TEXTURE_WRAP_T integer
--- @field NEAREST integer
--- @field LINEAR integer
--- @field REPEAT integer
--- @field CLAMP_TO_EDGE integer
--- @field CLAMP_TO_BORDER integer
--- @field FRAMEBUFFER integer
--- @field DRAW_FRAMEBUFFER integer
--- @field READ_FRAMEBUFFER integer
--- @field DEPTH_STENCIL_ATTACHMENT integer
--- @field COLOR_ATTACHMENT0 integer
--- @field DEPTH_ATTACHMENT integer
--- @field STENCIL_ATTACHMENT integer
--- @field ARRAY_BUFFER integer
--- @field ELEMENT_ARRAY_BUFFER integer
--- @field UNIFORM_BUFFER integer
--- @field DYNAMIC_READ integer
--- @field DYNAMIC_COPY integer
--- @field STATIC_READ integer
--- @field STATIC_COPY integer
--- @field STREAM_READ integer
--- @field STREAM_COPY integer
--- @field DYNAMIC_DRAW integer
--- @field STATIC_DRAW integer
--- @field STREAM_DRAW integer
--- @field FRAGMENT_SHADER integer
--- @field VERTEX_SHADER integer
--- @field GEOMETRY_SHADER integer
local gl = {}

function gl.loadGlad(proc) end

---Set OpenGL viewport
---@param x integer
---@param y integer
---@param w integer
---@param h integer
function gl.viewport(x, y, w, h) end