#include "selene.h"
#include "modules/filesystem.h"
#ifndef SELENE_NO_FILESYSTEM

#include <errno.h>
#include <sys/stat.h>
#if defined(OS_WIN)
#include <direct.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif
#if defined(OS_APPLE)
#include <mach-o/dyld.h>
#endif

#include <miniz/miniz.h>

/*
 * Virtual filesystem
 *
 * Paths can be prefixed with one of the virtual roots:
 *   exec://  directory of the executable
 *   root://  game root (a directory, or a mounted zip archive)
 *   user://  user writable directory
 *
 * Paths without a prefix are looked up first in the game root (mounted zip,
 * or root directory when set with -d) and then as plain disk paths, so
 * `image.png` works the same running from a folder or from a packed game.
 */

int r_exec_path = LUA_NOREF;
int r_root_path = LUA_NOREF;
int r_user_path = LUA_NOREF;

#if defined(OS_ANDROID)
static AAssetManager* g_asset_manager = NULL;

JNIEXPORT void JNICALL
Java_com_selene_app_NativeLib_setAssetManager(JNIEnv* env, jobject thiz, jobject java_asset_manager) {
    g_asset_manager = AAssetManager_fromJava(env, java_asset_manager);
}
#endif

/*==============*
 *  Zip mount   *
 *==============*/

static struct {
    int active;
    mz_zip_archive zip;
    void* mem;              /* owned buffer when mounted from memory */
    char path[1024];
} s_mount;

enum {
    FS_RES_DISK = 0,
    FS_RES_ZIP,
};

/* Strip "./" and leading slashes so the name matches the archive entries */
static const char* zip_name(const char* name) {
    for (;;) {
        if (name[0] == '/' || name[0] == '\\') name++;
        else if (name[0] == '.' && (name[1] == '/' || name[1] == '\\')) name += 2;
        else break;
    }
    if (name[0] == '.' && name[1] == '\0') return name + 1;
    return name;
}

static int zip_find_file(const char* name) {
    if (!s_mount.active || !*name) return -1;
    return mz_zip_reader_locate_file(&s_mount.zip, name, NULL, 0);
}

