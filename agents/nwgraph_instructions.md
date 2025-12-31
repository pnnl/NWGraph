# NWGraph Cleanup Instructions

This document tracks cleanup tasks for the NWGraph codebase, including completed work and planned changes.

---

## Priority 1: Critical Cleanup

### Remove Dead Code Files
- [x] `include/nwgraph/scrap.cpp` - 112 lines of disabled code
- [x] `include/nwgraph/scrap.txt` - Old CMake code for Parallelism TS
- [x] `include/nwgraph/containers/scrap.cpp` - Unused `const_outer_iterator` class

### Remove Deprecated Headers
- [x] `include/nwgraph/access.hpp` - Deprecated, empty (only `#warning`)
- [x] `include/nwgraph/compat.hpp` - Deprecated, all code in `#if 0`
- [x] `test/compat_eg.cpp` - Test file for removed `compat.hpp`

### Add Include Guards to Graph Headers
- [x] `include/nwgraph/graphs/karate-graph.hpp` - Added `NW_GRAPH_KARATE_GRAPH_HPP`
- [x] `include/nwgraph/graphs/imdb-graph.hpp` - Added `NW_GRAPH_IMDB_GRAPH_HPP`
- [x] `include/nwgraph/graphs/ospf-graph.hpp` - Added `NW_GRAPH_OSPF_GRAPH_HPP`

### Fix Buggy Experimental Code
- [ ] `include/nwgraph/experimental/algorithms/jones_plassmann_coloring.hpp`
  - Line 31: `"TODO This implementation is buggy. Need to fix."`
  - Either fix or clearly document limitations

---

## Priority 2: Code Quality

### Apply Consistent Code Formatting
- [x] Run clang-format on all 88 headers in `include/nwgraph/`
- [x] Run clang-format on all test files in `test/`
- [x] Run clang-format on all benchmark files in `bench/` and `apb/`
- [x] Run clang-format on all example files in `examples/`

**Formatting standards (defined in `.clang-format`):**
- 2-space indentation
- 144 character line limit
- Attached braces (Allman style)
- No namespace indentation
- Aligned consecutive assignments and declarations
- Sorted includes

### Remove `#if 0` Disabled Code Blocks
All `#if 0` blocks removed from library headers:
- [x] `include/nwgraph/adjacency.hpp` - Old `index_compressed` class
- [x] `include/nwgraph/build.hpp` - Kept active `#else` branch
- [x] `include/nwgraph/containers/zip.hpp` - Unused member functions
- [x] `include/nwgraph/containers/aos.hpp` - Unused member functions
- [x] `include/nwgraph/containers/flattened.hpp` - **DELETED** (entirely dead code)
- [x] `include/nwgraph/vofos.hpp` - Unused `num_vertices` function
- [x] `include/nwgraph/volos.hpp` - Unused `num_vertices` function
- [x] `include/nwgraph/util/proxysort.hpp` - TBB alternative (kept std::iota)
- [x] `include/nwgraph/adaptors/bfs_range.hpp` - Old color-based implementation
- [x] `include/nwgraph/adaptors/bfs_edge_range.hpp` - Old color-based implementation
- [x] `include/nwgraph/io/mmio.hpp` - `par_read_mm`/`par_load_mm` functions
- [x] `include/nwgraph/experimental/algorithms/jaccard.hpp` - `jaccard_similarity_v2`
- [x] `include/nwgraph/experimental/algorithms/betweenness_centrality.hpp` - `PBFS`/`Brandes`
- [x] `include/nwgraph/experimental/algorithms/page_rank.hpp` - `page_rank_range_for`

### Remove Debug Output from Library Headers
- [x] Removed unconditional debug output from `experimental/algorithms/betweenness_centrality.hpp`
- [ ] Wrap remaining debug output in `#ifdef PRINT_DEBUG` guards (follow pattern in `jones_plassmann_coloring.hpp`)
- [ ] Review `std::cerr` statements in `io/mmio.hpp` (some are appropriate error messages)
- [ ] Review `stream_stats()` and `stream_indices()` functions (intentional output APIs)

---

## Priority 3: Documentation

### Update Sphinx and Documentation Dependencies
- [ ] Update `doc-src/sphinx/requirements.txt` with current package versions
  - `sphinxcontrib-bibtex<2.0.0` is outdated (current ~2.6)
  - Add version pins to all packages for reproducibility
- [ ] Update `doc-src/sphinx/conf.py` if needed for new package versions
- [ ] Test documentation build with updated packages

**Packages needing version updates:**

| Package | Current | Issue |
|---------|---------|-------|
| `sphinxcontrib-bibtex` | <2.0.0 | Very outdated (current ~2.6) |
| `breathe` | unpinned | No version pin |
| `exhale` | unpinned | No version pin |
| `sphinx-rtd-theme` | unpinned | No version pin |
| `sphinx-book-theme` | unpinned | No version pin |
| `myst_parser` | unpinned | No version pin |
| `jinja2-highlight` | unpinned | No version pin |
| `sphinx_copybutton` | unpinned | No version pin |

