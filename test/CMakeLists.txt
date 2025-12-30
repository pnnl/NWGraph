

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
nwgraph_add_test(aos_test)
nwgraph_add_test(compressed_test)
nwgraph_add_test(edge_list_test)
nwgraph_add_test(soa_test)
nwgraph_add_test(volos_test)
nwgraph_add_test(vov_test)

# -----------------------------------------------------------------------------
# Graph Concept and Traits Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(graph_concepts_test)
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
nwgraph_add_test(vertex_range_test)

# -----------------------------------------------------------------------------
# I/O Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(mmio_test)

# -----------------------------------------------------------------------------
# Utility Tests
# -----------------------------------------------------------------------------
nwgraph_add_test(disjoint_set_test)
nwgraph_add_test(timer_test)


# nwgraph_add_test(bk_test)
# nwgraph_add_test(kcore_test)
# nwgraph_add_test(max_flow_test)
# nwgraph_add_test(rcm_test)


# expected failures for now
# set_tests_properties(jp_coloring_test
#  PROPERTIES WILL_FAIL TRUE)

# Example programs (not catch2)
nwgraph_add_exe(adjacency_eg)
nwgraph_add_exe(concepts_eg)
nwgraph_add_exe(containers_eg)
nwgraph_add_exe(cstar_eg)
nwgraph_add_exe(degree_eg)
nwgraph_add_exe(dijkstra_eg)
nwgraph_add_exe(edge_list_eg)
nwgraph_add_exe(edge_range_eg)
# nwgraph_add_exe(graph_adaptor_eg)
nwgraph_add_exe(graph_traits_eg)
nwgraph_add_exe(index_soa_eg)
nwgraph_add_exe(indexing_eg)
nwgraph_add_exe(init_eg)
nwgraph_add_exe(jaccard_eg)
nwgraph_add_exe(neighbor_range_eg)
nwgraph_add_exe(plain_range_eg)
nwgraph_add_exe(proxysort_eg)
nwgraph_add_exe(random_eg)
nwgraph_add_exe(spmm_eg)
nwgraph_add_exe(stream_eg)
nwgraph_add_exe(std_containers_eg)
nwgraph_add_exe(tag_invoke_eg)
nwgraph_add_exe(tbb_eg)
nwgraph_add_exe(triangular_eg)
# nwgraph_add_exe(vector_bool_eg)
nwgraph_add_exe(vertex_range_eg)
nwgraph_add_exe(volos_eg)
nwgraph_add_exe(zip_eg)

# nwgraph_add_exe(binio_eg)
# nwgraph_add_exe(io_eg)
# nwgraph_add_exe(math_eg)
# nwgraph_add_exe(ranges_and_concepts_eg)
# nwgraph_add_exe(symmetrize_eg)


