# ---------------------------------------------------------------------------
# Vulkan discovery.  Included only when SELENE_NO_VULKAN is OFF (see
# src/CMakeLists.txt).
#
# On success exports ``SELENE_VULKAN_LIBRARY`` and the include directory; on
# failure defines ``SELENE_NO_VULKAN`` so the renderer/window sources compile
# the Vulkan paths out.
# ---------------------------------------------------------------------------
if (WIN32)
    find_library(VULKAN_LIBRARY
        NAMES vulkan vulkan-1
        PATHS
        "$ENV{VULKAN_SDK}/Lib"   # Windows Vulkan SDK default location
    )
    find_path(VULKAN_INCLUDE_DIR
        NAMES vulkan/vulkan.h
        PATHS
        "$ENV{VULKAN_SDK}/Include"  # Windows Vulkan SDK
    )
else ()
    find_library(VULKAN_LIBRARY NAMES vulkan vulkan-1
        PATHS
        /usr/lib/x86_64-linux-gnu
        /usr/lib
        /lib/x86_64-linux-gnu
        /lib
        NO_DEFAULT_PATH
    )
    find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h
        PATHS
        /usr/include
        NO_DEFAULT_PATH
    )
endif ()

if (VULKAN_LIBRARY AND VULKAN_INCLUDE_DIR)
    message("Using Vulkan: ${VULKAN_LIBRARY} ${VULKAN_INCLUDE_DIR}")
    set(SELENE_VULKAN_LIBRARY ${VULKAN_LIBRARY})
    include_directories(${VULKAN_INCLUDE_DIR})
else ()
    message("Failed to find Vulkan library")
    add_definitions(-DSELENE_NO_VULKAN=1)
endif ()
