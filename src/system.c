#include "selene.h"

static BEGIN_FUNCTION(system, GetOS)
#if defined(__EMSCRIPTEN__)
    PUSH_STRING("Emscripten");
#elif defined(OS_WIN)
    PUSH_STRING("Windows");
#elif defined(OS_OSX)
    PUSH_STRING("OSX");
#elif defined(OS_LINUX)
    PUSH_STRING("Linux");
#elif defined(OS_BSD)
    PUSH_STRING("BSD");
#elif defined(OS_UNIX)
    PUSH_STRING("Unix");
#endif
END_FUNCTION(1)

static BEGIN_FUNCTION(system, GetArch)
#if defined(ARCH_X86)
    PUSH_STRING("x86");
#elif defined(ARCH_X64)
    PUSH_STRING("x64");
#elif defined(ARCH_ARM)
    PUSH_STRING("arm");
#endif
END_FUNCTION(1)

BEGIN_MODULE(system)
    BEGIN_REG(system)
        REG_FIELD(system, GetOS),
        REG_FIELD(system, GetArch),
    END_REG()
    NEW_MODULE(system);
END_MODULE(1)
