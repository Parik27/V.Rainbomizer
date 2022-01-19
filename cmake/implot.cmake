include(FetchContent)
FetchContent_Declare(LIB_implot
  GIT_REPOSITORY https://github.com/epezent/implot
  GIT_TAG v0.12)

message("Fetching implot from GitHub...")

FetchContent_MakeAvailable (LIB_implot)
FetchContent_GetProperties(LIB_implot
  SOURCE_DIR LIB_implot_SOURCE_DIR)

file(GLOB SOURCES CONFIGURE_DEPENDS
  ${LIB_implot_SOURCE_DIR}/implot.cpp
  ${LIB_implot_SOURCE_DIR}/implot_items.cpp
  )

add_library (implot STATIC ${SOURCES})
target_include_directories(implot PUBLIC
  ${LIB_implot_SOURCE_DIR}
  )

target_link_libraries(implot PUBLIC imgui)
