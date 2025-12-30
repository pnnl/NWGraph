# -----------------------------------------------------------------------------
# Catch2 v3.x for unit testing
# -----------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
  GIT_TAG        "v3.5.2"
)

FetchContent_MakeAvailable(catch2)

# Make Catch2's CMake helpers available
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
