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
- [x] Run clang-format on all benchmark files in `bench/` (gapbs and abstraction_penalty)
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

## Priority 3: Documentation ✅

### Update Sphinx and Documentation Dependencies
- [x] Update `doc-src/sphinx/requirements.txt` with current package versions
  - Updated all packages with version pins using `~=` for patch compatibility
- [x] Update `doc-src/sphinx/conf.py` if needed for new package versions
  - No changes needed - already using modern sphinxcontrib-bibtex API
- [x] Test documentation build with updated packages
  - Build succeeds with 444 warnings (mostly Doxygen @param mismatches)
  - Requires: `python3 -m venv .venv && source .venv/bin/activate && pip install -r requirements.txt`
  - Added `.venv/` to `.gitignore`

**Updated package versions:**

| Package | Version | Notes |
|---------|---------|-------|
| `sphinxcontrib-bibtex` | ~=2.6.0 | Updated from <2.0.0 |
| `breathe` | ~=4.36.0 | Added version pin |
| `exhale` | ~=0.3.7 | Added version pin |
| `sphinx-rtd-theme` | ~=3.0.0 | Added version pin |
| `sphinx-book-theme` | ~=1.1.0 | Added version pin |
| `myst-parser` | ~=4.0.0 | Added version pin |
| `jinja2-highlight` | ~=0.6.0 | Added version pin |
| `sphinx-copybutton` | ~=0.5.0 | Added version pin |

### Clean Up Old Documentation
- [x] Renamed `doc-src/sphinx/userguide/old/` → `archive/` (pedagogical content with figures)
- [x] `doc-src/PowerPoint/` - Directory does not exist (already removed)

### Beautification
- [x] Use a better syntax highlighting scheme that reflects common practice for C++
  - Changed from `emacs` to `friendly` Pygments style

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

## Priority 5.5: Parallel Backend Infrastructure ✅

NWGraph supports three parallel backend configurations (one at a time):
- **TBB** (default) — `std::execution` policies backed by TBB's work-stealing scheduler
- **HPX** — HPX execution policies with distributed parallelism support
- **None** — Sequential fallback, no parallel runtime needed

### Key Files
- `include/nwgraph/util/execution_policy.hpp` — Compatibility layer
  - Three-way `#if`: `NWGRAPH_BACKEND_HPX` / `NWGRAPH_BACKEND_TBB` / neither
  - Provides `nw::graph::execution::{seq, par, par_unseq}` for all backends
  - Algorithm wrappers: `par_sort`, `par_copy`, `par_for_each`, `par_fill`, `par_equal`, `par_unique`, `par_inclusive_scan`, `par_transform`, `par_stable_sort`
- `include/nwgraph/util/backend.hpp` — Backend detection and runtime management
  - `backend::is_hpx()`, `backend::is_tbb()`, `backend::is_parallel()`
  - `backend::init_guard` — RAII guard for HPX lazy initialization
  - `backend::set_num_threads()` — Pre-init thread control (HPX only)
- `doc/HPX_BACKEND.md` — Comprehensive documentation for all three backends

### CMake Integration
- `NWGRAPH_BACKEND_HPX=ON` — Use HPX backend
- `NWGRAPH_BACKEND_NONE=ON` — Sequential only (no TBB or HPX)
- Neither set — TBB backend (default)
- Support for `TBB_ROOT`, `TBBROOT`, `HPX_ROOT` via CMake variables or environment

### Compiler Requirements
- **macOS**: Must use g++-13 or later (`brew install gcc@13`)
  - Apple Clang lacks `<execution>` header / `std::execution` support
  - Always pass `-DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-13`
  - Both HPX and NWGraph must be built with the same compiler
- **Linux**: g++-11 or later recommended
- HPX lives at `~/usr/local/hpx` (user-local) — set via `-DHPX_ROOT=$HOME/usr/local/hpx`

### Backend-Specific Features (not in compatibility layer)
Each backend has features with no cross-backend equivalent. Code using these
must be guarded with `#if NWGRAPH_BACKEND_TBB_ENABLED` or `#if NWGRAPH_BACKEND_HPX_ENABLED`.

**TBB-specific (no std/HPX equivalent):**
- `tbb::global_control` — scoped thread count control
- `tbb::parallel_reduce` with `tbb::blocked_range` — parallel reduction with custom join
- `tbb::parallel_for` with `tbb::blocked_range` — chunked parallel loops with grain size control
- `tbb::task_group`, `tbb::flow::graph`, `tbb::scalable_allocator`

**HPX-specific (no std/TBB equivalent):**
- `--hpx:threads=N` / `backend::set_num_threads()` — thread count (immutable after runtime start)
- `hpx::for_loop`, `hpx::transform_reduce` — index-based parallel loops
- `hpx::async()`, `hpx::future<T>` — async task execution
- Distributed parallelism (remote actions, `hpx::find_here()`)
- `--hpx:bind=scatter|compact|balanced` — NUMA-aware thread binding

