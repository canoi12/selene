#include "selene.h"

#if !defined(OS_WIN)
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

static BEGIN_META_FUNCTION(File, SeekSet)
    CHECK_INTEGER(n);
    fseek(*self, n, SEEK_SET);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, SeekEnd)
    fseek(*self, 0, SEEK_END);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, SeekCur)
    CHECK_INTEGER(n);
    fseek(*self, n, SEEK_CUR);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, Tell)
    PUSH_INTEGER(ftell(*self));
END_FUNCTION(1)

static BEGIN_META_FUNCTION(File, Read)
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, Write)
END_FUNCTION(0)

static BEGIN_META_FUNCTION(File, Append)
END_FUNCTION(0)

static BEGIN_META(File)
    BEGIN_REG(File)
        REG_META_FIELD(File, SeekSet),
        REG_META_FIELD(File, SeekEnd),
        REG_META_FIELD(File, SeekCur),
        REG_META_FIELD(File, Tell),
        REG_META_FIELD(File, Read),
        REG_META_FIELD(File, Write),
        REG_META_FIELD(File, Append),
    END_REG()
    NEW_META(File);
END_META(1)

static BEGIN_FUNCTION(fs, open)
    CHECK_STRING(path);
    NEW_UDATA(File, file);
    #if defined(OS_WIN)
        fopen_s(file, path, "rb");
    #else
        *file = fopen(path, "rb");
    #endif
    if (!(*file)) {
        return luaL_error(L, "Failed to load file");
    }
END_FUNCTION(1)

static BEGIN_FUNCTION(fs, exists)
    struct stat info;
    CHECK_STRING(path);
    int exists = 1;
    if (stat(path, &info) == -1)
        exists = 0;
    PUSH_BOOLEAN(exists);
END_FUNCTION(1)

static BEGIN_FUNCTION(fs, read)
    CHECK_STRING(path);
    #if defined(OS_WIN)
        FILE* fp;
        fopen_s(&fp, path, "rb");
    #else
        FILE* fp = fopen(path, "rb");
    #endif
    
    if (!fp)
        return luaL_error(L, "Failed to read file: %s", path);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* data = malloc(size+1);
    data[size] = '\0';
    fread(data, 1, size+1, fp);
    fclose(fp);
    PUSH_STRING(data);
    PUSH_INTEGER(size);
    free(data);
END_FUNCTION(2)

static BEGIN_FUNCTION(fs, write)
    CHECK_STRING(path);
    CHECK_STRING(text);

    #if defined(OS_WIN)
        FILE* fp;
        fopen_s(&fp, path, "w");
    #else
	    FILE* fp = fopen(path, "w");
    #endif
	if (!fp)
        return luaL_error(L, "failed to create file %s\n", path);
	fprintf(fp, "%s", text);
	fclose(fp);
    PUSH_BOOLEAN(1);
END_FUNCTION(1)

static BEGIN_FUNCTION(fs, mkdir)
    CHECK_STRING(path);
#if defined(OS_WIN)
    mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
END_FUNCTION(0)

static BEGIN_FUNCTION(fs, rmdir)
    CHECK_STRING(path);
    rmdir(path);
END_FUNCTION(0)

static BEGIN_FUNCTION(fs, load)
    CHECK_STRING(path);
    luaL_loadfile(L, path);
END_FUNCTION(1)

BEGIN_MODULE(fs)
    BEGIN_REG(fs)
        REG_FIELD(fs, open),
        REG_FIELD(fs, exists),
        REG_FIELD(fs, read),
        REG_FIELD(fs, write),
        REG_FIELD(fs, mkdir),
        REG_FIELD(fs, rmdir),
        REG_FIELD(fs, load),
    END_REG()
    NEW_MODULE(fs);
    LOAD_META(File);
END_MODULE(1)