#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#if defined(ANDROID) || defined(__ANDROID__)
#define OS_ANDROID 1
#elif defined(__EMSCRIPTEN__)
#define OS_EMSCRIPTEN 1
#elif defined(_WIN32) || defined(_WIN64)
#define OS_WIN 1
#elif defined(__APPLE__)
#define OS_OSX 1
#elif defined(__linux__)
#define OS_LINUX 1
#define OS_UNIX 1
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#define OS_BSD 1
#elif defined(__unix__)
#define OS_UNIX 1
#endif

#if defined(__i386__) || defined(_M_IX86)
#define ARCH_X86 1
#elif defined(__amd64__) || defined(_M_X64)
#define ARCH_X64 1
#elif defined(__arm__) || defined(__ARM__) || defined(ARM) || defined(__ARM) || defined(__arm)
#define ARCH_ARM 1
#else
#endif

#ifdef OS_WIN
#define SDL_MAIN_HANDLED
#include <windows.h>
#elif defined(OS_EMSCRIPTEN)
#include <emscripten.h>
#elif defined(OS_ANDROID)
#include <android/log.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

extern AAssetManager* asset_manager;

const char* read_from_asset_manager(AAssetManager* mgr, const char* path, size_t* size, int is_binary);

extern int l_load_from_asset_manager(void* lua_state);
#endif

#endif /* PLATFORMS_H_ */