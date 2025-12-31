

include(Catch2)
include(CTest)
include(Catch)

macro (nwgraph_add_exe EXENAME)
  add_executable(${EXENAME}.exe ${EXENAME}.cpp)
  target_link_libraries(${EXENAME}.exe nwgraph)
endmacro (nwgraph_add_exe)

macro (nwgraph_add_test TESTNAME)
  add_executable(${TESTNAME}.exe ${TESTNAME}.cpp)
  target_link_libraries(${TESTNAME}.exe nwgraph Catch2::Catch2WithMain)
  target_compile_definitions(${TESTNAME}.exe PRIVATE DATA_DIR="${CMAKE_SOURCE_DIR}/data/")
  target_compile_definitions(${TESTNAME}.exe PRIVATE DATA_FILE="${CMAKE_SOURCE_DIR}/data/karate.mtx")
  catch_discover_tests(${TESTNAME}.exe)
endmacro (nwgraph_add_test)

# =============================================================================
# Catch2 Unit Tests
# =============================================================================
# Tests are organized by category for clarity and maintainability.

# -----------------------------------------------------------------------------
# Container Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(adjacency_test)
nwgraph_add_test(aos_test)
nwgraph_add_test(compressed_test)
nwgraph_add_test(edge_list_test)
nwgraph_add_test(soa_test)
nwgraph_add_test(vofos_test)
nwgraph_add_test(volos_test)
nwgraph_add_test(vov_test)
nwgraph_add_test(zip_test)

# -----------------------------------------------------------------------------
# Graph Concept and Traits Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(graph_adaptor_test)
nwgraph_add_test(graph_base_test)
nwgraph_add_test(graph_concepts_test)
nwgraph_add_test(graph_traits_test)
nwgraph_add_test(size_test)

# -----------------------------------------------------------------------------
# Algorithm Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(bfs_test_0)
nwgraph_add_test(bfs_test_1)
nwgraph_add_test(connected_component_test)
nwgraph_add_test(dijkstra_test)
nwgraph_add_test(jp_coloring_test)
nwgraph_add_test(kruskal_test)
nwgraph_add_test(mis_test)
nwgraph_add_test(new_dfs_test)
nwgraph_add_test(page_rank_test)
nwgraph_add_test(spanning_tree_test)
nwgraph_add_test(spMatspMat_test)
nwgraph_add_test(tc_test)

# -----------------------------------------------------------------------------
# Adaptor Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(back_edge_test)
nwgraph_add_test(bfs_edge_range_test)
nwgraph_add_test(dag_range_test)
nwgraph_add_test(filtered_bfs_test)
nwgraph_add_test(random_range_test)
nwgraph_add_test(range_adaptors_test)
nwgraph_add_test(reverse_test)
nwgraph_add_test(vertex_range_test)
nwgraph_add_test(worklist_test)

# -----------------------------------------------------------------------------
# I/O Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(mmio_test)

# -----------------------------------------------------------------------------
# Utility Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(atomic_test)
nwgraph_add_test(atomic_bit_vector_test)
nwgraph_add_test(build_test)
nwgraph_add_test(demangle_test)
nwgraph_add_test(disjoint_set_test)
nwgraph_add_test(intersection_size_test)
nwgraph_add_test(permutation_iterator_test)
nwgraph_add_test(priority_queue_test)
nwgraph_add_test(provenance_test)
nwgraph_add_test(proxy_sort_test)
nwgraph_add_test(tag_invoke_test)
nwgraph_add_test(timer_test)
nwgraph_add_test(util_test)

# -----------------------------------------------------------------------------
# Built-in Graph Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(builtin_graphs_test)


# Disabled tests (need investigation)
# nwgraph_add_test(bk_test)
# nwgraph_add_test(dfs_range_test)  # dfs_range.hpp has type errors (vertex_id_type undeclared)
# nwgraph_add_test(kcore_test)
# nwgraph_add_test(max_flow_test)
# nwgraph_add_test(rcm_test)

# Note: Legacy example programs (*_eg.cpp) have been moved to test/eg/
# They are not built by default. Convert to proper tests as needed.

