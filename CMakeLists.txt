##########################################################################
#	CMake Build Rules for  NWGraph (v0.3)                            #
##########################################################################
# Basic Usage:                                                           #
#   cmake ..								 #
#   make  								 #
#   make test                                                            #
# For more information about CMake, see http://www.cmake.org             #
##########################################################################
#
# Useful things to know 
# make VERBOSE=1
# cmake --trace
# cmake .. -DCMAKE_CXX_COMPILER=icpx
#
##########################################################################
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

# The cmake options don't seem to really work
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}") 
  message(FATAL_ERROR "In-source builds are disabled.
    Please create a subfolder and use `cmake ..` inside it.
    IMPORTANT: cmake will still have created CMakeCache.txt and CMakeFiles/*.
               You must delete them, or cmake will refuse to work.")
endif() # yes, end-markers and even else() need empty parens

# The cmake way, but still creates CMakeCache.txt and CMakeFiles/*, but
# does not provide any error message about what went wrong (nor how
# to remedy
# set(CMAKE_DISABLE_SOURCE_CHANGES ON)
# set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_CXX_STANDARD 20)

set(NWGRAPH_VERSION 0.3)

project(
  NWGRAPH
  VERSION ${NWGRAPH_VERSION}
  LANGUAGES CXX)

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# -----------------------------------------------------------------------------
# Extend the module path so we can find our custom modules
# -----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


# -----------------------------------------------------------------------------
# Build types and default flags
# -----------------------------------------------------------------------------

message("Architecture is ${CMAKE_HOST_SYSTEM_PROCESSOR}")

if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL arm64)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-elide-constructors -fconcepts-diagnostics-depth=3 " CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG " CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -g -DNDEBUG" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG " CACHE STRING "" FORCE)
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-elide-constructors -fconcepts-diagnostics-depth=3 " CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -DNDEBUG " CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -g -march=native -DNDEBUG" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -march=native -DNDEBUG " CACHE STRING "" FORCE)
endif()

# Default to Release build
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()

# Control compiler-specific flags.
include(CompilerFlags)

##

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
set(NWGRAPH_SANITIZE "" CACHE STRING "-fsanitize compiler options (e.g., =address)")
if (NWGRAPH_SANITIZE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=${NWGRAPH_SANITIZE}")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=${NWGRAPH_SANITIZE}")
endif (NWGRAPH_SANITIZE)

option(NWGRAPH_BUILD_BENCH "Determines whether to build benchmarks (GAP Suite and APB)." OFF)
option(NWGRAPH_BUILD_DOCS "Determines whether to build documentation (requires Sphinx, Doxygen)." OFF)
option(NWGRAPH_BUILD_EXAMPLES "Determines whether to build examples." OFF)
option(NWGRAPH_BUILD_TESTS "Determines whether to build tests." ON)
option(NWGRAPH_USE_TBBMALLOC "Link to tbbmalloc" OFF)

# Parallel backend selection: HPX, TBB (default), or none
# Only one of these should be ON at a time.
option(NWGRAPH_BACKEND_HPX "Use HPX for parallel execution" OFF)
option(NWGRAPH_BACKEND_NONE "Build without any parallel backend (sequential only)" OFF)

# Validate backend selection
if(NWGRAPH_BACKEND_HPX AND NWGRAPH_BACKEND_NONE)
  message(FATAL_ERROR "Cannot enable both NWGRAPH_BACKEND_HPX and NWGRAPH_BACKEND_NONE")
endif()

if(NWGRAPH_BACKEND_HPX)
  message(STATUS "NWGraph parallel backend: HPX")
elseif(NWGRAPH_BACKEND_NONE)
  message(STATUS "NWGraph parallel backend: none (sequential only)")
else()
  message(STATUS "NWGraph parallel backend: TBB (default)")
endif()

# -----------------------------------------------------------------------------
# Handle requests for parallel execution
# -----------------------------------------------------------------------------

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# TBB/HPX Root Path Configuration
# -----------------------------------------------------------------------------
# Users can specify paths via:
#   - CMake variables: TBB_ROOT, HPX_ROOT
#   - Environment variables: TBB_ROOT, TBBROOT, HPX_ROOT
#
# Example:
#   cmake .. -DTBB_ROOT=/opt/intel/tbb
#   cmake .. -DHPX_ROOT=/opt/hpx
#   TBB_ROOT=/opt/intel/tbb cmake ..

# TBB path hints (check environment variables)
if(NOT TBB_ROOT)
  if(DEFINED ENV{TBB_ROOT})
    set(TBB_ROOT $ENV{TBB_ROOT})
    message(STATUS "Using TBB_ROOT from environment: ${TBB_ROOT}")
  elseif(DEFINED ENV{TBBROOT})
    set(TBB_ROOT $ENV{TBBROOT})
    message(STATUS "Using TBBROOT from environment: ${TBB_ROOT}")
  endif()
endif()

# HPX path hints (check environment variables)
if(NOT HPX_ROOT)
  if(DEFINED ENV{HPX_ROOT})
    set(HPX_ROOT $ENV{HPX_ROOT})
    message(STATUS "Using HPX_ROOT from environment: ${HPX_ROOT}")
  endif()
endif()

# Only find/fetch TBB if using TBB backend (not HPX, not none)
if(NOT NWGRAPH_BACKEND_HPX AND NOT NWGRAPH_BACKEND_NONE)
  option(TBB_TEST "" OFF)
  option(BUILD_SHARED_LIBS "Enable building TBB shared library" ON)
  option(TBBMALLOC_BUILD "Enable tbbmalloc build" OFF)

  message(STATUS "BUILD_SHARED_LIBS " ${BUILD_SHARED_LIBS})

  if(NWGRAPH_FETCH_TBB)
    # Use fetch content rather than as a submodule
    include(FetchContent)
    FetchContent_Declare(
      oneTBB
      GIT_REPOSITORY https://github.com/oneapi-src/oneTBB.git
      #  GIT_TAG v2021.5.0
    )
    message(STATUS "Fetching TBB")
    FetchContent_MakeAvailable(oneTBB)
  else()
    # Find TBB with optional hints from TBB_ROOT
    if(TBB_ROOT)
      find_package(TBB REQUIRED HINTS ${TBB_ROOT} ${TBB_ROOT}/lib/cmake/TBB)
    else()
      find_package(TBB REQUIRED)
    endif()
  endif()
endif()

# -----------------------------------------------------------------------------
# Backend-specific setup
# -----------------------------------------------------------------------------
if(NWGRAPH_BACKEND_HPX)
  # --- HPX backend ---
  if(HPX_ROOT)
    find_package(HPX REQUIRED HINTS ${HPX_ROOT} ${HPX_ROOT}/lib/cmake/HPX)
  else()
    find_package(HPX REQUIRED)
  endif()

  add_library(nwgraph_parallel INTERFACE)
  target_compile_definitions(nwgraph_parallel INTERFACE NWGRAPH_BACKEND_HPX)
  target_link_libraries(nwgraph_parallel INTERFACE HPX::hpx Threads::Threads)

  # Alias for backwards compatibility with older CMakeLists that use nwgraph_tbb
  add_library(nwgraph_tbb ALIAS nwgraph_parallel)

elseif(NWGRAPH_BACKEND_NONE)
  # --- No parallel backend (sequential only) ---
  add_library(nwgraph_parallel INTERFACE)
  target_link_libraries(nwgraph_parallel INTERFACE Threads::Threads)

  # Alias for backwards compatibility
  add_library(nwgraph_tbb ALIAS nwgraph_parallel)

else()
  # --- TBB backend (default) ---
  add_library(nwgraph_tbb INTERFACE)
  target_compile_definitions(nwgraph_tbb INTERFACE NWGRAPH_BACKEND_TBB)
  target_link_libraries(nwgraph_tbb INTERFACE TBB::tbb Threads::Threads)
  if (NWGRAPH_USE_TBBMALLOC)
    target_link_libraries(nwgraph_tbb INTERFACE TBB::tbbmalloc)
  endif ()

  # Alias for new code
  add_library(nwgraph_parallel ALIAS nwgraph_tbb)

endif()

include(Docopt)

add_subdirectory(include)
  
if (NWGRAPH_BUILD_BENCH)
  add_subdirectory(bench)
endif()

# Documentation build targets
if (NWGRAPH_BUILD_DOCS)
  add_subdirectory(doc-src)
endif()

if (NWGRAPH_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

#Artifacts
option(NWGRAPH_BUILD_HPEC "Determines whether to build HPEC artifact." OFF)
if (NWGRAPH_BUILD_HPEC)
   add_subdirectory(${CMAKE_SOURCE_DIR}/../Artifacts/hpec hpec)
endif()

# Set up testing after all other includes (so third-party tests aren't included)
if (NWGRAPH_BUILD_TESTS)
#  enable_testing()
  include(CTest)
  add_subdirectory(test)
endif()


# https://cmake.org/cmake/help/latest/command/install.html
install(TARGETS nwgraph)
