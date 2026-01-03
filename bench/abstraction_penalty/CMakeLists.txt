# =============================================================================
# Abstraction Penalty Benchmarks (APB)
# =============================================================================
# These benchmarks measure the performance overhead of various C++ abstractions
# including containers, iteration patterns, and algorithm implementations.
# =============================================================================

# -----------------------------------------------------------------------------
# Plain iteration benchmark
# -----------------------------------------------------------------------------
add_executable(plain plain.cpp)
target_link_libraries(plain nwgraph)
add_dependencies(bench plain)

# -----------------------------------------------------------------------------
# Sparse matrix-vector multiplication
# -----------------------------------------------------------------------------
add_executable(spmv spmv.cpp)
target_link_libraries(spmv nwgraph)
add_dependencies(bench spmv)

# -----------------------------------------------------------------------------
# Execution policy benchmarks
# -----------------------------------------------------------------------------
add_executable(exec exec.cpp)
target_link_libraries(exec nwgraph)
add_dependencies(bench exec)

# -----------------------------------------------------------------------------
# TBB parallel benchmarks
# -----------------------------------------------------------------------------
add_executable(tbb tbb.cpp)
target_link_libraries(tbb nwgraph)
add_dependencies(bench tbb)

# -----------------------------------------------------------------------------
# BFS abstraction penalty
# -----------------------------------------------------------------------------
add_executable(bfs_apb bfs.cpp)
target_link_libraries(bfs_apb nwgraph)
add_dependencies(bench bfs_apb)

# -----------------------------------------------------------------------------
# Dijkstra abstraction penalty
# -----------------------------------------------------------------------------
add_executable(dijkstra dijkstra.cpp)
target_link_libraries(dijkstra nwgraph)
add_dependencies(bench dijkstra)

# -----------------------------------------------------------------------------
# DFS abstraction penalty
# -----------------------------------------------------------------------------
add_executable(dfs dfs.cpp)
target_link_libraries(dfs nwgraph)
add_dependencies(bench dfs)

# -----------------------------------------------------------------------------
# Container abstraction penalty (requires docopt)
# -----------------------------------------------------------------------------
add_executable(containers containers.cpp)
target_link_libraries(containers nwgraph docopt)
add_dependencies(bench containers)
