# =============================================================================
# NWGraph Benchmarks
# =============================================================================
# This directory contains benchmark implementations organized into subdirectories:
#
#   gapbs/           - NWGraph implementations of GAP Benchmark Suite
#   apb/             - Abstraction Penalty Benchmarks
#   gapbs-reference/ - Original GAP Benchmark Suite (submodule, optional)
# =============================================================================

# Create top-level benchmark target
add_custom_target(bench)

# -----------------------------------------------------------------------------
# Include subdirectories
# -----------------------------------------------------------------------------
add_subdirectory(gapbs)
add_subdirectory(apb)

# -----------------------------------------------------------------------------
# Original GAP Benchmark Suite (optional, for comparison)
# -----------------------------------------------------------------------------
# Enable with: cmake -DNWGRAPH_BUILD_GAPBS_REFERENCE=ON ..
# Requires: git submodule update --init bench/gapbs-reference
# -----------------------------------------------------------------------------
option(NWGRAPH_BUILD_GAPBS_REFERENCE "Build original GAP Benchmark Suite for comparison" OFF)

if (NWGRAPH_BUILD_GAPBS_REFERENCE)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gapbs-reference/Makefile")
    # Build using the original Makefile
    add_custom_target(gapbs-reference
      COMMAND $(MAKE) CXX=${CMAKE_CXX_COMPILER}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gapbs-reference
      COMMENT "Building original GAP Benchmark Suite"
    )
    add_dependencies(bench gapbs-reference)

    # Clean target
    add_custom_target(gapbs-reference-clean
      COMMAND $(MAKE) clean
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gapbs-reference
      COMMENT "Cleaning original GAP Benchmark Suite"
    )
  else()
    message(WARNING "gapbs-reference submodule not initialized. Run: git submodule update --init bench/gapbs-reference")
  endif()
endif()
