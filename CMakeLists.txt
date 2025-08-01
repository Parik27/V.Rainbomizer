cmake_minimum_required(VERSION 3.15)
project(V.Rainbomizer)

include(cmake/Minhook.cmake)

option(ENABLE_DEBUG_MENU "Enables an imgui-menu for debugging purposes")

# C++20
if (MSVC_VERSION GREATER_EQUAL "1900")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/Zc:preprocessor)
  endif()
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set (CMAKE_CXX_STANDARD 23)

# Sources
include_directories(include lib src)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  src/common/*.cc
  src/exceptions/*.cc
  src/colours/*.cc
  src/misc/*.cc
  src/mission/*.cc
  src/sounds/*.cc
  src/peds/*.cc
  src/vehicles/*.cc
  src/weapons/*.cc
  lib/*.cc
  lib/Patterns/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES} )

if (ENABLE_DEBUG_MENU)
  include(cmake/DebugImgui.cmake)
endif()

# Properties
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".asi")
target_link_libraries(${PROJECT_NAME} PUBLIC dbghelp minhook)

target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
