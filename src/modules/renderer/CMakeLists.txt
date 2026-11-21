# renderer module: backend-agnostic core plus the per-API drivers.
# All drivers are globbed unconditionally; each one guards itself internally
# on OS/option (OS_WIN, SELENE_NO_D3D11/D3D12, SELENE_NO_VULKAN,
# SELENE_NO_OPENGL), exactly like before.
file(GLOB RENDERER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/*.cpp)
selene_add_module_sources(${RENDERER_SOURCES})
if (MSVC)
    source_group("Source Files\\renderer" FILES ${RENDERER_SOURCES})
endif ()