### Clean Up Old Documentation
- [ ] Remove or archive `doc-src/sphinx/userguide/old/` (deprecated documentation)
- [ ] Remove `doc-src/PowerPoint/top_down_nwgraph.pptx` (binary file in git)

---

## Priority 4: Clarification

### Document Experimental vs Production Algorithms
- [ ] Add clear documentation on when to use experimental versions
- [ ] Document status of each experimental algorithm
- [ ] Create migration guide from experimental to production

### Address TODO Comments
Known TODO locations:
- [ ] `include/nwgraph/experimental/algorithms/connected_components.hpp:202`
- [ ] `include/nwgraph/experimental/algorithms/jones_plassmann_coloring.hpp:81`
- [ ] `include/nwgraph/util/disjoint_set.hpp:370` - "NOT fully tested"

---

## Priority 5: CMake

### Modernize CMake Configuration
- [ ] Clean up main `CMakeLists.txt`
  - Remove commented-out code and alternative approaches
  - Consolidate TBB finding logic
  - Standardize build option naming (e.g., `NWGRAPH_BUILD_*` prefix)
- [ ] Update all `CMakeLists.txt` files for consistency
- [ ] Ensure proper target dependencies and modern CMake practices

---

## Priority 6: Testing

### Modernize Test Infrastructure
- [ ] Upgrade to latest header-only Catch2 (v3.x)
- [ ] Add Catch2 as a proper CMake dependency (FetchContent or find_package)
- [ ] Create comprehensive unit tests corresponding to each header file
- [ ] Organize tests to mirror the `include/nwgraph/` directory structure
- [ ] Verify all tests pass with clean build

---

## Priority 7: Examples

### Organize and Expand Examples
- [x] **BGL Book Examples** (`examples/bgl-book/`) - Complete
  - [x] ch3_toposort.cpp - Topological sort (File Dependencies)
  - [x] ch4_kevin_bacon.cpp - BFS (Six Degrees of Kevin Bacon)
  - [x] ch4_loop_detection.cpp - DFS cycle detection
  - [x] ch5_dijkstra.cpp - Dijkstra's algorithm (OSPF routing)
  - [x] ch5_bellman_ford.cpp - Bellman-Ford algorithm (distance vector routing)
  - [x] ch6_kruskal.cpp - Kruskal's MST algorithm
  - [x] ch6_prim.cpp - Prim's MST algorithm
  - [x] ch7_connected.cpp - Connected components (BFS-based)
  - [x] ch7_strongly_connected.cpp - Strongly connected components (Tarjan's)
  - [x] ch8_maxflow.cpp - Maximum flow (Edmonds-Karp)
  - [x] ch9_knights_tour.cpp - Knight's tour (implicit graph, Warnsdorff's heuristic)
- [ ] **IMDB Examples** (`examples/imdb/`)
  - Clean up existing IMDB examples
  - Ensure they build and run correctly
- [ ] **Algorithm Examples**
  - Add examples demonstrating important NWGraph use cases
  - Ensure consistency between examples and Doxygen/Sphinx documentation
- [ ] **Separation of Concerns**
  - Examples should demonstrate usage patterns (not test correctness)
  - Unit tests should verify correctness (not demonstrate usage)
- [x] Verify BGL book examples compile and run correctly (11 of 11 implemented)

---

## Priority 8: Benchmarks

### Reorganize Benchmark Structure
- [ ] Move all benchmarks under `bench/` with subdirectories:
  ```
  bench/
  ├── gapbs/          # GAP Benchmark Suite implementations
  └── apb/            # Abstraction Penalty Benchmarks
  ```
- [ ] Update `CMakeLists.txt` to reflect new directory structure

### GAP Benchmark Suite (`bench/gapbs/`)
- [ ] Move current `bench/*.cpp` files (bfs, cc, pr, sssp, tc, bc) to `bench/gapbs/`
- [ ] Reference paper: [Beamer et al., IISWC 2020](https://scottbeamer.net/pubs/beamer-iiswc2020.pdf)
- [ ] Standardize all benchmark files to follow the pattern in `bfs.cpp`:
  - Use `Log.hpp` for structured output
  - Use `common.hpp` for shared utilities
  - Remove any `nlohmann/json` (lohmann) dependencies
- [ ] Remove lohmann JSON library usage from entire NWGraph codebase

### Abstraction Penalty Benchmarks (`bench/apb/`)
- [ ] Move current `apb/` contents to `bench/apb/`
- [ ] Bring APB benchmarks to same quality level as GAP benchmarks
- [ ] Ensure consistent logging and output format
- [ ] Document what each APB measures (iteration overhead, container overhead, etc.)

---

## Notes

- All changes should be verified with a clean build
- Run tests after significant changes:
  ```bash
  cmake .. -DNWGRAPH_BUILD_TESTS=ON && make && ctest
  ```
- Consider creating feature branches for major changes
- Large changes should be reviewed before merging to master
