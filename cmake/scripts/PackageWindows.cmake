include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)
if (NOT VS_GENERATOR)
    set(VS_GENERATOR "Visual Studio 17 2022")
endif ()
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif ()
if (NOT PLATFORMS)
    set(PLATFORMS "Win32;x64")
endif ()
if (NOT BUILD_DIR)
    set(BUILD_DIR "build/windows")
endif ()
file(REMOVE_RECURSE ${BUILD_DIR}/dist)
file(MAKE_DIRECTORY ${BUILD_DIR}/dist)
# execute_process(
#     ${CMAKE_COMMAND}
# )
# file(REMOVE ${CMAKE_SOURCE_DIR}/build)
foreach(platform IN LISTS PLATFORMS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -B ${BUILD_DIR}/${platform} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -G ${VS_GENERATOR} -A ${platform}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE config_result
    )
    if(NOT config_result EQUAL 0)
        message(FATAL_ERROR "Failed to generate config Window ${platform} project (code: ${config_result})")
    else()
        message(STATUS "Windows ${platform} project config generated with success in: ${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${platform}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${BUILD_DIR}/${platform} --target package --config ${CMAKE_BUILD_TYPE}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE build_result
    )
    if(NOT build_result EQUAL 0)
        message(FATAL_ERROR "Failed to generate build Window x86 project (code: ${build_result})")
    else()
        message(STATUS "Windows ${platform} project build generated with success in: ${CMAKE_SOURCE_DIR}/${BUILD_DIR}/${platform}")
    endif()
    
    file(GLOB BUILD_FILES ${BUILD_DIR}/${platform}/*.zip ${BUILD_DIR}/${platform}/*.exe)
    file(COPY ${BUILD_FILES} DESTINATION ${BUILD_DIR}/dist)
endforeach()