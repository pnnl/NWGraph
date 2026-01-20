# =============================================================================
# Scalability Benchmarks
# =============================================================================
# These benchmarks measure the parallel scaling characteristics of NWGraph's
# parallel primitives. They can be built with either TBB or HPX backend.
# =============================================================================

# -----------------------------------------------------------------------------
# parallel_for scalability benchmark
# -----------------------------------------------------------------------------
add_executable(parallel_for_scaling parallel_for_scaling.cpp)
target_link_libraries(parallel_for_scaling nwgraph)
add_dependencies(bench parallel_for_scaling)

# -----------------------------------------------------------------------------
# parallel_reduce scalability benchmark
# -----------------------------------------------------------------------------
add_executable(parallel_reduce_scaling parallel_reduce_scaling.cpp)
target_link_libraries(parallel_reduce_scaling nwgraph)
add_dependencies(bench parallel_reduce_scaling)

# -----------------------------------------------------------------------------
# Create convenience target for all scalability benchmarks
# -----------------------------------------------------------------------------
add_custom_target(scalability_benchmarks
  DEPENDS parallel_for_scaling parallel_reduce_scaling
  COMMENT "Building all scalability benchmarks"
)
