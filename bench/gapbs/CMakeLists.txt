# =============================================================================
# GAP Benchmark Suite Implementations
# =============================================================================
# Reference: Beamer et al., "The GAP Benchmark Suite", IISWC 2020
# https://scottbeamer.net/pubs/beamer-iiswc2020.pdf
# =============================================================================

# -----------------------------------------------------------------------------
# The date library from MIT provides C++20 std date functionality in C++17
# -----------------------------------------------------------------------------
include(Date)

# -----------------------------------------------------------------------------
# Generate the configuration files used for logging, and -I its built path.
# -----------------------------------------------------------------------------
get_filename_component(NWGRAPH_CXX_COMPILER ${CMAKE_CXX_COMPILER} NAME)
configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# -----------------------------------------------------------------------------
# Create an interface library for the benchmarks to depend on.
# -----------------------------------------------------------------------------
add_library(gapbs_lib INTERFACE)
target_sources(gapbs_lib INTERFACE
  common.hpp
  Log.hpp
)
target_include_directories(gapbs_lib INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(gapbs_lib INTERFACE nwgraph docopt date::date)

# -----------------------------------------------------------------------------
# NWGRAPH driver to process edgelists.
# -----------------------------------------------------------------------------
add_executable(process_edge_list ProcessEdgelist.cpp)
target_link_libraries(process_edge_list nwgraph docopt)
add_dependencies(bench process_edge_list)

# -----------------------------------------------------------------------------
# NWGRAPH driver to verify that we're correctly generating source lists.
# -----------------------------------------------------------------------------
add_executable(sources SourceVerifier.cpp)
target_link_libraries(sources gapbs_lib)
add_dependencies(bench sources)

# -----------------------------------------------------------------------------
# GAP Benchmark Suite drivers
# -----------------------------------------------------------------------------

# Betweenness Centrality
add_executable(bc bc.cpp)
target_link_libraries(bc gapbs_lib)
add_dependencies(bench bc)

# Breadth-First Search
add_executable(bfs bfs.cpp)
target_link_libraries(bfs gapbs_lib)
add_dependencies(bench bfs)

# Connected Components
add_executable(cc cc.cpp)
target_link_libraries(cc gapbs_lib)
add_dependencies(bench cc)

# PageRank
add_executable(pr pr.cpp)
target_link_libraries(pr gapbs_lib)
add_dependencies(bench pr)

# Single-Source Shortest Path
add_executable(sssp sssp.cpp)
target_link_libraries(sssp gapbs_lib)
add_dependencies(bench sssp)

# Triangle Counting
add_executable(tc tc.cpp)
target_link_libraries(tc gapbs_lib)
add_dependencies(bench tc)

# Jaccard Similarity (disabled - uses nlohmann_json, needs conversion to Log.hpp)
# add_executable(js js.cpp)
# target_link_libraries(js gapbs_lib)
# add_dependencies(bench js)
