# -----------------------------------------------------------------------------
# Define the primary NW_GRAPH target.
# -----------------------------------------------------------------------------
#
# This target keeps track of all of the header sources for NW_GRAPH, and sets up all
# of the associated flags and dependencies such that we can correctly compile
# and link executables that are dependent on NW_GRAPH. Ultimately this target will
# control the install target too, once we have one.
#
# Executable targets should be defined with `target_link_libraries(exec nw_graph)`
# and then they'll have the appropriate `-I`, `-D`, and `-l` stuff set
# automagically though the transitive dependency infrastructure.

# @note
#   If NW_GRAPH starts to have .cpp sources, change this from an interface library to
#   a STATIC (or DYNAMIC) library, and add those sources here. Change other
#   instances of INTERFACE to PUBLIC.
# @note

include(FetchContent)

# FetchContent_Declare(nwut
#   GIT_REPOSITORY     git@github.com:NWmath/NWut.git
#   GIT_TAG            master
#   # URL ${CMAKE_SOURCE_DIR}/contrib/nwut-something.zip
#   # URL_MD5 HASH_FOR_ZIP^
#   )
# FetchContent_MakeAvailable(nwut)

add_library(nw_graph INTERFACE)

# Require C++20
target_compile_features(nw_graph INTERFACE cxx_std_20)

# Add an alias for external uses
add_library(nw::graph ALIAS nw_graph)

# make sure we have the right include path
target_include_directories(nw_graph INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# Extend this as needed.
target_sources(nw_graph
  INTERFACE
  nwgraph/adaptors/back_edge_range.hpp
  nwgraph/adaptors/bfs_edge_range.hpp
  nwgraph/adaptors/bfs_range.hpp
  nwgraph/adaptors/dag_range.hpp
  nwgraph/adaptors/dfs_range.hpp
  nwgraph/adaptors/edge_range.hpp
  nwgraph/adaptors/filtered_bfs_range.hpp
  nwgraph/adaptors/neighbor_range.hpp
  nwgraph/adaptors/new_dfs_range.hpp
  nwgraph/adaptors/plain_range.hpp
  nwgraph/adaptors/random_range.hpp
  nwgraph/adaptors/reverse.hpp
  nwgraph/adaptors/vertex_range.hpp
  nwgraph/adaptors/worklist.hpp
  nwgraph/algorithms/betweenness_centrality.hpp
  nwgraph/algorithms/bfs.hpp
  nwgraph/algorithms/boykov_kolmogorov.hpp
  nwgraph/algorithms/connected_components.hpp
  nwgraph/algorithms/dag_based_mis.hpp
  nwgraph/algorithms/delta_stepping.hpp
  nwgraph/algorithms/dijkstra.hpp
  nwgraph/algorithms/jones_plassmann_coloring.hpp
  nwgraph/algorithms/k_core.hpp
  nwgraph/algorithms/kruskal.hpp
  nwgraph/algorithms/max_flow.hpp
  nwgraph/algorithms/maximal_independent_set.hpp
  nwgraph/algorithms/page_rank.hpp
  nwgraph/algorithms/prim.hpp
  nwgraph/algorithms/spMatspMat.hpp
  nwgraph/algorithms/triangle_count.hpp
  nwgraph/experimental/algorithms/betweenness_centrality.hpp
  nwgraph/experimental/algorithms/bfs.hpp
  nwgraph/experimental/algorithms/connected_components.hpp
  nwgraph/experimental/algorithms/jaccard.hpp
  nwgraph/experimental/algorithms/jones_plassmann_coloring.hpp
  nwgraph/experimental/algorithms/page_rank.hpp
  nwgraph/experimental/algorithms/delta_stepping.hpp
  nwgraph/experimental/algorithms/triangle_count.hpp
  nwgraph/containers/aos.hpp
  nwgraph/containers/compressed.hpp
  nwgraph/containers/soa.hpp
  nwgraph/generators/configuration_model.hpp
  nwgraph/io/mmio.hpp
  nwgraph/util/disjoint_set.hpp
  nwgraph/util/print_types.hpp
  nwgraph/util/provenance.hpp
  nwgraph/util/proxysort.hpp
  nwgraph/util/tag_invoke.hpp
  nwgraph/util/timer.hpp
  nwgraph/util/util.hpp
  nwgraph/util/util_par.hpp
  nwgraph/build.hpp
  nwgraph/edge_list.hpp
  nwgraph/graph_base.hpp
  nwgraph/graph_concepts.hpp
  nwgraph/graph_traits.hpp
  nwgraph/volos.hpp
  nwgraph/adjacency.hpp
  nwgraph/vovos.hpp
  nwgraph/csr.hpp
  nwgraph/csc.hpp
  nwgraph/coo.hpp)


# -----------------------------------------------------------------------------
# Handle requests for parallel execution
# -----------------------------------------------------------------------------

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(TBB REQUIRED)

# find_package(TBB)
# if (not TBB_Found)
#   FetchContent(
#     TBB_STUFF)
# endif()

find_package(Threads REQUIRED)
target_compile_definitions(nw_graph INTERFACE EXECUTION_POLICY)
target_link_libraries(nw_graph INTERFACE TBB::tbb Threads::Threads)
if (NW_GRAPH_USE_TBBMALLOC)
  target_link_libraries(nw_graph INTERFACE TBB::tbbmalloc) 
endif ()



# -----------------------------------------------------------------------------
# Ranges V3
# -----------------------------------------------------------------------------

# FetchContent_Declare(range-v3
#   GIT_REPOSITORY     https://github.com/ericniebler/range-v3
#   GIT_TAG            master)
# FetchContent_MakeAvailable(range-v3)
# target_link_libraries(nw_graph INTERFACE range-v3::range-v3)


# -----------------------------------------------------------------------------
# Handle Apple-specific things
# -----------------------------------------------------------------------------
if (APPLE)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_definitions(nw_graph INTERFACE _Atomic=volatile)
  endif()
endif()

