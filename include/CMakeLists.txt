# -----------------------------------------------------------------------------
# Define the primary NWGRAPH target.
# -----------------------------------------------------------------------------
#
# This target keeps track of all of the header sources for NWGRAPH, and sets up all
# of the associated flags and dependencies such that we can correctly compile
# and link executables that are dependent on NWGRAPH. Ultimately this target will
# control the install target too, once we have one.
#
# Executable targets should be defined with `target_link_libraries(exec nwgraph)`
# and then they'll have the appropriate `-I`, `-D`, and `-l` stuff set
# automagically though the transitive dependency infrastructure.

# @note
#   If NWGRAPH starts to have .cpp sources, change this from an interface library to
#   a STATIC (or DYNAMIC) library, and add those sources here. Change other
#   instances of INTERFACE to PUBLIC.
# @note

add_library(nwgraph INTERFACE)
target_link_libraries(nwgraph INTERFACE nwgraph_tbb)

# Require C++20
target_compile_features(nwgraph INTERFACE cxx_std_20)

# Add an alias for external uses
add_library(nw::graph ALIAS nwgraph)

# make sure we have the right include path
target_include_directories(nwgraph INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# Extend this as needed.
target_sources(nwgraph
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
# Handle Apple-specific things
# -----------------------------------------------------------------------------
if (APPLE)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_definitions(nwgraph INTERFACE _Atomic=volatile)
  endif()
endif()

