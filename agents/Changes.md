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

## Change Log Format

Each change entry should include:
- **Date**: When the change was made
- **Category**: Type of change (Remove, Fix, Refactor, Document)
- **Files**: List of affected files
- **Description**: What was changed and why
- **Verification**: How the change was verified (tests pass, no references, etc.)