### Known Shortcomings
- **HPX + custom iterators**: HPX algorithms don't work with NWGraph's `soa_iterator`; e.g., `tc.cpp` cannot run with HPX
- **HPX thread count immutable**: `set_num_threads()` must be called before first parallel op
- **TBB `parallel_reduce` not portable**: No std or HPX equivalent for `blocked_range` + join pattern
- **TBB `global_control` not portable**: Thread limiting is TBB-specific
- **Sequential backend**: `par` / `par_unseq` are syntactically accepted but execute sequentially

### Headers Updated to Use Compatibility Layer
- [x] `include/nwgraph/containers/soa.hpp`
- [x] `include/nwgraph/containers/aos.hpp`
- [x] `include/nwgraph/containers/compressed.hpp`
- [x] `include/nwgraph/containers/zip.hpp`
- [x] `include/nwgraph/util/proxysort.hpp`
- [x] `include/nwgraph/util/disjoint_set.hpp`
- [x] `include/nwgraph/util/AtomicBitVector.hpp`
- [x] `include/nwgraph/util/intersection_size.hpp`
- [x] `include/nwgraph/algorithms/bfs.hpp`
- [x] `include/nwgraph/build.hpp`
- [x] `include/nwgraph/adjacency.hpp`

### Unit Tests
- [x] `test/parallel_backend_test.cpp`
  - Tests execution policy types and instances
  - Tests wrapper functions (par_sort, par_copy, par_for_each, etc.)
  - TBB-specific tests (parallel_reduce) guarded with `#if !defined(NWGRAPH_BACKEND_HPX)`
  - Backend detection tests

---

## Priority 6: Testing ✅

### Modernize Test Infrastructure
- [x] Upgrade to latest header-only Catch2 (v3.x) - Updated to v3.5.2
- [x] Add Catch2 as a proper CMake dependency (FetchContent)
- [x] Create comprehensive unit tests corresponding to each header file
- [x] Organize tests to mirror the `include/nwgraph/` directory structure
- [x] Verify all tests pass with clean build - 171 tests passing

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

## Priority 8: Benchmarks ✅

### Reorganize Benchmark Structure
- [x] Move all benchmarks under `bench/` with subdirectories:
  ```
  bench/
  ├── gapbs/               # NWGraph GAP Benchmark Suite implementations
  ├── abstraction_penalty/ # Abstraction Penalty Benchmarks
  └── gapbs-reference/     # Original GAP Benchmark Suite (submodule)
  ```
- [x] Update `CMakeLists.txt` to reflect new directory structure
- [x] Add original GAP Benchmark Suite as submodule for comparison

### GAP Benchmark Suite (`bench/gapbs/`)
- [x] Move current `bench/*.cpp` files (bfs, cc, pr, sssp, tc, bc) to `bench/gapbs/`
- [x] Reference paper: [Beamer et al., IISWC 2020](https://scottbeamer.net/pubs/beamer-iiswc2020.pdf)
- [x] Standardize all benchmark files to follow the pattern in `bfs.cpp`:
  - Use `Log.hpp` for structured output
  - Use `common.hpp` for shared utilities
  - Remove any `nlohmann/json` (lohmann) dependencies
- [x] Remove lohmann JSON library usage from entire NWGraph codebase
  - `tc.cpp` converted to use `Times` class and `Log.hpp`
  - `js.cpp` disabled (commented out, needs conversion if re-enabled)

### Abstraction Penalty Benchmarks (`bench/abstraction_penalty/`)
- [x] Move current `apb/` contents to `bench/abstraction_penalty/`
- [x] Connect APB executables to `bench` target
- [x] Create shared infrastructure (`apb_common.hpp`):
  - `bench()` function template to eliminate timing boilerplate
  - `Args` struct for consistent CLI parsing
  - `load_graph()` function for graph loading
- [x] Refactor all APB benchmarks to use shared infrastructure:
  - `bfs.cpp` - BFS traversal benchmarks (261 lines, was 419)
  - `dfs.cpp` - DFS traversal benchmarks (170 lines, was 277)
  - `spmv.cpp` - SpMV benchmarks (180 lines, was 444)
  - `plain.cpp` - Plain range benchmarks (296 lines, was 596)
  - `dijkstra.cpp` - Dijkstra property access (188 lines, was 384)
  - `exec.cpp` - Execution policy benchmarks (121 lines, was 222)
  - `tbb.cpp` - TBB parallelization benchmarks (176 lines, was 359)
  - `containers.cpp` - Container comparison benchmarks (235 lines)
- [x] Remove dead code (`#if 0` blocks, unused variables)
- [x] Add descriptive file headers documenting what each benchmark measures
- [ ] Ensure consistent logging and output format (future)
- [ ] Add structured output matching gapbs pattern (future)

### Build Options
- `NWGRAPH_BUILD_BENCH=ON` - Build NWGraph benchmarks (gapbs + apb)
- `NWGRAPH_BUILD_GAPBS_REFERENCE=ON` - Also build original GAP suite for comparison

---

## Notes

- All changes should be verified with a clean build
- Run tests after significant changes:
  ```bash
  cmake .. -DNWGRAPH_BUILD_TESTS=ON && make && ctest
  ```
- Consider creating feature branches for major changes
- Large changes should be reviewed before merging to master
