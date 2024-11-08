#include "minigl.h"

static void* s_get_proc(const char* name);
static char s_load_gl(void);
static void s_setup_gl(void);
static void s_close_gl(void);

#if !defined(__APPLE__) && !defined(__HAIKU__)
void* (*s_proc_address)(const char*);
#endif

int setup_minigl(GetProcFunction proc) {
    s_proc_address = proc ? proc : NULL;
    if (s_load_gl()) {
        s_setup_gl();
        s_close_gl();
        return 0;
    }
    return 1;
}












char s_load_gl(void) {
#if defined(_WIN32)
    s_glsym = LoadLibraryW(L"opengl32.dll");
    if (s_glsym == NULL) {
        LPVOID lpMsgBuf;
        DWORD err = GetLastError();
        int s = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        char* str = (char*)lpMsgBuf;
        fprintf(stderr, "failed to load opengl32.dll: (%d) %s\n", err, str);
        LocalFree(lpMsgBuf);
        return 0;
    }
    if (!s_proc_address)
        s_proc_address = (void*(*)(const char*))GetProcAddress(s_glsym, "wglGetProcAddress");
    fprintf(stdout, "loaded OpenGL32.dll\n");
    return 1;
#else
#if defined(USE_GL_LIB)
    const char* names[] = {NULL};
    return 1;
#elif defined(__APPLE__)
    const char *names[] = {
        "../Frameworks/OpenGL.framework/OpenGL",
        "/Library/Frameworks/OpenGL.framework/Opengl",
        "/System/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL",
        NULL,
    };
#else
    const char *names[] = {
        "libGL.so.1",
        "libGL.so",
        NULL,
    };
#endif
    int index;
    for (index = 0; names[index] != NULL; ++index) {
        s_glsym = dlopen(names[index], RTLD_LAZY | RTLD_GLOBAL);
        if (s_glsym != NULL) {
#if defined(__APPLE__) || defined(__HAIKU__)
            return 1;
#else
            if (!s_proc_address)
                s_proc_address = (void*(*)(const char*))dlsym(s_glsym, "glXGetProcAddress");
            return s_proc_address != NULL;
#endif
            break;
        }
    }
#endif
    return 0;
}

void s_setup_gl() {
#if !defined(USE_GL_LIB)
    glGetString = s_get_proc("glGetString");
#else
    glGetString = (GLPROCGetString)glGetString;
#endif
    const char *version = (const char*)glGetString(GL_VERSION);
    const char *glsl = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (version == NULL) {
        fprintf(stderr, "failed to get OpenGL version\n");
    }
    const char *prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        NULL,
    };

    fprintf(stderr, "OpenGL: %s\n", version);
    fprintf(stderr, "OpenGL shading language: %s\n", glsl);
#if defined(USE_GL_LIB)
    return;
