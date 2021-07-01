cmake_minimum_required(VERSION 3.1.5)
project(rainbomizer_v)

option(ENABLE_DEBUG_MENU "Enables an imgui-menu for debugging purposes")

# C++20
if (MSVC_VERSION GREATER_EQUAL "1900")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  # Because CMAKE_MSVC_RUNTIME_LIBRARY doesn't work for whatever reason
  set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()

  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set (CMAKE_CXX_STANDARD 17)

# Sources
include_directories(include lib src)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  src/common/*.cc
  src/exceptions/*.cc
  src/misc/*.cc
  src/mission/*.cc
  src/sounds/*.cc
  src/peds/*.cc
  src/vehicles/*.cc
  src/weapons/*.cc
  lib/*.cc
  lib/Patterns/*.cpp)

add_library("rainbomizer" SHARED ${SOURCES} )

if (ENABLE_DEBUG_MENU)
  include(cmake/DebugImgui.cmake)
endif()

# Properties
set_target_properties("rainbomizer" PROPERTIES SUFFIX ".asi")
target_link_libraries("rainbomizer" PUBLIC dbghelp)
target_compile_definitions("rainbomizer" PUBLIC NOMINMAX)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

else()
  target_compile_features("rainbomizer" PRIVATE cxx_auto_type)
endif()
