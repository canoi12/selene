include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)

if (NOT EXISTS ${ANDROID_SDK_DIR})
    file(MAKE_DIRECTORY ${ANDROID_SDK_DIR})
endif ()

if(WIN32)
    set(CMDLT_FILE "commandlinetools-win-11076708_latest.zip")
elseif(APPLE)
    set(CMDLT_FILE "commandlinetools-mac-11076708_latest.zip")
else()
    set(CMDLT_FILE "commandlinetools-linux-11076708_latest.zip")
endif()

set(CMDLINE_TOOLS_URL "https://dl.google.com/android/repository/${CMDLT_FILE}")

if (NOT EXISTS "${CACHE_DIR}/${CMDLT_FILE}")
    message(STATUS "Downloading Android Commandline Tools...")
    file(DOWNLOAD
        "${CMDLINE_TOOLS_URL}"
        "${CACHE_DIR}/${CMDLT_FILE}"
        SHOW_PROGRESS
        TLS_VERIFY ON
    )
endif ()

if(NOT EXISTS "${ANDROID_SDK_DIR}/cmdline-tools/latest")
    
    message(STATUS "Extracting Android SDK...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${CACHE_DIR}/${CMDLT_FILE}"
        WORKING_DIRECTORY "${ANDROID_SDK_DIR}"
        RESULT_VARIABLE extract_result
    )

    if(NOT extract_result EQUAL 0)
        message(FATAL_ERROR "Falha ao extrair Commandline Tools!")
    endif()

    file(RENAME 
        "${ANDROID_SDK_DIR}/cmdline-tools/" 
        "${ANDROID_SDK_DIR}/latest"
    )

    file(MAKE_DIRECTORY "${ANDROID_SDK_DIR}/cmdline-tools")

    file(RENAME 
        "${ANDROID_SDK_DIR}/latest" 
        "${ANDROID_SDK_DIR}/cmdline-tools/latest"
    )
endif()

if(WIN32)
    set(SDK_MANAGER "${ANDROID_SDK_DIR}/cmdline-tools/latest/bin/sdkmanager.bat")
else()
    set(SDK_MANAGER "${ANDROID_SDK_DIR}/cmdline-tools/latest/bin/sdkmanager")
endif()

message(STATUS "Installing Android SDK (Platform: ${ANDROID_PLATFORM}, NDK: ${ANDROID_NDK_VERSION})...")

set(ANDROID_PACKAGES
    "platform-tools"
    "platforms\;android-${ANDROID_MIN_SDK_VERSION}"
    "platforms\;android-${ANDROID_COMPILE_SDK_VERSION}"
    "build-tools\;${ANDROID_BUILD_TOOLS_VERSION}"
    "ndk\;${ANDROID_NDK_VERSION}"
)
message("SDKManager: ${SDK_MANAGER}")
function(android_install_package pkg)
    message(STATUS "Installing: ${pkg}")

    if(WIN32)
        file(TO_NATIVE_PATH "${SDK_MANAGER}" NATIVE_SDK_MANAGER)
        # string(REPLACE "\\" "/" NATIVE_SDK_MANAGER "${NATIVE_SDK_MANAGER}")  # Uniformiza barras
        execute_process(
            COMMAND cmd /c "echo y | ${NATIVE_SDK_MANAGER} --install ${pkg}"
            WORKING_DIRECTORY "${ANDROID_SDK_DIR}"
            RESULT_VARIABLE result
        )
    else()
        execute_process(
            COMMAND bash -c "echo \"y\" | ${SDK_MANAGER} --install \"${pkg}\""
            WORKING_DIRECTORY "${ANDROID_SDK_DIR}"
            RESULT_VARIABLE result
        )
    endif()

    # If fails, try again
    if(NOT result EQUAL 0)
        message(WARNING "Trying interactive installation...")
        execute_process(
            COMMAND ${SDK_MANAGER} --install ${pkg}
            WORKING_DIRECTORY "${ANDROID_SDK_DIR}"
            RESULT_VARIABLE result
        )
    endif()

    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to install ${pkg}")
    endif()
endfunction()

foreach(pkg IN LISTS ANDROID_PACKAGES)
    # string(REPLACE ";" "\\;" escaped_pkg "${pkg}")
    android_install_package("${pkg}")
endforeach()

set(ENV{ANDROID_SDK_ROOT} "${ANDROID_SDK_DIR}")
set(ENV{ANDROID_NDK_HOME} "${ANDROID_SDK_DIR}/ndk/${ANDROID_NDK_VERSION}")

message(STATUS "Configured Android SDK in: ${ANDROID_SDK_DIR}")