#endif
    /*for (int i = 0; _procs[i].func != NULL; i++) {
        void* proc = s_get_proc(_procs[i].names[0]);
        // fprintf(stdout, "loading: %s, %p\n", _procs[i].names[0], proc);
        *(_procs[i].func) = proc;
    }*/
    glGetString = (GLPROCGetString)s_get_proc("GetString");
    glClear = (GLPROCClear)s_get_proc("Clear");
    glClearColor = (GLPROCClearColor)s_get_proc("ClearColor");
    glEnable = (GLPROCEnable)s_get_proc("Enable");
    glDisable = (GLPROCDisable)s_get_proc("Disable");
    glGetUniformLocation = (GLPROCGetUniformLocation)s_get_proc("GetUniformLocation");
    glGetAttribLocation = (GLPROCGetAttribLocation)s_get_proc("GetAttribLocation");
    glBlendFunc = (GLPROCBlendFunc)s_get_proc("BlendFunc");
    glBlendEquation = (GLPROCBlendEquation)s_get_proc("BlendEquation");
    glUniformMatrix4fv = (GLPROCUniformMatrix4fv)s_get_proc("UniformMatrix4fv");
    glGenVertexArrays = (GLPROCGenVertexArrays)s_get_proc("GenVertexArrays");
    glDeleteVertexArrays = (GLPROCDeleteVertexArrays)s_get_proc("DeleteVertexArrays");
    glEnableVertexAttribArray = (GLPROCEnableVertexAttribArray)s_get_proc("EnableVertexAttribArray");
    glDisableVertexAttribArray = (GLPROCDisableVertexAttribArray)s_get_proc("DisableVertexAttribArray");
    glVertexAttribPointer = (GLPROCVertexAttribPointer)s_get_proc("VertexAttribPointer");
    glBindVertexArray = (GLPROCBindVertexArray)s_get_proc("BindVertexArray");
    glGenBuffers = (GLPROCGenBuffers)s_get_proc("GenBuffers");
    glDeleteBuffers = (GLPROCDeleteBuffers)s_get_proc("DeleteBuffers");
    glBindBuffer = (GLPROCBindBuffer)s_get_proc("BindBuffer");
    glBufferData = (GLPROCBufferData)s_get_proc("BufferData");
    glBufferSubData = (GLPROCBufferSubData)s_get_proc("BufferSubData");
    glGenTextures = (GLPROCGenTextures)s_get_proc("GenTextures");
    glDeleteTextures = (GLPROCDeleteTextures)s_get_proc("DeleteTextures");
    glBindTexture = (GLPROCBindTexture)s_get_proc("BindTexture");
    glTexParameteri = (GLPROCTexParameteri)s_get_proc("TexParameteri");
    glTexParameterf = (GLPROCTexParameterf)s_get_proc("TexParameterf");
    glTexImage2D = (GLPROCTexImage2D)s_get_proc("TexImage2D");
    glGenTextures = (GLPROCGenTextures)s_get_proc("GenTextures");
    glDeleteFramebuffers = (GLPROCDeleteFramebuffers)s_get_proc("DeleteFramebuffers");
    glBindFramebuffer = (GLPROCBindFramebuffer)s_get_proc("BindFramebuffer");
    glFramebufferFramebuffer2D = (GLPROCFramebufferFramebuffer2D)s_get_proc("FramebufferFramebuffer2D");
    glCreateProgram = (GLPROCCreateProgram)s_get_proc("CreateProgram");
    glDeleteProgram = (GLPROCDeleteProgram)s_get_proc("DeleteProgram");
    glUseProgram = (GLPROCUseProgram)s_get_proc("UseProgram");
    glAttachShader = (GLPROCAttachShader)s_get_proc("AttachShader");
    glLinkProgram = (GLPROCLinkProgram)s_get_proc("LinkProgram");
    glGetProgramiv = (GLPROCGetProgramiv)s_get_proc("GetProgramiv");
    glGetProgramInfoLog = (GLPROCGetProgramInfoLog)s_get_proc("GetProgramInfoLog");
    glCreateShader = (GLPROCCreateShader)s_get_proc("CreateShader");
    glDeleteShader = (GLPROCDeleteShader)s_get_proc("DeleteShader");
    glShaderSource = (GLPROCShaderSource)s_get_proc("ShaderSource");
    glCompileShader = (GLPROCCompileShader)s_get_proc("CompileShader");
    glGetShaderiv = (GLPROCGetShaderiv)s_get_proc("GetShaderiv");
    glGetShaderInfoLog = (GLPROCGetShaderInfoLog)s_get_proc("GetShaderInfoLog");
    glUniform1f = (GLPROCUniform1f)s_get_proc("Uniform1f");
    glUniform1fv = (GLPROCUniform1fv)s_get_proc("Uniform1fv");
    glUniform2f = (GLPROCUniform2f)s_get_proc("Uniform2f");
    glUniform2fv = (GLPROCUniform2fv)s_get_proc("Uniform2fv");
    glUniform3f = (GLPROCUniform3f)s_get_proc("Uniform3f");
    glUniform3fv = (GLPROCUniform3fv)s_get_proc("Uniform3fv");
    glUniform4f = (GLPROCUniform4f)s_get_proc("Uniform4f");
    glUniform4fv = (GLPROCUniform4fv)s_get_proc("Uniform4fv");
    glUniform1i = (GLPROCUniform1i)s_get_proc("Uniform1i");
    glUniform1iv = (GLPROCUniform1iv)s_get_proc("Uniform1iv");
    glUniform2i = (GLPROCUniform2i)s_get_proc("Uniform2i");
    glUniform2iv = (GLPROCUniform2iv)s_get_proc("Uniform2iv");
    glUniform3i = (GLPROCUniform3i)s_get_proc("Uniform3i");
    glUniform3iv = (GLPROCUniform3iv)s_get_proc("Uniform3iv");
    glUniform4i = (GLPROCUniform4i)s_get_proc("Uniform4i");
    glUniform4iv = (GLPROCUniform4iv)s_get_proc("Uniform4iv");

}

void s_close_gl(void) {
#if defined(USE_GL_LIB)
    return;
#endif
    if (s_glsym != NULL) {
#if defined(_WIN32)
        FreeLibrary(s_glsym);
#else
        dlclose(s_glsym);
#endif
        s_glsym = NULL;
    }
}

void* s_get_proc(const char *name) {
    void *sym = NULL;
    if (s_glsym == NULL) return sym;
#if !defined(__APPLE__) && !defined(__HAIKU__)
    if (s_proc_address != NULL) {
        sym = s_proc_address(name);
    }
#endif
    if (sym == NULL) {
#if defined(_WIN32) || defined(__CYGWIN__)
        sym = (void*)GetProcAddress(s_glsym, name);
#else
        sym = (void*)dlsym(s_glsym, name);
#endif
    }
    return sym;
}
