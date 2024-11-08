#include "selene.h"

#if 0
AAssetManager* asset_manager = NULL;

JNIEXPORT void JNICALL
Java_org_selene_app_SDLActivity_initAManager(JNIEnv* env, jobject obj, jobject assetManager) {
    __android_log_print(ANDROID_LOG_ERROR, "selene", "testando essa porra\n");
    // Converte o AssetManager Java para o tipo nativo AAssetManager
    asset_manager = AAssetManager_fromJava(env, assetManager);
    // jmethodID getAssetsMethod = (*env)->GetMethodID(env, obj, "getAssets", "()Landroid/content/res/AssetManager;");
    // asset_manager = (AAssetManager*)(*env)->CallObjectMethod(env, obj, getAssetsMethod);
    // asset_manager = AAssetManager_fromJava(env, assetManagerJavaObj);
}


const char* read_from_asset_manager(AAssetManager* mgr, const char* path, size_t* size, int is_binary) {
    AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_STREAMING);
    if (!asset)
        return NULL;

    if (size) *size = AAsset_getLength(asset);
    is_binary = is_binary > 0 ? 1 : 0;
    char* buffer = (char*) malloc(*size + !is_binary);
    AAsset_read(asset, buffer, *size);
    if (!is_binary)
        buffer[*size] = '\0';
    AAsset_close(asset);

    return buffer;
}

int l_load_from_asset_manager(void* lua_state) {
    lua_State* L = (lua_State*)lua_state;
    const char* module_name = luaL_checkstring(L, 1);
    size_t len = strlen(module_name);
    char path[256];
    strcpy(path, module_name);
    for (int i = 0; i < len; i++) {
        if (path[i] == '.')
            path[i] = '/';
    }
    strcat(path, ".lua");

    __android_log_print(ANDROID_LOG_DEBUG, "selene", "[%s] load file: %s", module_name, path);

    SDL_RWops* rw = SDL_RWFromFile(path, "r");
    if (!rw) {
        lua_pushfstring(L, "[selene] failed to open file: %s", path);
        return 1;
    }
    size_t size = SDL_RWsize(rw);
    char* content = malloc(size);
    // const char* content = read_from_asset_manager(asset_manager, path, &size, 0);
    SDL_RWread(rw, content, 1, size);
    // content[size] = '\0';
    if (luaL_loadbuffer(L, content, size, path) != LUA_OK) {
        free((void*)content);
        lua_pushfstring(L, "[selene] failed to parse Lua file: %s", path);
        return 1;
    }
    free((void*)content);
    return 1;
}
#endif