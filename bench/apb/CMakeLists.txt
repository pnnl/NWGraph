# =============================================================================
# Abstraction Penalty Benchmarks (APB)
# =============================================================================
# These benchmarks measure the performance overhead of various C++ abstractions
# including containers, iteration patterns, and algorithm implementations.
# =============================================================================

# -----------------------------------------------------------------------------
# Plain iteration benchmark
# -----------------------------------------------------------------------------
add_executable(plain.exe plain.cpp)
target_link_libraries(plain.exe nwgraph)
add_dependencies(bench plain.exe)

# -----------------------------------------------------------------------------
# Sparse matrix-vector multiplication
# -----------------------------------------------------------------------------
add_executable(spmv.exe spmv.cpp)
target_link_libraries(spmv.exe nwgraph)
add_dependencies(bench spmv.exe)

# -----------------------------------------------------------------------------
# Execution policy benchmarks
# -----------------------------------------------------------------------------
add_executable(exec.exe exec.cpp)
target_link_libraries(exec.exe nwgraph)
add_dependencies(bench exec.exe)

# -----------------------------------------------------------------------------
# TBB parallel benchmarks
# -----------------------------------------------------------------------------
add_executable(tbb.exe tbb.cpp)
target_link_libraries(tbb.exe nwgraph)
add_dependencies(bench tbb.exe)

# -----------------------------------------------------------------------------
# BFS abstraction penalty
# -----------------------------------------------------------------------------
add_executable(bfs_apb.exe bfs.cpp)
target_link_libraries(bfs_apb.exe nwgraph)
add_dependencies(bench bfs_apb.exe)

# -----------------------------------------------------------------------------
# Dijkstra abstraction penalty
# -----------------------------------------------------------------------------
add_executable(dijkstra.exe dijkstra.cpp)
target_link_libraries(dijkstra.exe nwgraph)
add_dependencies(bench dijkstra.exe)

# -----------------------------------------------------------------------------
# DFS abstraction penalty
# -----------------------------------------------------------------------------
add_executable(dfs.exe dfs.cpp)
target_link_libraries(dfs.exe nwgraph)
add_dependencies(bench dfs.exe)