/* Directories may exist only implicitly (as a prefix of some entry) */
static int zip_is_dir(const char* name) {
    if (!s_mount.active) return 0;
    if (!*name) return 1; /* archive root */
    size_t len = strlen(name);
    if (name[len - 1] == '/') len--;
    mz_uint n = mz_zip_reader_get_num_files(&s_mount.zip);
    char entry[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
    for (mz_uint i = 0; i < n; i++) {
        mz_zip_reader_get_filename(&s_mount.zip, i, entry, sizeof(entry));
        if (strncmp(entry, name, len) == 0 && entry[len] == '/') return 1;
    }
    return 0;
}

int selene_fs_mount_zip(const char* path, unsigned long long offset, unsigned long long size, const char** err) {
    selene_fs_unmount();
    memset(&s_mount.zip, 0, sizeof(s_mount.zip));
    if (mz_zip_reader_init_file_v2(&s_mount.zip, path, 0, offset, size)) {
        s_mount.active = 1;
    }
#if !defined(OS_EMSCRIPTEN)
    else {
        /* On Android relative paths live inside the APK assets, SDL can read them */
        size_t data_size = 0;
        void* data = SDL_LoadFile(path, &data_size);
        if (data) {
            if (offset > 0 && offset < data_size) {
                size_t len = size > 0 ? (size_t)size : data_size - (size_t)offset;
                memmove(data, (char*)data + offset, len);
                data_size = len;
            }
            if (selene_fs_mount_zip_mem(data, data_size, 1)) {
                strncpy(s_mount.path, path, sizeof(s_mount.path) - 1);
                return 1;
            }
            if (err) *err = "invalid zip archive";
            return 0;
        }
    }
#endif
    if (!s_mount.active) {
        if (err) *err = mz_zip_get_error_string(mz_zip_get_last_error(&s_mount.zip));
        return 0;
    }
    strncpy(s_mount.path, path, sizeof(s_mount.path) - 1);
    s_mount.path[sizeof(s_mount.path) - 1] = '\0';
    return 1;
}

int selene_fs_mount_zip_mem(void* data, size_t size, int take_ownership) {
    selene_fs_unmount();
    memset(&s_mount.zip, 0, sizeof(s_mount.zip));
    if (!mz_zip_reader_init_mem(&s_mount.zip, data, size, 0)) {
        if (take_ownership) SDL_free(data);
        return 0;
    }
    s_mount.active = 1;
    s_mount.mem = take_ownership ? data : NULL;
    strcpy(s_mount.path, "<memory>");
    return 1;
}

void selene_fs_unmount(void) {
    if (s_mount.active) mz_zip_reader_end(&s_mount.zip);
    if (s_mount.mem) SDL_free(s_mount.mem);
    s_mount.active = 0;
    s_mount.mem = NULL;
    s_mount.path[0] = '\0';
}

int selene_fs_is_mounted(void) { return s_mount.active; }

/*==============*
 * Path helpers *
 *==============*/

int get_virt_path(lua_State* L, const char* path) {
    char* start = strstr(path, "exec://");
    int virt_path = -1;
    if (start) {
        virt_path = r_exec_path;
    } else if ((start = strstr(path, "root://"))) {
        virt_path = r_root_path;
    } else if ((start = strstr(path, "user://"))) {
        virt_path = r_user_path;
    }
    if (virt_path != -1) {
        start += 6;
        lua_rawgeti(L, LUA_REGISTRYINDEX, virt_path);
        lua_pushstring(L, start);
        lua_concat(L, 2);
    }
    else {
        lua_pushstring(L, path);
    }
    return virt_path;
}

static int is_absolute(const char* path) {
    if (path[0] == '/' || path[0] == '\\') return 1;
#if defined(OS_WIN)
    if (path[0] && path[1] == ':') return 1;
#endif
    return 0;
}

static int disk_stat(const char* path, struct stat* st) {
    return stat(path, st) == 0;
}

/*
 * Resolve a (virtual) path. Pushes the resolved name onto the stack and
 * returns FS_RES_ZIP (name inside the mounted archive) or FS_RES_DISK.
 */
static int fs_resolve(lua_State* L, const char* path) {
    if (strncmp(path, "root://", 7) == 0) {
        if (s_mount.active) {
            lua_pushstring(L, zip_name(path + 7));
            return FS_RES_ZIP;
        }
#if defined(OS_ANDROID)
        /* Unpacked Android builds: root:// maps to the APK assets */
        lua_pushstring(L, zip_name(path + 7));
        return FS_RES_DISK;
#endif
        get_virt_path(L, path);
        return FS_RES_DISK;
    }
    if (strstr(path, "://")) {
        get_virt_path(L, path);
        return FS_RES_DISK;
    }
    if (!is_absolute(path)) {
        const char* name = zip_name(path);
        if (s_mount.active && (zip_find_file(name) >= 0 || zip_is_dir(name))) {
            lua_pushstring(L, name);
            return FS_RES_ZIP;
        }
        /* relative to the root directory (set with -d), if it exists there */
        lua_rawgeti(L, LUA_REGISTRYINDEX, r_root_path);
        const char* root = lua_tostring(L, -1);
        if (root && strcmp(root, ".") != 0 && !s_mount.active) {
            lua_pushfstring(L, "%s/%s", root, name);
            struct stat st;
            if (disk_stat(lua_tostring(L, -1), &st)) {
                lua_remove(L, -2);
                return FS_RES_DISK;
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    lua_pushstring(L, path);
    return FS_RES_DISK;
}

void* selene_fs_load(lua_State* L, const char* path, size_t* out_size) {
    void* data = NULL;
    size_t size = 0;
    int kind = fs_resolve(L, path);
    const char* name = lua_tostring(L, -1);
    if (kind == FS_RES_ZIP) {
        int index = zip_find_file(name);
        if (index >= 0) {
            void* zdata = mz_zip_reader_extract_to_heap(&s_mount.zip, (mz_uint)index, &size, 0);
            if (zdata) {
                /* keep the SDL_malloc/SDL_free contract and add a NUL terminator */
                data = SDL_malloc(size + 1);
                if (data) {
                    memcpy(data, zdata, size);
                    ((char*)data)[size] = '\0';
                }
                mz_free(zdata);
            }
        }
    } else {
        /* SDL_LoadFile is NUL terminated and also reads Android assets */
        data = SDL_LoadFile(name, &size);
    }
    lua_pop(L, 1);
    if (data && out_size) *out_size = size;
    return data;
}

/*==============*
 *   Searcher   *
 *==============*/

/* luaL_loadfile skips a UTF-8 BOM, luaL_loadbuffer doesn't */
static const char* skip_bom(const char* data, size_t* size) {
    if (*size >= 3 && (unsigned char)data[0] == 0xEF && (unsigned char)data[1] == 0xBB && (unsigned char)data[2] == 0xBF) {
        *size -= 3;
        return data + 3;
    }
    return data;
}

/* Lua 5.4 prefixes each searcher message with "\n\t" by itself */
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 504
#define SEARCHER_SEP ""
#else
#define SEARCHER_SEP "\n\t"
#endif

/* package.searchers entry: loads Lua modules from the mounted zip archive */
static int l_zip_searcher(lua_State* L) {
    const char* modname = luaL_checkstring(L, 1);
    if (!s_mount.active) {
        lua_pushstring(L, SEARCHER_SEP "no game archive mounted");
        return 1;
    }
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    for (const char* p = modname; *p; p++) luaL_addchar(&b, *p == '.' ? '/' : *p);
    luaL_pushresult(&b);
    const char* base = lua_tostring(L, -1);
    const char* patterns[] = { "%s.lua", "%s/init.lua", NULL };
    for (int i = 0; patterns[i]; i++) {
        lua_pushfstring(L, patterns[i], base);
        const char* filename = lua_tostring(L, -1);
        int index = zip_find_file(filename);
        if (index >= 0) {
            size_t size = 0;
            void* data = mz_zip_reader_extract_to_heap(&s_mount.zip, (mz_uint)index, &size, 0);
            if (!data) return luaL_error(L, "error reading module '%s' from archive", modname);
            lua_pushfstring(L, "@root://%s", filename);
            const char* code = skip_bom((const char*)data, &size);
            int res = luaL_loadbuffer(L, code, size, lua_tostring(L, -1));
            mz_free(data);
            if (res != LUA_OK)
                return luaL_error(L, "error loading module '%s' from archive:\n\t%s",
                                  modname, lua_tostring(L, -1));
            lua_pushfstring(L, "root://%s", filename);
            return 2;
        }
        lua_pop(L, 1);
    }
    lua_pushfstring(L, SEARCHER_SEP "no file '%s.lua' in game archive"
                       "\n\tno file '%s/init.lua' in game archive", base, base);
    return 1;
}

void selene_fs_insert_searcher(lua_State* L, lua_CFunction fn, int pos) {
    lua_getglobal(L, "package");
#if SELENE_USE_JIT
    lua_getfield(L, -1, "loaders");
#else
    lua_getfield(L, -1, "searchers");
#endif
    int n = (int)lua_rawlen(L, -1);
    if (pos > n + 1) pos = n + 1;
    for (int i = n; i >= pos; i--) {
        lua_rawgeti(L, -1, i);
        lua_rawseti(L, -2, i + 1);
    }
    lua_pushcfunction(L, fn);
    lua_rawseti(L, -2, pos);
    lua_pop(L, 2);
}

#if defined(OS_ANDROID)
int lua_android_require(lua_State* L) {
    const char* module_name = luaL_checkstring(L, 1);
    size_t len = strlen(module_name);
    char filename[256];
    strcpy(filename, module_name);
    for (int i = 0; i < len; i++) {
        if (filename[i] == '.') filename[i] = '/';
    }
    strcat(filename, ".lua");

    if (!g_asset_manager) {
        lua_pushstring(L, "AssetManager not initialized\n");
        return 1;
    }
    AAsset* asset = AAssetManager_open(g_asset_manager, filename, AASSET_MODE_BUFFER);
    char* buffer = NULL;
    size_t size;
    if (asset) {
        size = AAsset_getLength(asset);
        buffer = (char*)malloc(size + 1);
        if (!buffer) {
            AAsset_close(asset);
            lua_pushstring(L, "Memory allocation failed");
            return 1;
        }

        AAsset_read(asset, buffer, size);
        buffer[size] = '\0'; // Null-terminate
        AAsset_close(asset);
    } else {
        lua_rawgeti(L, LUA_REGISTRYINDEX, r_user_path);
        lua_pushstring(L, filename);
        lua_concat(L, 2);
        FILE* fp = fopen(lua_tostring(L, -1), "rb");
        if (!fp) {
            lua_pushfstring(L, "\n\tno file '%s'", filename);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(size + 1);
        if (!buffer) {
            fclose(fp);
            lua_pushstring(L, "failed to allocate memory\n");
            return 1;
        }
        fread(buffer, 1, size, fp);
        buffer[size] = '\0';
        fclose(fp);
    }

    if (luaL_loadbuffer(L, buffer, size, filename) != LUA_OK) {
        fprintf(stderr, "[ERROR] failed to load Lua buffer %s\n", filename);
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
    }
    free(buffer);
    return 1;
}
#endif /* OS_ANDROID */

/*==============*
 * Fused games  *
 *==============*/

/* Trailer appended after the zip: "SELENEPK" + u64 LE offset of the zip start */
#define FUSED_MAGIC "SELENEPK"
#define FUSED_TRAILER_SIZE 16

int selene_fs_get_exe_path(char* out, size_t size) {
    if (size == 0) return 0;
    out[0] = '\0';
#if defined(OS_WIN)
    DWORD len = GetModuleFileNameA(NULL, out, (DWORD)size);
    if (len == 0 || len >= size) return 0;
    return 1;
#elif defined(OS_APPLE)
    uint32_t len = (uint32_t)size;
    if (_NSGetExecutablePath(out, &len) != 0) return 0;
    return 1;
#elif defined(OS_LINUX) || defined(OS_ANDROID)
    ssize_t len = readlink("/proc/self/exe", out, size - 1);
    if (len <= 0) return 0;
    out[len] = '\0';
    return 1;
#elif defined(OS_BSD)
    ssize_t len = readlink("/proc/curproc/file", out, size - 1);
    if (len <= 0) return 0;
    out[len] = '\0';
    return 1;
#else
    return 0;
#endif
}

int selene_fs_fused_info(const char* path, unsigned long long* offset, unsigned long long* size) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
    unsigned char trailer[FUSED_TRAILER_SIZE];
    int found = 0;
    if (fseek(fp, 0, SEEK_END) == 0) {
        long long file_size = (long long)ftell(fp);
        if (file_size > FUSED_TRAILER_SIZE &&
            fseek(fp, -FUSED_TRAILER_SIZE, SEEK_END) == 0 &&
            fread(trailer, 1, FUSED_TRAILER_SIZE, fp) == FUSED_TRAILER_SIZE &&
            memcmp(trailer, FUSED_MAGIC, 8) == 0) {
            unsigned long long off = 0;
            for (int i = 7; i >= 0; i--) off = (off << 8) | trailer[8 + i];
            if (off < (unsigned long long)(file_size - FUSED_TRAILER_SIZE)) {
                if (offset) *offset = off;
                if (size) *size = (unsigned long long)(file_size - FUSED_TRAILER_SIZE) - off;
                found = 1;
            }
        }
    }
    fclose(fp);
    return found;
}

static int copy_stream(FILE* dst, FILE* src, unsigned long long* written) {
    char buf[64 * 1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) return 0;
        if (written) *written += n;
    }
    return !ferror(src);
}

/*==============*
 *     Lua      *
 *==============*/

static int push_fail(lua_State* L, const char* fmt, const char* arg) {
    lua_pushnil(L);
    lua_pushfstring(L, fmt, arg);
    return 2;
}

static int l_filesystem_open(lua_State* L) {
    return 0;
}

/* filesystem.read(path) -> string, size | nil, err */
static int l_filesystem_read(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    size_t size = 0;
    char* buffer = (char*)selene_fs_load(L, filename, &size);
    if (!buffer) return push_fail(L, "failed to open file: %s", filename);
    lua_pushlstring(L, buffer, size);
    lua_pushinteger(L, (lua_Integer)size);
    SDL_free(buffer);
    return 2;
}

/* filesystem.write(path, data) -> true | nil, err */
static int l_filesystem_write(lua_State* L) {
    size_t size;
    const char* filename = luaL_checkstring(L, 1);
    const char* buffer = luaL_checklstring(L, 2, &size);
    int virt = get_virt_path(L, filename);
    filename = lua_tostring(L, -1);
#if defined(OS_ANDROID) || defined(OS_PS4) || defined(OS_PS5) || defined(OS_NSWITCH)
    if (virt == r_root_path || virt == r_exec_path) {
        return luaL_error(L, "cannot write in the root path\n");
    }
#endif
    if (virt == r_root_path && s_mount.active)
        return push_fail(L, "cannot write into a mounted archive: %s", filename);
    FILE* fp = fopen(filename, "wb");
    if (!fp) return push_fail(L, "failed to open file: %s", filename);
    size_t written = fwrite(buffer, 1, size, fp);
    fclose(fp);
    if (written != size) return push_fail(L, "failed to write file: %s", filename);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_filesystem_mkdir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int virt = get_virt_path(L, path);
    path = lua_tostring(L, -1);
#if defined(OS_ANDROID) || defined(OS_PS4) || defined(OS_PS5) || defined(OS_NSWITCH)
    if (virt == r_root_path || virt == r_exec_path) {
        return luaL_error(L, "cannot create dir in the root path\n");
    }
#endif
    (void)virt;
    int res =
#if defined(OS_WIN)
    _mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    lua_pushinteger(L, res);
    return 1;
}

static int l_filesystem_rmdir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int virt = get_virt_path(L, path);
    path = lua_tostring(L, -1);
#if defined(OS_ANDROID) || defined(OS_PS4) || defined(OS_PS5) || defined(OS_NSWITCH)
    if (virt == r_root_path || virt == r_exec_path) {
        return luaL_error(L, "cannot remove dir in the root path\n");
    }
#endif
    (void)virt;
#if defined(OS_WIN)
    int res = _rmdir(path);
#else
    int res = rmdir(path);
#endif
    lua_pushboolean(L, res == 0);
    return 1;
}

/* filesystem.remove(path) -> true | nil, err  (files only, disk only) */
static int l_filesystem_remove(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    get_virt_path(L, path);
    path = lua_tostring(L, -1);
    if (remove(path) != 0) return push_fail(L, "failed to remove: %s", path);
    lua_pushboolean(L, 1);
    return 1;
}

/* filesystem.load(path) -> function | nil, err */
static int l_filesystem_load(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    size_t size = 0;
    char* buffer = (char*)selene_fs_load(L, path, &size);
    if (!buffer) return push_fail(L, "failed to open file: %s", path);
    lua_pushfstring(L, "@%s", path);
    const char* code = skip_bom(buffer, &size);
    int res = luaL_loadbuffer(L, code, size, lua_tostring(L, -1));
    SDL_free(buffer);
    if (res != LUA_OK) {
        lua_pushnil(L);
        lua_insert(L, -2);
        return 2;
    }
    return 1;
}

static int l_filesystem_resolve(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    get_virt_path(L, filename);
    return 1;
}

static int l_filesystem_set_path(lua_State* L) {
    const char* virt = luaL_checkstring(L, 1);
    luaL_checkstring(L, 2);
    lua_pushvalue(L, 2);
    if (strstr(virt, "exec://")) lua_rawseti(L, LUA_REGISTRYINDEX, r_exec_path);
    else if (strstr(virt, "root://")) lua_rawseti(L, LUA_REGISTRYINDEX, r_root_path);
    else if (strstr(virt, "user://")) lua_rawseti(L, LUA_REGISTRYINDEX, r_user_path);
    else return luaL_error(L, "Invalid virtual path");
    return 0;
}

/* filesystem.stat(path) -> { type = 'file'|'directory', size, modtime } | nil */
static int l_filesystem_stat(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int kind = fs_resolve(L, path);
    const char* name = lua_tostring(L, -1);
    if (kind == FS_RES_ZIP) {
        int index = zip_find_file(name);
        mz_zip_archive_file_stat st;
        if (index >= 0 && mz_zip_reader_file_stat(&s_mount.zip, (mz_uint)index, &st)) {
            lua_newtable(L);
            lua_pushstring(L, st.m_is_directory ? "directory" : "file");
            lua_setfield(L, -2, "type");
            lua_pushinteger(L, (lua_Integer)st.m_uncomp_size);
            lua_setfield(L, -2, "size");
#ifndef MINIZ_NO_TIME
            lua_pushinteger(L, (lua_Integer)st.m_time);
            lua_setfield(L, -2, "modtime");
#endif
            return 1;
        }
        if (zip_is_dir(name)) {
            lua_newtable(L);
            lua_pushstring(L, "directory");
            lua_setfield(L, -2, "type");
            lua_pushinteger(L, 0);
            lua_setfield(L, -2, "size");
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }
    struct stat st;
    if (!disk_stat(name, &st)) {
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    lua_pushstring(L, (st.st_mode & S_IFMT) == S_IFDIR ? "directory" : "file");
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, (lua_Integer)st.st_size);
    lua_setfield(L, -2, "size");
    lua_pushinteger(L, (lua_Integer)st.st_mtime);
    lua_setfield(L, -2, "modtime");
    return 1;
}

static int l_filesystem_exists(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int kind = fs_resolve(L, path);
    const char* name = lua_tostring(L, -1);
    int exists;
    if (kind == FS_RES_ZIP) {
        exists = zip_find_file(name) >= 0 || zip_is_dir(name);
    } else {
        struct stat st;
        exists = disk_stat(name, &st);
    }
    lua_pushboolean(L, exists);
    return 1;
}

/* filesystem.list(dir) -> { name, ... } | nil, err  (non recursive, no '.'/'..') */
static int l_filesystem_list(lua_State* L) {
    const char* path = luaL_optstring(L, 1, ".");
    int kind = fs_resolve(L, path);
    const char* name = lua_tostring(L, -1);
    int count = 0;
    if (kind == FS_RES_ZIP) {
        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '/') len--;
        lua_newtable(L);            /* result */
        lua_newtable(L);            /* seen set */
        mz_uint n = mz_zip_reader_get_num_files(&s_mount.zip);
        char entry[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
        for (mz_uint i = 0; i < n; i++) {
            mz_zip_reader_get_filename(&s_mount.zip, i, entry, sizeof(entry));
            const char* child = entry;
            if (len > 0) {
                if (strncmp(entry, name, len) != 0 || entry[len] != '/') continue;
                child = entry + len + 1;
            }
            if (!*child) continue;
            const char* slash = strchr(child, '/');
            size_t clen = slash ? (size_t)(slash - child) : strlen(child);
            lua_pushlstring(L, child, clen);
            lua_pushvalue(L, -1);
            lua_rawget(L, -3);
            if (lua_toboolean(L, -1)) {
                lua_pop(L, 2);
                continue;
            }
            lua_pop(L, 1);
            lua_pushvalue(L, -1);
            lua_pushboolean(L, 1);
            lua_rawset(L, -4);
            lua_rawseti(L, -3, ++count);
        }
        lua_pop(L, 1);
        return 1;
    }
#if defined(OS_WIN)
    lua_pushfstring(L, "%s\\*", name);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(lua_tostring(L, -1), &fd);
    lua_pop(L, 1);
    if (h == INVALID_HANDLE_VALUE) return push_fail(L, "failed to open directory: %s", name);
    lua_newtable(L);
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        lua_pushstring(L, fd.cFileName);
        lua_rawseti(L, -2, ++count);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR* dir = opendir(name);
    if (!dir) return push_fail(L, "failed to open directory: %s", name);
    lua_newtable(L);
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        lua_pushstring(L, ent->d_name);
        lua_rawseti(L, -2, ++count);
    }
    closedir(dir);
#endif
    return 1;
}

/* filesystem.copy(src, dst) -> true | nil, err  (src may be inside the game archive) */
static int l_filesystem_copy(lua_State* L) {
    const char* src = luaL_checkstring(L, 1);
    const char* dst = luaL_checkstring(L, 2);
    size_t size = 0;
    void* data = selene_fs_load(L, src, &size);
    if (!data) return push_fail(L, "failed to open file: %s", src);
    get_virt_path(L, dst);
    dst = lua_tostring(L, -1);
    FILE* fp = fopen(dst, "wb");
    if (!fp) {
        SDL_free(data);
        return push_fail(L, "failed to open file: %s", dst);
    }
    size_t written = fwrite(data, 1, size, fp);
    fclose(fp);
    SDL_free(data);
    if (written != size) return push_fail(L, "failed to write file: %s", dst);
    lua_pushboolean(L, 1);
    return 1;
}

/*
 * filesystem.mount(path [, offset [, size]]) -> true | nil, err
 * Mounts a zip archive (or a directory) as the game root.
 */
static int l_filesystem_mount(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    unsigned long long offset = (unsigned long long)luaL_optnumber(L, 2, 0);
    unsigned long long size = (unsigned long long)luaL_optnumber(L, 3, 0);
    get_virt_path(L, path);
    const char* real = lua_tostring(L, -1);
    struct stat st;
    if (disk_stat(real, &st) && (st.st_mode & S_IFMT) == S_IFDIR) {
        selene_fs_unmount();
        lua_rawseti(L, LUA_REGISTRYINDEX, r_root_path);
        lua_pushboolean(L, 1);
        return 1;
    }
    const char* err = NULL;
    if (!selene_fs_mount_zip(real, offset, size, &err)) {
        lua_pushnil(L);
        lua_pushfstring(L, "%s: %s", real, err ? err : "unknown error");
        return 2;
    }
    lua_rawseti(L, LUA_REGISTRYINDEX, r_root_path);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_filesystem_unmount(lua_State* L) {
    selene_fs_unmount();
    lua_pushstring(L, ".");
    lua_rawseti(L, LUA_REGISTRYINDEX, r_root_path);
    return 0;
}

/* filesystem.mounted() -> path of the mounted archive | nil */
static int l_filesystem_mounted(lua_State* L) {
    if (!s_mount.active) lua_pushnil(L);
    else lua_pushstring(L, s_mount.path);
    return 1;
}

static int l_filesystem_exe_path(lua_State* L) {
    char path[4096];
    if (!selene_fs_get_exe_path(path, sizeof(path))) lua_pushnil(L);
    else lua_pushstring(L, path);
    return 1;
}

/* filesystem.fused_info(path) -> offset, size | nil */
static int l_filesystem_fused_info(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    get_virt_path(L, path);
    unsigned long long offset, size;
    if (!selene_fs_fused_info(lua_tostring(L, -1), &offset, &size)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushinteger(L, (lua_Integer)offset);
    lua_pushinteger(L, (lua_Integer)size);
    return 2;
}

/* filesystem.fuse(exe, zip, out) -> true | nil, err   writes [exe][zip][trailer] */
static int l_filesystem_fuse(lua_State* L) {
    const char* paths[3];
    for (int i = 0; i < 3; i++) {
        get_virt_path(L, luaL_checkstring(L, i + 1));
        paths[i] = lua_tostring(L, -1);
    }
    FILE* exe = fopen(paths[0], "rb");
    if (!exe) return push_fail(L, "failed to open file: %s", paths[0]);
    FILE* zip = fopen(paths[1], "rb");
    if (!zip) {
        fclose(exe);
        return push_fail(L, "failed to open file: %s", paths[1]);
    }
    FILE* out = fopen(paths[2], "wb");
    if (!out) {
        fclose(exe);
        fclose(zip);
        return push_fail(L, "failed to open file: %s", paths[2]);
    }
    unsigned long long offset = 0;
    int ok = copy_stream(out, exe, &offset) && copy_stream(out, zip, NULL);
    if (ok) {
        unsigned char trailer[FUSED_TRAILER_SIZE];
        memcpy(trailer, FUSED_MAGIC, 8);
        for (int i = 0; i < 8; i++) trailer[8 + i] = (unsigned char)((offset >> (8 * i)) & 0xff);
        ok = fwrite(trailer, 1, FUSED_TRAILER_SIZE, out) == FUSED_TRAILER_SIZE;
    }
    fclose(exe);
    fclose(zip);
    if (fclose(out) != 0) ok = 0;
    if (!ok) return push_fail(L, "failed to write fused executable: %s", paths[2]);
#if !defined(OS_WIN)
    chmod(paths[2], 0755);
#endif
    lua_pushboolean(L, 1);
    return 1;
}

int luaopen_filesystem(lua_State* L) {
    const luaL_Reg _reg[] = {
        {"read", l_filesystem_read},
        {"write", l_filesystem_write},
        {"mkdir", l_filesystem_mkdir},
        {"rmdir", l_filesystem_rmdir},
        {"remove", l_filesystem_remove},
        {"load", l_filesystem_load},
        {"resolve", l_filesystem_resolve},
        {"set_path", l_filesystem_set_path},
        {"exists", l_filesystem_exists},
        {"stat", l_filesystem_stat},
        {"list", l_filesystem_list},
        {"copy", l_filesystem_copy},
        {"mount", l_filesystem_mount},
        {"unmount", l_filesystem_unmount},
        {"mounted", l_filesystem_mounted},
        {"exe_path", l_filesystem_exe_path},
        {"fused_info", l_filesystem_fused_info},
        {"fuse", l_filesystem_fuse},
        {NULL, NULL}
    };
    (void)l_filesystem_open;
    luaL_newlib(L, _reg);
    /* zip searcher goes right after package.preload */
    selene_fs_insert_searcher(L, l_zip_searcher, 2);
    return 1;
}
#endif /* SELENE_NO_FILESYSTEM */
