local functions = {
    {
        name = 'GetString',
        rettype = 'const char*',
        args = func_args('int pname')
    },
    {
        name = 'Clear',
        rettype = 'void',
        args = func_args('int mask')
    },
    {
        name = 'ClearColor',
        rettype = 'void',
        args = func_args('float r', 'float g', 'float b', 'float a')
    },
    {
        name = 'Enable',
        rettype = 'void',
        args = func_args('unsigned int pname')
    },
    {
        name = 'Disable',
        rettype = 'void',
        args = func_args('unsigned int pname')
    },
    { name = 'break' },
    {
        name = 'GetUniformLocation',
        rettype = 'int',
        args = func_args('unsigned int program', 'const char* name')
    },
    {
        name = 'GetAttribLocation',
        rettype = 'int',
        args = func_args('unsigned int program', 'const char* name')
    },
    {
        name = 'BlendFunc',
        rettype = 'void',
        args = func_args('int a', 'int b')
    },
    {
        name = 'BlendEquation',
        rettype = 'void',
        args = func_args('int a')
    },
    {
        name = 'UniformMatrix4fv',
        rettype = 'void',
        args = func_args('int loc', 'int count', 'int transpose', 'const float* v')
    },
    { name = 'break' },
    {
        name = 'GenVertexArrays',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* vaos')
    },
    {
        name = 'DeleteVertexArrays',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* vaos')
    },
    {
        name = 'EnableVertexAttribArray',
        rettype = 'void',
        args = func_args('int attrib')
    },
    {
        name = 'DisableVertexAttribArray',
        rettype = 'void',
        args = func_args('int attrib')
    },
    {
        name = 'VertexAttribPointer',
        rettype = 'void',
        args = func_args("int attrib", "int size", "int type", "int normalized", "int stride", "void* offset")
    },
    {
        name = 'BindVertexArray',
        rettype = 'void',
        args = func_args('unsigned int vao')
    },
    { name = 'break' },
    {
        name = 'GenBuffers',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* buffers')
    },
    {
        name = 'DeleteBuffers',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* buffers')
    },
    {
        name = 'BindBuffer',
        rettype = 'void',
        args = func_args('unsigned int target', 'unsigned int buffer')
    },
    {
        name = 'BufferData',
        rettype = 'void',
        args = func_args("uint32_t target", "size_t size", "const void* data", "uint32_t usage")
    },
    {
        name = 'BufferSubData',
        rettype = 'void',
        args = func_args("uint32_t target", "int offset", "size_t size", "const void* data")
    },
    { name = 'break' },
    {
        name = 'GenTextures',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* textures')
    },
    {
        name = 'DeleteTextures',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* textures')
    },
    {
        name = 'BindTexture',
        rettype = 'void',
        args = func_args('unsigned int target', 'unsigned int texture')
    },
    {
        name = 'TexParameteri',
        rettype = 'void',
        args = func_args('int pname', 'int param')
    },
    {
        name = 'TexParameterf',
        rettype = 'void',
        args = func_args('int pname', 'float param')
    },
    {
        name = 'TexImage2D',
        rettype = 'void',
        args = func_args("int target", "int level", "int internalf", "int w", "int h", "int border", "int format", "int type", "const void* data")
    },
    { name = 'break' },
    {
        name = 'GenTextures',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* framebuffers')
    },
    {
        name = 'DeleteFramebuffers',
        rettype = 'void',
        args = func_args('int n', 'unsigned int* framebuffers')
    },
    {
        name = 'BindFramebuffer',
        rettype = 'void',
        args = func_args('unsigned int target', 'unsigned int framebuffer')
    },
    {
        name = 'FramebufferFramebuffer2D',
        rettype = 'void',
        args = func_args("int target", "int attachment", "int textarget", "int texture", "int level")
    },
    { name = 'break' },
    {
        name = 'CreateProgram',
        rettype = 'unsigned int',
        args = {}
    },
    {
        name = 'DeleteProgram',
        rettype = 'void',
        args = func_args('unsigned int program')
    },
    {
        name = 'UseProgram',
        rettype = 'void',
        args = func_args('unsigned int program')
    },
    {
        name = 'AttachShader',
        rettype = 'void',
        args = func_args('unsigned int program', 'unsigned int shader')
    },
    {
        name = 'LinkProgram',
        rettype = 'void',
        args = func_args('unsigned int program')
    },
    {
        name = 'GetProgramiv',
        rettype = 'void',
        args = func_args('unsigned int program', 'int pname', 'int* pout')
    },
    {
        name = 'GetProgramInfoLog',
        rettype = 'void',
        args = func_args('unsigned int program', 'size_t maxlen', 'size_t* len', 'char* log')
    },

    { name = 'break' },
    --[[ Shader ]]
    {
        name = 'CreateShader',
        rettype = 'unsigned int',
        args = func_args('unsigned int shader_type')
    },
    {
        name = 'DeleteShader',
        rettype = 'void',
        args = func_args('unsigned int shader')
    },
    {
        name = 'ShaderSource',
        rettype = 'void',
        args = func_args('unsigned int shader', 'int n', 'const char** sources', 'const int* out')
    },
    {
        name = 'CompileShader',
        rettype = 'void',
        args = func_args('unsigned int shader')
    },
    {
        name = 'GetShaderiv',
        rettype = 'void',
        args = func_args('unsigned int shader', 'int pname', 'int* pout')
    },
    {
        name = 'GetShaderInfoLog',
        rettype = 'void',
        args = func_args('unsigned int shader', 'size_t maxlen', 'size_t* len', 'char* log')
    },
    { name = 'break' }
}
return functions