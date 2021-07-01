include(cmake/imgui.cmake)
include(cmake/fmt.cmake)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/debug/*.cc)

target_sources("rainbomizer" PUBLIC ${SOURCES})
target_compile_definitions("rainbomizer" PRIVATE -DENABLE_DEBUG_MENU)
target_link_libraries ("rainbomizer" PUBLIC fmt imgui dxguid)

