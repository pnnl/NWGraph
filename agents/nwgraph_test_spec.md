# NWGraph Test Specification

## Overview

NWGraph uses Catch2 as its primary testing framework. Tests are organized into unit tests (Catch2-based) and example programs demonstrating library usage.

## Test Framework

- **Framework:** Catch2 (C++ Unit Testing Framework)
- **Entry Point:** `test/catch_main.cpp` with `CATCH_CONFIG_MAIN`
- **Header:** `#include <catch2/catch.hpp>`
- **Assertions:** `REQUIRE()`, `REQUIRE(x == Approx(y).epsilon(...))`
- **Test Definition:** `TEST_CASE()` and `SECTION()` macros

## Directory Structure

```
test/
├── CMakeLists.txt              # Test build configuration
├── catch_main.cpp              # Catch2 main entry point
│
├── common/                     # Test utilities
│   ├── abstract_test.hpp       # Graph generation utilities
│   └── test_header.hpp         # Common includes
│
├── data/                       # Test data files (31 files)
│   ├── karate.mtx              # Karate club network
│   ├── football.mtx            # Football network
│   ├── USAir97.mtx             # US Air transportation
│   ├── delaunay_n10.mtx        # Delaunay triangulation
│   └── ...                     # Additional test graphs
│
├── *_test.cpp                  # Catch2 unit tests (23 files)
└── *_eg.cpp                    # Example programs (38 files)
```

## Test Categories

### Unit Tests (Catch2)

#### Data Structure Tests

| Test File | Description |
|-----------|-------------|
| `aos_test.cpp` | Array of Structures container |
| `soa_test.cpp` | Struct of Arrays container |
| `compressed_test.cpp` | Compressed sparse structures |
| `edge_list_test.cpp` | Edge list construction/sorting |
| `size_test.cpp` | Graph size and capacity |
| `vov_test.cpp` | Vector of vectors container |

#### Algorithm Tests

| Test File | Description |
|-----------|-------------|
| `bfs_test_0.cpp` | BFS with edge ranges |
| `bfs_test_1.cpp` | BFS with level-based implementation |
| `connected_component_test.cpp` | Connected components (v1, v2, v5, v6, v8) |
| `tc_test.cpp` | Triangle counting (15 implementations) |
| `page_rank_test.cpp` | PageRank validation |
| `spanning_tree_test.cpp` | Minimum spanning tree |
| `jp_coloring_test.cpp` | Graph coloring |
| `mis_test.cpp` | Maximal independent set |
| `back_edge_test.cpp` | Back edge detection |
| `new_dfs_test.cpp` | Depth-first search |
| `volos_test.cpp` | Volos algorithm tests |
| `spMatspMat_test.cpp` | Sparse matrix multiplication |

#### Concept and Range Tests

| Test File | Description |
|-----------|-------------|
| `ranges_and_concepts_test.cpp` | Range and concept validation |

#### I/O Tests

| Test File | Description |
|-----------|-------------|
| `mmio_test.cpp` | Matrix Market I/O |

### Disabled Tests

Tests available but currently disabled in CMakeLists.txt:

| Test File | Description |
|-----------|-------------|
| `bk_test.cpp` | Betweenness Centrality |
| `kcore_test.cpp` | K-Core decomposition |
| `max_flow_test.cpp` | Maximum flow algorithm |
| `rcm_test.cpp` | Reverse Cuthill-McKee |

### Example Programs

Non-Catch2 executable demonstrations:

| Example | Description |
|---------|-------------|
| `adjacency_eg.cpp` | Adjacency container usage |
| `binio_eg.cpp` | Binary I/O |
| `containers_eg.cpp` | Container operations |
| `concepts_eg.cpp` | Concept demonstrations |
| `dijkstra_eg.cpp` | Dijkstra algorithm |
| `jaccard_eg.cpp` | Jaccard similarity |
| `bfs_eg.cpp` | BFS demonstration |
| `dfs_eg.cpp` | DFS demonstration |
| `tbb_eg.cpp` | TBB integration |
| `zip_eg.cpp` | Zip operations |

## Test Data

### Graph Files (31 total)

#### Real-World Graphs

| File | Size | Description |
|------|------|-------------|
| `karate.mtx` | 1.6 KB | Karate club network (34 vertices) |
| `football.mtx` | 7.5 KB | Football network |
| `USAir97.mtx` | - | US Air transportation |
| `Erdos981.mtx` | 11+ KB | Erdos collaboration |
| `chesapeake.mtx` | 12 KB | Chesapeake Bay food web |
| `spaceStation_1.mtx` | 15 KB | Space station network |

#### Synthetic Graphs

| File | Size | Description |
|------|------|-------------|
| `delaunay_n10.mtx` | 33 KB | Delaunay triangulation |
| `G1.mtx` | 149 KB | General test graph |
| `G8.mtx` | 197 KB | General test graph |
| `dwt___66.mtx` | 1.2 KB | Small test graph |

#### Algorithm-Specific Test Data

