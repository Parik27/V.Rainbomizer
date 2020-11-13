include(cmake/uWebSockets.cmake)
include(cmake/nlohmannJson.cmake)
include(cmake/fmt.cmake)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/debug/*.cc)

# Don't need these
set_target_properties(
  example minigzip zlib uv
  PROPERTIES EXCLUDE_FROM_ALL true)

if (TARGET example64)
  set_target_properties(
    example64 minigzip64
    PROPERTIES EXCLUDE_FROM_ALL true)
endif()

target_sources("rainbomizer" PUBLIC ${SOURCES})
target_compile_definitions("rainbomizer" PRIVATE -DENABLE_DEBUG_SERVER)
target_precompile_headers("rainbomizer" PRIVATE
  <nlohmann/json.hpp>
  <App.h>
  )
target_link_libraries ("rainbomizer" PUBLIC
  uWebSockets
  nlohmann_json::nlohmann_json
  fmt)
