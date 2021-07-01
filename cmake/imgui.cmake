include(FetchContent)
FetchContent_Declare(LIB_imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.83)

message("Fetching imgui from GitHub...")

FetchContent_MakeAvailable (LIB_imgui)
FetchContent_GetProperties(LIB_imgui
  SOURCE_DIR LIB_imgui_SOURCE_DIR)

file(GLOB SOURCES CONFIGURE_DEPENDS
  ${LIB_imgui_SOURCE_DIR}/*.cpp
  ${LIB_imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${LIB_imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
  ${LIB_imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
  )

add_library (imgui STATIC ${SOURCES})
target_include_directories(imgui PUBLIC
  ${LIB_imgui_SOURCE_DIR}
  ${LIB_imgui_SOURCE_DIR}/backends
  ${LIB_imgui_SOURCE_DIR}/misc/cpp/
  )
target_link_libraries(imgui PUBLIC d3dcompiler)
