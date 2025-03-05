local gl = {}
gl.enums = require('enums')

function func_args(...)
    local args = {...}
    local res = {}
    for i,a in ipairs(args) do
        -- print(a)
        local p = string.explode(a, ' ')
        if #p > 2 then
            for c=2,#p-1 do
                p[1] = p[1] .. ' ' .. p[c]
            end
            p[2] = p[#p]
        end
        res[i] = {
            type = p[1],
            name = p[2]
        }
    end
    return res
end

gl.functions = require('functions')

function uniform_functions(ftype)
    local ret = {}
    local aux = {
        {ftype .. ' v0'},
        {ftype .. ' v0', ftype .. ' v1'},
        {ftype .. ' v0', ftype .. ' v1', ftype .. ' v2'},
        {ftype .. ' v0', ftype .. ' v1', ftype .. ' v2', ftype .. ' v3'},
    }
    local suffix = 'f'
    if ftype == 'int' then
        suffix = 'i'
    end
    for i=1,4 do
        local args = {}
        local a = {
            name = 'Uniform' .. i .. suffix,
            rettype = 'void',
            args = func_args('int loc', table.unpack(aux[i]))
        }
        table.insert(gl.functions, a)

        local c = {
            name = 'Uniform' .. i .. suffix .. 'v',
            rettype = 'void',
            args = func_args('int loc', 'int n', ftype .. '* v')
        }
        table.insert(gl.functions, c)
    end
    return table.unpack(ret)
end

for i,func in ipairs({uniform_functions('float')}) do
    table.insert(gl.functions, func)
end
for i,func in ipairs({uniform_functions('int')}) do
    table.insert(gl.functions, func)
end

function generate_header()
    local file =
[[
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

]]

    for enum,value in pairs(gl.enums) do
        file = file .. '#define GL_' .. enum .. ' ' .. string.format('0x%x', value) .. '\n'
    end

    file = file .. '\n'

    for i,func in ipairs(gl.functions) do
        if func.name == 'break' then
            file = file .. '\n'
        else
            file = file .. 'typedef ' .. func.rettype
            file = file .. '(*GLPROC' .. func.name .. ')('
            local args = func.args or {}
            for j,a in ipairs(args) do
                -- print(a.type, a.name)
                file = file .. a.type .. ' ' .. a.name
                if j ~= #args then
                    file = file .. ', '
                end
            end
            file = file .. ');\n'
            file = file .. 'extern GLPROC' .. func.name .. ' gl' .. func.name .. ';\n'
        end
    end

    file = file ..
[[

typedef void*(*GetProcFunction)(const char*);

int setup_minigl(GetProcFunction proc);

#endif /* SELENE_MINIGL_H_ */]]

    io.writefile(_MAIN_SCRIPT_DIR .. '/minigl.h', file)
end

function generate_source()
    local file =
[[
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

]]

    for i,func in ipairs(gl.functions) do
        if func.name == 'break' then
            file = file .. '\n'
        else

        end
    end

    file = file ..
[[



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
]]
    for i,func in ipairs(gl.functions) do
        if func.name ~= 'break' then
            file = file .. '    gl' .. func.name .. ' = (GLPROC' .. func.name .. ')' .. 's_get_proc("' .. func.name .. '");\n'
        end
    end

file = file..
[[

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
]]

    io.writefile(_MAIN_SCRIPT_DIR .. '/minigl.c', file)
end

function generate_files()
    generate_header()
    generate_source()
end

newaction {
    trigger = 'generate',
    description = 'generate minigl files',
    execute = generate_files
}
