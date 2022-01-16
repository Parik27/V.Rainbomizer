include(cmake/imgui.cmake)
include(cmake/fmt.cmake)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/debug/*.cc)

target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DENABLE_DEBUG_MENU)
target_link_libraries (${PROJECT_NAME} PUBLIC fmt imgui dxguid dwmapi imm32)

