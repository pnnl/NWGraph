# -----------------------------------------------------------------------------
# Allow our executables to use docopt.
# -----------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  date
  GIT_REPOSITORY https://github.com/HowardHinnant/date.git
  GIT_TAG master)

FetchContent_MakeAvailable(date)

target_compile_options(date INTERFACE -Wno-deprecated-declarations)
