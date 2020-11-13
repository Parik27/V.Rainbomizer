include(FetchContent)
FetchContent_Declare(FMT
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 7.1.2)

message("Fetching fmt from GitHub...")
FetchContent_MakeAvailable(FMT)
