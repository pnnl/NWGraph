##########################################################################
#	CMake Build Rules for the NW Graph (v0.3)                        #
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

set(NW_GRAPH_VERSION 0.3)

project(
  NW_GRAPH
  VERSION ${NW_GRAPH_VERSION}
  LANGUAGES CXX)

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
set(NW_GRAPH_SANITIZE "" CACHE STRING "-fsanitize compiler options (e.g., =address)")
if (NW_GRAPH_SANITIZE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=${NW_GRAPH_SANITIZE}")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=${NW_GRAPH_SANITIZE}")
endif (NW_GRAPH_SANITIZE)

option(NW_GRAPH_BUILD_APBS "Determines whether to build abstraction penalty benchmarks." OFF)
option(NW_GRAPH_BUILD_BENCH "Determines whether to build performance benchmarks." OFF)
# option(NW_GRAPH_BUILD_DOCS "Determines whether to build documentation." OFF)
option(NW_GRAPH_BUILD_EXAMPLES "Determines whether to build examples." OFF)
option(NW_GRAPH_BUILD_TESTS "Determines whether to build tests." ON)
option(NW_GRAPH_USE_TBBMALLOC "Link to tbbmalloc" OFF)

add_subdirectory(include)

include(Docopt)
  
if (NW_GRAPH_BUILD_APBS)
  add_subdirectory(apb)
endif()

if (NW_GRAPH_BUILD_BENCH)
  add_subdirectory(bench)
endif()

# Documentation is built specially in CI for now
# if (NW_GRAPH_BUILD_DOCS)
#   add_subdirectory(doc-src)
# endif()

if (NW_GRAPH_BUILD_EXAMPLES)
  add_subdirectory(examples/imdb)
endif()

#Artifacts
option(NW_GRAPH_BUILD_HPEC "Determines whether to build HPEC artifact." OFF)
if (NW_GRAPH_BUILD_HPEC)
   add_subdirectory(${CMAKE_SOURCE_DIR}/../Artifacts/hpec hpec)
endif()

# Set up testing after all other includes (so third-party tests aren't included)
if (NW_GRAPH_BUILD_TESTS)
#  enable_testing()
  include(CTest)
  add_subdirectory(test)
endif()

# https://cmake.org/cmake/help/latest/command/install.html
install(TARGETS nw_graph)
