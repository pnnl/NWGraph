# -----------------------------------------------------------------------------
# The date library from MIT provides C++20 std date functionality in C++17
# -----------------------------------------------------------------------------
include(Date)
# find_package(Filesystem REQUIRED Final Experimental)
# message(STATUS ${CXX_FILESYSTEM_IS_EXPERIMENTAL})

# -----------------------------------------------------------------------------
# Generate the configuration files used for logging, and -I its built path.
# -----------------------------------------------------------------------------
get_filename_component(NW_GRAPH_CXX_COMPILER ${CMAKE_CXX_COMPILER} NAME)
configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# -----------------------------------------------------------------------------
# Create an interface library for the benchmarks to depend on.
# -----------------------------------------------------------------------------
add_library(bench_lib INTERFACE)
target_sources(bench_lib INTERFACE
  common.hpp
  Log.hpp
  #config.h
  )
target_include_directories(bench_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(bench_lib INTERFACE nw_graph docopt date::date nlohmann_json::nlohmann_json) 

find_package(nlohmann_json 3.2.0 QUIET)
if (NOT nlohmann_json_FOUND)
  message("Installed json not found -- fetching")
  FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.7.3)

  FetchContent_GetProperties(json)
  if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif()



add_custom_target(bench)

# -----------------------------------------------------------------------------
# NW_GRAPH driver to process edgelists.
# -----------------------------------------------------------------------------
add_executable(process_edge_list ProcessEdgelist.cpp)
target_link_libraries(process_edge_list nw_graph docopt)
add_dependencies(bench process_edge_list)


# -----------------------------------------------------------------------------
# NW_GRAPH driver to verify that we're correctly generating source lists.
# -----------------------------------------------------------------------------
# Set policy to suppress warnings
cmake_policy(SET CMP0115 NEW)

add_executable(sources SourceVerifier.cpp)
target_link_libraries(sources bench_lib)
add_dependencies(bench sources)

# -----------------------------------------------------------------------------
# NW_GRAPH benchmark drivers
# -----------------------------------------------------------------------------

add_executable(bc.exe bc.cpp)
target_link_libraries(bc.exe bench_lib)
add_dependencies(bench bc.exe)

add_executable(bfs.exe bfs.cpp)
target_link_libraries(bfs.exe bench_lib)
add_dependencies(bench bfs.exe)

add_executable(cc.exe cc.cpp) 
target_link_libraries(cc.exe bench_lib) 
add_dependencies(bench cc.exe)

# add_executable(js.exe js.cpp)
# target_link_libraries(js.exe bench_lib)
# add_dependencies(bench js.exe)

add_executable(pr.exe pr.cpp)
target_link_libraries(pr.exe bench_lib)
add_dependencies(bench pr.exe)

add_executable(sssp.exe sssp.cpp)
target_link_libraries(sssp.exe bench_lib) 
add_dependencies(bench sssp.exe)

add_executable(tc.exe tc.cpp)
target_link_libraries(tc.exe bench_lib nlohmann_json::nlohmann_json)
add_dependencies(bench tc.exe)
