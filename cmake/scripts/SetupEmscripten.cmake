include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)

if(NOT EXISTS "${EMSDK_DIR}")
    file(DOWNLOAD "https://github.com/emscripten-core/emsdk/archive/main.zip" "${CACHE_DIR}/emsdk.zip")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${CACHE_DIR}/emsdk.zip" WORKING_DIRECTORY "${CACHE_DIR}")
    file(RENAME "${CACHE_DIR}/emsdk-main" "${EMSDK_DIR}")
endif()
if (WIN32)
    find_program(EMSDK_EXEC
            NAMES "emsdk.bat"
            PATHS "${EMSDK_DIR}"
            REQUIRED
    )
    set(EXEC cmd /c)
else ()
    find_program(EMSDK_EXEC
            NAMES "emsdk"
            PATHS "${EMSDK_DIR}"
            REQUIRED
    )
    set(EXEC bash -c)
endif ()

execute_process(COMMAND ${EXEC} "${EMSDK_EXEC} install latest && ${EMSDK_EXEC} activate latest" WORKING_DIRECTORY "${EMSDK_DIR}")

message(STATUS "Setup Emscripten in: ${EMSDK_DIR}")