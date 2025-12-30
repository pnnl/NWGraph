# NWGraph Cleanup Changes

This document tracks cleanup changes made to the NWGraph codebase.

For planned/pending changes, see [nwgraph_instructions.md](nwgraph_instructions.md).

---

## Completed Changes

### 2024-12-30: Remove Scrap/Dead Code Files

**Files Deleted:**

| File | Description | Reason |
|------|-------------|--------|
| `include/nwgraph/scrap.cpp` | 112 lines of disabled code (`#if 0` blocks) with old iterator implementations and orphan template functions | Dead code, not part of build, not referenced anywhere |
| `include/nwgraph/scrap.txt` | 42 lines of old CMake code for Parallelism TS feature detection | Outdated, not used by current build system |
| `include/nwgraph/containers/scrap.cpp` | 86 lines with unused `const_outer_iterator` class | Dead code, not integrated into build system |

**Verification:**
- Confirmed no references to these files in CMakeLists.txt
- Confirmed no includes or dependencies from other source files
- Files were not part of the nwgraph library target

---

### 2024-12-30: Code Formatting and Cleanup

**Applied clang-format to entire codebase:**
- 88 header files in `include/nwgraph/`
- All test files in `test/`
- All benchmark files in `bench/` and `apb/`
- All example files in `examples/`

**Formatting standards applied:**
- 2-space indentation
- 144 character line limit
- Attached braces (Allman style)
- No namespace indentation
- Aligned consecutive assignments and declarations
- Sorted includes

**Added include guards to graph data headers:**

| File | Guard Added |
|------|-------------|
| `include/nwgraph/graphs/karate-graph.hpp` | `NW_GRAPH_KARATE_GRAPH_HPP` |
| `include/nwgraph/graphs/imdb-graph.hpp` | `NW_GRAPH_IMDB_GRAPH_HPP` |
| `include/nwgraph/graphs/ospf-graph.hpp` | `NW_GRAPH_OSPF_GRAPH_HPP` |

**Removed deprecated headers:**

| File | Reason |
|------|--------|
| `include/nwgraph/access.hpp` | Deprecated, empty (only contained `#warning`), no users |
| `include/nwgraph/compat.hpp` | Deprecated, all code inside `#if 0`, no functional content |
| `test/compat_eg.cpp` | Test file for removed `compat.hpp` |

---

### 2024-12-30: Remove #if 0 Disabled Code Blocks

**Removed dead code from headers:**

| File | Lines Removed | Description |
|------|---------------|-------------|
| `include/nwgraph/adjacency.hpp` | ~20 | Old `index_compressed` class |
| `include/nwgraph/build.hpp` | ~15 | Kept active `#else` branch code, removed disabled block |
| `include/nwgraph/containers/zip.hpp` | ~30 | `push_back`, `push_at`, `iter_swap`, `swap` functions |
| `include/nwgraph/containers/aos.hpp` | ~10 | Unused `clear`, `resize`, `reserve`, `begin`, `end`, `erase`, `size` methods |
| `include/nwgraph/vofos.hpp` | ~8 | Unused `num_vertices` template function |
| `include/nwgraph/volos.hpp` | ~8 | Unused `num_vertices` template function |
| `include/nwgraph/util/proxysort.hpp` | ~7 | TBB parallel_for alternative (kept std::iota) |
| `include/nwgraph/adaptors/bfs_range.hpp` | ~80 | Old `topdown_bfs_range` with color-based visited tracking |
| `include/nwgraph/adaptors/bfs_edge_range.hpp` | ~85 | Old `bfs_edge_range` with color-based visited tracking |
| `include/nwgraph/io/mmio.hpp` | ~200 | `par_read_mm` and `par_load_mm` functions |
| `include/nwgraph/experimental/algorithms/jaccard.hpp` | ~15 | `jaccard_similarity_v2` function |
| `include/nwgraph/experimental/algorithms/betweenness_centrality.hpp` | ~100 | `PBFS` and `Brandes` functions |
| `include/nwgraph/experimental/algorithms/page_rank.hpp` | ~70 | `page_rank_range_for` and incomplete `tbb::parallel_reduce` block |

**Deleted dead code file:**

| File | Reason |
|------|--------|
| `include/nwgraph/containers/flattened.hpp` | Entire file was inside `#if 0` block, not included anywhere |

---

### 2024-12-30: Remove Debug Output from Experimental Algorithms

**Removed unconditional std::cout statements:**

| File | Description |
|------|-------------|
| `include/nwgraph/experimental/algorithms/betweenness_centrality.hpp` | Removed debug output in `approx_betweenness_worklist` ("back", u/v edge printing), `bc2_v2` ("depth phase"), `bc2_v3` ("source:") |

**Note:** Many debug statements remain in experimental code that should be wrapped in `#ifdef PRINT_DEBUG` guards or removed entirely. The `jones_plassmann_coloring.hpp` file already uses proper `#ifdef PRINT_DEBUG` guards as a pattern to follow.

---

## Change Log Format

Each change entry should include:
- **Date**: When the change was made
- **Category**: Type of change (Remove, Fix, Refactor, Document)
- **Files**: List of affected files
- **Description**: What was changed and why
- **Verification**: How the change was verified (tests pass, no references, etc.)
