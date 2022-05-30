# -----------------------------------------------------------------------------
# Allow our executables to use docopt.
# -----------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
  GIT_TAG        "v2.13.4"
)

FetchContent_MakeAvailable(catch2)
