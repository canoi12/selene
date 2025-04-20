include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)

include(FetchContent)

option(DOWNLOAD_SDL_SOURCE "Download the SDL source" OFF)
option(DOWNLOAD_SDL_APPLE "Download the SDL for Apple" OFF)
option(DOWNLOAD_SDL_MSVC "Download the SDL for Windows MSVC" OFF)
option(DOWNLOAD_SDL_MINGW "Download the SDL for Windows MINGW" OFF)
set(SDL_DOWNLOAD_DIR "${CACHE_DIR}/SDL2-${SDL_VERSION}")

# ===== URLs por plataforma =====
if (${DOWNLOAD_SDL_SOURCE})
    set(SDL_URL "https://www.libsdl.org/release/SDL2-${SDL_VERSION}.tar.gz")
    set(SDL_FINAL_DIR "${SDL_DOWNLOAD_DIR}/source")
elseif (${DOWNLOAD_SDL_APPLE})
    set(SDL_URL "https://www.libsdl.org/release/SDL2-${SDL_VERSION}.dmg")
    set(SDL_FINAL_DIR "${SDL_DOWNLOAD_DIR}/Apple")
elseif (${DOWNLOAD_SDL_MSVC})
    set(SDL_URL "https://www.libsdl.org/release/SDL2-devel-${SDL_VERSION}-VC.zip")
    set(SDL_FINAL_DIR "${SDL_DOWNLOAD_DIR}/MSVC")
else ()
    if(WIN32)
        set(SDL_URL "https://www.libsdl.org/release/SDL2-devel-${SDL_VERSION}-VC.zip")
        set(SDL_FINAL_DIR "${SDL_DOWNLOAD_DIR}/MSVC")
        set(DOWNLOAD_SDL_MSVC ON)
    elseif(APPLE)
        set(SDL_URL "https://www.libsdl.org/release/SDL2-${SDL_VERSION}.dmg")
        set(SDL_FINAL_DIR "${SDL_DOWNLOAD_DIR}/Apple")
        set(DOWNLOAD_SDL_APPLE ON)
    elseif(UNIX)  # Linux/BSD
        set(SDL_URL "https://www.libsdl.org/release/SDL2-${SDL_VERSION}.tar.gz")
        set(SDL_FINAL_DIR "${SDL_DOWNLOAD_DIR}/source")
        set(DOWNLOAD_SDL_SOURCE ON)
    endif()
endif()

message("SDL Directory: ${SDL_FINAL_DIR}")
if (NOT EXISTS ${SDL_FINAL_DIR})
    # ===== Download =====
    get_filename_component(SDL_FILENAME "${SDL_URL}" NAME)
    set(SDL_DEST "${SDL_DOWNLOAD_DIR}/${SDL_FILENAME}")

    if (NOT EXISTS ${SDL_DEST})
        message(STATUS "Downloading SDL2 ${SDL_VERSION} from: ${SDL_URL}")
        file(DOWNLOAD 
            "${SDL_URL}" 
            "${SDL_DEST}"
            SHOW_PROGRESS
            TLS_VERIFY ON
            STATUS DOWNLOAD_STATUS
        )

        # ===== Verificação =====
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        if(NOT STATUS_CODE EQUAL 0)
            list(GET DOWNLOAD_STATUS 1 ERROR_MSG)
            message(FATAL_ERROR "Download failed: ${ERROR_MSG}")
        endif()
    endif()


    # ===== Extração =====
    message(STATUS "Extracting SDL2...")
    if(WIN32)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${SDL_DEST}"
            WORKING_DIRECTORY "${SDL_DOWNLOAD_DIR}"
        )
        set(SDL_ROOT "${SDL_DOWNLOAD_DIR}/SDL2-${SDL_VERSION}")
    elseif(UNIX AND NOT APPLE)
        execute_process(
            COMMAND tar -xzf "${SDL_DEST}" --strip-components=1
            WORKING_DIRECTORY "${SDL_DOWNLOAD_DIR}"
        )
        set(SDL_ROOT "${SDL_DOWNLOAD_DIR}")
    endif()
    if (NOT APPLE AND NOT DOWNLOAD_SDL_APPLE)
        file(RENAME ${SDL_ROOT} ${SDL_FINAL_DIR})
    endif ()
endif ()