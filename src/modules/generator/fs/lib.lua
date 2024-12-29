local sources = {}

--- @type generator.Group
sources[1] = {
    function_block('boolean', 'exists', 'string path',
    {
        ccall =
[[int exists = 1;
    struct stat info;
    if (stat(path, &info) == -1)
        exists = 0;
    PUSH_BOOLEAN(exists);]]
    }),
    function_block('Data', 'read', 'string path',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_STRING(path);
    #if defined(OS_WIN)
        FILE* fp;
        fopen_s(&fp, path, "rb");
    #else
        FILE* fp = fopen(path, "rb");
    #endif

    if (!fp)
        return luaL_error(L, "Failed to read file: %s\n", path);

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    NEW_UDATA_ADD(Data, d, size);
    // d->root = (char*)(d + 1);
    d[0] = size;
    
    d++;
    fread(d, 1, size, fp);
    // fread(d->root, 1, size, fp);
    fclose(fp);

    return 1;
]]
    }),
    function_block('string', 'read_text', 'string path',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_STRING(path);
#if defined(OS_WIN)
    FILE* fp;
    fopen_s(&fp, path, "rb");
#else
    FILE* fp = fopen(path, "rb");
#endif

    if (!fp)
        return luaL_error(L, "Failed to read file: %s\n", path);

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* data = malloc(size+1);
    fread(data, 1, size, fp);
    data[size] = '\0';
    fclose(fp);

    lua_pushstring(L, data);
    free(data);

    return 1;
]]
    }),
    function_block('boolean', 'write', 'string path, string text',
    {
        cdef =
[[
    INIT_ARG();
    size_t len;
    CHECK_STRING(path);
    CHECK_LSTRING(text, &len);
#if defined(OS_WIN)
    FILE* fp;
    fopen_s(&fp, path, "w");
#else
    FILE* fp = fopen(path, "w");
#endif
    if (!fp)
        return luaL_error(L, "failed to create file %s\n", path);
    fprintf(fp, "%*s", (int)len, text);
    fclose(fp);
    PUSH_BOOLEAN(1);
    return 1;
]]
    }),
    function_block('', 'mkdir', 'string path',
    {
        ccall =
[[#if defined(OS_WIN)
    mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif]]
    }),
    function_block('', 'rmdir', 'string path', { ccall = "rmdir(path);" }),
    -- function_block('boolean', 'load', 'string path', { ccall = "luaL_loadfile(L, path);" })
}

local lib = Lib.create('fs')
lib:add_source(
{
c_code_block([[
#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
]])
}
)
for i,value in ipairs(sources) do
    lib:add_source(value)
end

lib.header_include =
[[
#include "selene.h"
#include "lua_helper.h"
]]

return lib