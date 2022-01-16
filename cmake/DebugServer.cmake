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

target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DENABLE_DEBUG_SERVER)
target_precompile_headers(${PROJECT_NAME} PRIVATE
  <nlohmann/json.hpp>
  <App.h>
  )
target_link_libraries (${PROJECT_NAME} PUBLIC
  uWebSockets
  nlohmann_json::nlohmann_json
  fmt)
