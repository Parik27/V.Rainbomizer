include(FetchContent)
FetchContent_Declare(LIB_MINHOOK
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook
  GIT_TAG 6ffd0190232c670fa08e01dd4f2907ee5785a335)

message("Fetching MinHook from GitHub...")
FetchContent_MakeAvailable(LIB_MINHOOK)
