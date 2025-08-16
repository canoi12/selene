#include "selene.h"
#include "modules/filesystem.h"
#ifndef SELENE_NO_FILESYSTEM

//typedef struct _File selene_File;
struct _File {
    int virtual;
    int dirty;
    int offset, size;
    void* handle;
};

static int mount_virtual(lua_State* L, const char* path) {
    return 0;
}
static int unmount_virtual(lua_State* L, const char* path) {
    return 0;
}

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

int get_virt_path(lua_State* L, const char* path) {
    char* start = strstr(path, "exec://");
    int virt_path = -1;
    if (start) {
        virt_path = r_exec_path;
        goto CHECK_VIRT;
    }
    start = strstr(path, "root://");
    if (start) {
        virt_path = r_root_path;
        goto CHECK_VIRT;
    }
    start = strstr(path, "user://");
    if (start) virt_path = r_user_path;
CHECK_VIRT:
    if (virt_path != -1) {
        start += 6;
#if defined(OS_WIN)
        //*start = '\\';
#endif
        lua_rawgeti(L, LUA_REGISTRYINDEX, virt_path);
        //fprintf(stdout, "path: %s\n", lua_tostring(L, -1));
        //fprintf(stdout, "virt: %s\n", start);
        lua_pushstring(L, start);
        lua_concat(L, 2);
    }
    else {
        lua_pushstring(L, path);
    }
    return virt_path;
}

static int lua_selene_require(lua_State* L) {
    const char* module_name = luaL_checkstring(L, 1);
    size_t len = strlen(module_name);
    char filename[256];
    strcpy(filename, module_name);
    for (int i = 0; i < len; i++) {
        if (filename[i] == '.') filename[i] = '/';
    }
    strcat(filename, ".lua");

#if defined(OS_ANDROID)
    if (!g_asset_manager) {
        lua_pushstring(L, "AssetManager not initialized\n");
        return 1;
    }
    AAsset* asset = AAssetManager_open(g_assetManager, filename, AASSET_MODE_BUFFER);
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
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(size + 1);
        if (!buffer) {
            lua_pushstring(L, "failed to allocate memory\n");
            return 1;
        }
        fread(buffer, 1, size, fp);
        buffer[size] = '\0';
        fclose(fp);
    }
#else
    int virts[3] = {r_exec_path, r_root_path, r_user_path};
    char* buffer = NULL;
    size_t size;
    for (int i = 0; i < 3; i++) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, virts[i]);
        lua_pushstring(L, filename);
        lua_concat(L, 2);
        FILE* fp = fopen(lua_tostring(L, -1), "rb");
        lua_pop(L, 1);
        if (fp) {
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            buffer = malloc(size + 1);
            if (!buffer) {
                lua_pushstring(L, "failed to allocate memory\n");
                return 1;
            }
            fread(buffer, 1, size, fp);
            buffer[size] = '\0';
            fclose(fp);
            goto EXIT;
        }
    }
    goto EXIT_ERR;
#endif
EXIT:
    if (luaL_loadbuffer(L, buffer, size, filename) != LUA_OK) {
        fprintf(stderr, "[ERROR] failed to load Lua buffer %s\n", filename);
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
    }
    free(buffer);
EXIT_ERR:
    return 1;
}

static int l_filesystem_open(lua_State* L) {
    //NEW_META(selene_File, file);
    //file->handle = NULL;
    return 0;
}

static int l_filesystem_read(lua_State* L) {
    int arg = 0;
    const char* filename = luaL_checkstring(L, arg++);
    size_t size;
    char* buffer = NULL;
    int virt = get_virt_path(L, filename);
    if (virt > 0) filename = lua_tostring(L, -1);
#if defined(OS_ANDROID)
    if (virt == r_root_path) {
        if (!g_asset_manager) {
            lua_pushstring(L, "AssetManager not initialized\n");
            return 1;
        }
        AAsset* asset = AAssetManager_open(g_assetManager, filename, AASSET_MODE_BUFFER);
        if (!asset) {
            lua_pushfstring(L, "Asset not found: %s", filename);
            return 1;
        }
        size = AAsset_getLength(asset);
        buffer = (char*)malloc(size + 1);
        if (!buffer) {
            AAsset_close(asset);
            lua_pushstring(L, "Memory allocation failed");
            return 1;
        }

        AAsset_read(asset, buffer, size);
        buffer[size] = '\0';
        AAsset_close(asset);
        lua_pushlstring(L, buffer, size);
        lua_pushinteger(L, size);
        return 2;
    }
#endif
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {} 
#endif
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to open file: %s", filename);
        return 2;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to allocate memory");
        return 2;
    }
    int res = fread(buffer, 1, size, fp);
    fclose(fp);
    if (res != size) {
        lua_pushnil(L);
        lua_pushfstring(L, "Readed only %db of %db %s\n", res, size, filename);
        return 2;
    }
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {}
#endif
    buffer[size] = '\0';
    lua_pushlstring(L, buffer, size);
    lua_pushinteger(L, size);
    return 2;
}