| File | Purpose |
|------|---------|
| `coloringData.mmio` | Graph coloring tests |
| `line.mmio`, `ring.mmio`, `tree.mmio` | Simple geometric graphs |
| `bktest1.mtx`, `bktest2.mtx` | Betweenness centrality |
| `flowtest.mtx` | Maximum flow |
| `msttest.mtx` | Minimum spanning tree |
| `spmatA.mmio`, `spmatB.mmio` | Sparse matrix tests |

### Data Path Configuration

Tests use compile-time definitions:
- `DATA_DIR` - Directory containing test data
- `DATA_FILE` - Default test file (typically `karate.mtx`)

## Test Utilities

### common/abstract_test.hpp

```cpp
class test_util {
  directed_csr_graph_t generate_directed_graph();
  directed_csr_graph_t generate_directed_graph(const std::string& mmiofile);
  auto generate_directed_aos();
  auto generate_directed_aos(const std::string& mmiofile);
};
```

### common/test_header.hpp

Standard includes for most tests:
```cpp
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"
#include <catch2/catch.hpp>
```

## CMake Test Infrastructure

### Test Macro

```cmake
nwgraph_add_test(TESTNAME)
# Creates: ${TESTNAME}.exe from ${TESTNAME}.cpp
# Links: nwgraph, Catch2::Catch2, catch_main
# Defines: DATA_DIR, DATA_FILE
# Registers: via add_test()
```

### Example Macro

```cmake
nwgraph_add_exe(EXENAME)
# Creates: ${EXENAME}.exe from ${EXENAME}.cpp
# Links: nwgraph library only
```

## Running Tests

### Build Tests

```bash
mkdir build && cd build
cmake .. -DNWGRAPH_BUILD_TESTS=ON
make -j4
```

### Run All Tests

```bash
ctest
# or with verbose output
ctest -V
ctest --output-on-failure
```

### Run Specific Tests

```bash
cd build/test
./tc_test.exe           # Triangle counting
./page_rank_test.exe    # PageRank
./bfs_test_0.exe        # BFS
```

### Run Tests by Pattern

```bash
ctest -R "bfs"    # All BFS tests
ctest -R "aos"    # AOS container tests
ctest -R "mmio"   # I/O tests
```

### Test Executable Locations

- **Catch2 Tests:** `build/test/*_test.exe`
- **Examples:** `build/test/*_eg.exe`

## Test Patterns

### Validation Pattern (BFS)

```cpp
bool validate(EdgeListT& aos, size_t seed,
              std::vector<size_t> const& distance,
              std::vector<size_t> const& predecessor) {
  // Verify BFS properties:
  // - Distance differences at most 1
  // - Predecessor consistency
  // - Root properties
}
```

### Template Testing Pattern (PageRank)

```cpp
SECTION("adjacency") {
  adjacency<1, RealT> graph(A);
  REQUIRE(page_rank[idx] == Approx(answer[idx]).epsilon(tolerance));
}

SECTION("adj_list") {
  adj_list<0, RealT> graph(A);
  REQUIRE(page_rank[idx] == Approx(answer[idx]).epsilon(tolerance));
}
```

### Multiple Implementation Testing (Triangle Counting)

```cpp
SECTION("v0") {
  size_t triangles = triangle_count(A);
  REQUIRE(triangles == 45);
}
SECTION("v1") {
  size_t triangles = triangle_count_v1(A);
  REQUIRE(triangles == 45);
}
// ... v2, v3, v4, etc.
```

## Test Coverage

| Category | Files | Test Cases |
|----------|-------|------------|
| Unit Tests | 23 | 128+ sections |
| Examples | 38 | - |
| Test Data | 31 | - |
| Disabled | 4 | - |

## Test Organization Strategies

### By Abstraction Level
1. Container tests (AOS, SOA, compressed)
2. Algorithm tests (BFS, CC, PageRank, etc.)
3. I/O tests (Matrix Market)
4. Concept tests (C++20 concepts)

### By Algorithm Category
1. Traversal: BFS, DFS
2. Components: Connected Components
3. Centrality: PageRank, Triangle Counting, BC
4. Coloring: Graph coloring
5. Trees: MST
6. Optimization: MIS, K-Core

### Multiple Implementations
- Triangle counting: 15 versions (v0-v15)
- Connected components: 5 versions (v1, v2, v5, v6, v8)
- PageRank: Multiple container types

## Example Test Structure

```cpp
#include "common/abstract_test.hpp"
#include "nwgraph/algorithms/triangle_count.hpp"

TEST_CASE("triangle counting", "[tc]") {
  // Load test graph
  auto aos_a = read_mm<directedness::undirected>(DATA_DIR "karate.mtx");

  // Prepare graph
  swap_to_triangular<0>(aos_a, succession::successor);
  lexical_sort_by<0>(aos_a);
  adjacency<0> A(num_vertices(aos_a));
  push_back_fill(aos_a, A);

  // Test implementations
  SECTION("v0") {
    size_t triangles = triangle_count(A);
    REQUIRE(triangles == 45);
  }

  SECTION("v1") {
    size_t triangles = triangle_count_v1(A);
    REQUIRE(triangles == 45);
  }
}
```

## Floating-Point Comparison

```cpp
double tolerance(0.005);
REQUIRE(page_rank[idx] == Approx(answer[idx]).epsilon(tolerance));
```
