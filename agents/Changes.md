# NWGraph Cleanup Changes

This document tracks cleanup changes made to the NWGraph codebase.

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

## Planned Changes

### Priority 1: Critical Cleanup

#### Remove Deprecated Headers
- [ ] `include/nwgraph/access.hpp` - Marked deprecated, appears unused
- [ ] `include/nwgraph/compat.hpp` - Marked deprecated, used only by `test/compat_eg.cpp`

#### Fix Buggy Experimental Code
- [ ] `include/nwgraph/experimental/algorithms/jones_plassmann_coloring.hpp`
  - Line 31: `"TODO This implementation is buggy. Need to fix."`
  - Either fix or clearly document limitations

#### Add Include Guards to Graph Headers
- [ ] `include/nwgraph/graphs/karate-graph.hpp` - Missing guards
- [ ] `include/nwgraph/graphs/imdb-graph.hpp` - Missing guards
- [ ] `include/nwgraph/graphs/ospf-graph.hpp` - Missing guards

### Priority 2: Code Quality

#### Remove `#if 0` Disabled Code Blocks
45 instances identified across:
- [ ] `include/nwgraph/build.hpp`
- [ ] `include/nwgraph/containers/zip.hpp`
- [ ] `include/nwgraph/containers/aos.hpp`
- [ ] `include/nwgraph/containers/flattened.hpp`
- [ ] `include/nwgraph/adaptors/bfs_range.hpp`
- [ ] `include/nwgraph/adaptors/bfs_edge_range.hpp`
- [ ] Multiple test and benchmark files

#### Remove Debug Output from Library Headers
- [ ] Remove `std::cout`/`std::cerr` statements from production headers
- [ ] 10 files identified with debug output in library code

### Priority 3: Documentation and Organization

#### Clean Up Old Documentation
- [ ] `doc-src/sphinx/userguide/old/` - Contains deprecated documentation
- [ ] `doc-src/PowerPoint/top_down_nwgraph.pptx` - Binary file in git

#### Clean Up CMakeLists.txt
- [ ] Remove commented-out code and alternative approaches
- [ ] Consolidate TBB finding logic
- [ ] Standardize build option naming

### Priority 4: Clarification

#### Document Experimental vs Production Algorithms
- [ ] Add clear documentation on when to use experimental versions
- [ ] Document status of each experimental algorithm
- [ ] Create migration guide from experimental to production

#### Address TODO Comments
Known TODO locations:
- [ ] `include/nwgraph/experimental/algorithms/connected_components.hpp:202`
- [ ] `include/nwgraph/experimental/algorithms/jones_plassmann_coloring.hpp:81`
- [ ] `include/nwgraph/util/disjoint_set.hpp:370` - "NOT fully tested"

---

## Change Log Format

Each change entry should include:
- **Date**: When the change was made
- **Category**: Type of change (Remove, Fix, Refactor, Document)
- **Files**: List of affected files
- **Description**: What was changed and why
- **Verification**: How the change was verified (tests pass, no references, etc.)

---

## Notes

- All changes should be verified with a clean build
- Run tests after significant changes: `cmake .. -DNWGRAPH_BUILD_TESTS=ON && make && ctest`
- Consider creating a git branch for cleanup work
- Large changes should be reviewed before merging