static int l_filesystem_write(lua_State* L) {
    int arg = 0;
    size_t size;
    char* buffer;
    const char* filename = luaL_checkstring(L, arg++);
    if (lua_isstring(L, arg)) {
        buffer = (char*)luaL_checklstring(L, arg++, &size);
    }
    int virt = get_virt_path(L, filename);
    if (virt > 0) filename = lua_tostring(L, -1);
#if defined(OS_ANDROID) || defined(OS_PS4) || defined(OS_PS5) || defined(OS_NSWITCH)
    if (virt == r_root_path || virt == r_exec_path) {
        return luaL_error(L, "cannot write in the root path\n");
    }
#endif
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {
        // mount
    } 
#endif
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to open file: %s", filename);
        return 2;
    }
    fwrite(buffer, 1, size, fp);
    fclose(fp);
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {
        // unmount
    }
#endif
    return 0;
}

static int l_filesystem_mkdir(lua_State* L) {
    int arg = 0;
    const char* path = luaL_checkstring(L, arg++);
    int virt = get_virt_path(L, path);
    if (virt > 0) path = lua_tostring(L, -1);
    lua_pop(L, 1);
#if defined(OS_ANDROID) || defined(OS_PS4) || defined(OS_PS5) || defined(OS_NSWITCH)
    if (virt == r_root_path || virt == r_exec_path) {
        return luaL_error(L, "cannot create dir in the root path\n");
    }
#endif
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {
        // mount
    } 
#endif

    int res =
#if defined(OS_WIN)
    mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {
        // unmount
    }
#endif
    lua_pushinteger(L, res);
    return 1;
}

static int l_filesystem_rmdir(lua_State* L) {
    int arg = 0;
    const char* path = luaL_checkstring(L, arg++);
    int virt = get_virt_path(L, path);
    if (virt > 0) path = lua_tostring(L, -1);
    lua_pop(L, 1);
#if defined(OS_ANDROID) || defined(OS_PS4) || defined(OS_PS5) || defined(OS_NSWITCH)
    if (virt == r_root_path || virt == r_exec_path) {
        return luaL_error(L, "cannot remove dir in the root path\n");
    }
#endif
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {
        // mount
    } 
#endif
    int res = rmdir(path);
#if defined(OS_PS4) || defined(OS_PS5)
    if (virt == r_user_path) {
        // unmount
    }
#endif
    return 0;
}

static int l_filesystem_load(lua_State* L) {
    int arg = 0;
    const char* path = luaL_checkstring(L, arg++);
    int virt = get_virt_path(L, path);
    if (virt > 0) path = lua_tostring(L, -1);
    lua_pop(L, 1);
    size_t size = 0;
    char* buffer = NULL;
#if defined(OS_ANDROID)
    if (virt == r_root_path) {
        if (!g_asset_manager) {
            lua_pushstring(L, "AssetManager not initialized\n");
            return 1;
        }
        AAsset* asset = AAssetManager_open(g_assetManager, path, AASSET_MODE_BUFFER);
        if (!asset) {
            lua_pushfstring(L, "Asset not found: %s", path);
            return 1; // Retorna erro
        }
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
    }
    luaL_loadbuffer(L, buffer, size, path); 
#else
    luaL_loadfile(L, path);
#endif
    return 1;
}

static int l_filesystem_resolve(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    get_virt_path(L, filename);
    return 1;
}

static int l_filesystem_set_path(lua_State* L) {
    const char* virt = luaL_checkstring(L, 1);
    const char* path = luaL_checkstring(L, 2);
    lua_pushvalue(L, 2);
    if (strstr(virt, "exec://")) lua_rawseti(L, LUA_REGISTRYINDEX, r_exec_path);
    else if (strstr(virt, "root://")) lua_rawseti(L, LUA_REGISTRYINDEX, r_root_path);
    else if (strstr(virt, "user://")) lua_rawseti(L, LUA_REGISTRYINDEX, r_user_path);
    else return luaL_error(L, "Invalid virtual path");
    return 0;
}

int luaopen_filesystem(lua_State* L) {
    const luaL_Reg _reg[] = {
        {"mkdir", l_filesystem_mkdir},
        {"rmdir", l_filesystem_rmdir},
        {"load", l_filesystem_load},
        {"resolve", l_filesystem_resolve},
        {"set_path", l_filesystem_set_path},
        {NULL, NULL}
    };
    luaL_newlib(L, _reg);
    return 1;
}
#endif /* SELENE_NO_FILESYSTEM */