# NWGraph Test Specification

## Overview

NWGraph uses Catch2 v3 as its testing framework. All tests are organized by category and linked against `Catch2::Catch2WithMain`.

## Test Framework

- **Framework:** Catch2 v3.5.2 (fetched via CMake FetchContent)
- **Header:** `#include <catch2/catch_test_macros.hpp>`
- **Assertions:** `REQUIRE()`, `CHECK()`, `REQUIRE(x == Approx(y))`
- **Test Definition:** `TEST_CASE("name", "[tag]")` and `SECTION("name")`

## Directory Structure

```
test/
├── CMakeLists.txt              # Test build configuration
├── common/                     # Test utilities
│   ├── test_header.hpp         # Common includes for all tests
│   └── karate.hpp              # Karate graph data for tests
└── *_test.cpp                  # Catch2 unit tests
```

## Complete Header Coverage Matrix

This section maps every header file to its corresponding test(s).

### Adaptors (`include/nwgraph/adaptors/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `back_edge_range.hpp` | `back_edge_test.cpp` | ✅ Active |
| `bfs_edge_range.hpp` | `bfs_edge_range_test.cpp` | ✅ Active |
| `bfs_range.hpp` | `bfs_test_0.cpp`, `bfs_test_1.cpp` | ✅ Active |
| `cyclic_neighbor_range.hpp` | - | ⚠️ TBB-specific, not tested |
| `cyclic_range_adaptor.hpp` | - | ⚠️ TBB-specific, not tested |
| `dag_range.hpp` | `dag_range_test.cpp` | ✅ Active |
| `dfs_range.hpp` | `dfs_range_test.cpp` | ❌ Disabled (header has bugs) |
| `edge_range.hpp` | `range_adaptors_test.cpp` | ✅ Active |
| `filtered_bfs_range.hpp` | `filtered_bfs_test.cpp` | ✅ Active |
| `neighbor_range.hpp` | `range_adaptors_test.cpp` | ✅ Active |
| `new_dfs_range.hpp` | `new_dfs_test.cpp` | ✅ Active |
| `plain_range.hpp` | `range_adaptors_test.cpp` | ✅ Active |
| `random_range.hpp` | `random_range_test.cpp` | ✅ Active |
| `reverse.hpp` | `reverse_test.cpp` | ✅ Active |
| `splittable_range_adaptor.hpp` | - | ⚠️ TBB-specific, not tested |
| `vertex_range.hpp` | `vertex_range_test.cpp` | ✅ Active |
| `worklist.hpp` | `worklist_test.cpp` | ✅ Active |

### Core Graph Types (`include/nwgraph/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `adjacency.hpp` | `adjacency_test.cpp` | ✅ Active |
| `build.hpp` | `build_test.cpp` | ✅ Active |
| `coo.hpp` | - | ✅ Alias for edge_list (covered) |
| `csc.hpp` | - | ✅ Alias for adjacency<1> (covered) |
| `csr.hpp` | - | ✅ Alias for adjacency<0> (covered) |
| `edge_list.hpp` | `edge_list_test.cpp` | ✅ Active |
| `graph_adaptor.hpp` | `graph_adaptor_test.cpp` | ✅ Active |
| `graph_base.hpp` | `graph_base_test.cpp` | ✅ Active |
| `graph_concepts.hpp` | `graph_concepts_test.cpp` | ✅ Active |
| `graph_traits.hpp` | `graph_traits_test.cpp` | ✅ Active |
| `vofos.hpp` | `vofos_test.cpp` | ✅ Active |
| `volos.hpp` | `volos_test.cpp` | ✅ Active |
| `vovos.hpp` | `vov_test.cpp` | ✅ Active |

### Algorithms (`include/nwgraph/algorithms/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `betweenness_centrality.hpp` | `betweenness_centrality_test.cpp` | ✅ Active |
| `bfs.hpp` | `bfs_test_0.cpp`, `bfs_test_1.cpp` | ✅ Active |
| `boykov_kolmogorov.hpp` | `max_flow_test.cpp` | ❌ Disabled |
| `connected_components.hpp` | `connected_component_test.cpp` | ✅ Active |
| `dag_based_mis.hpp` | `mis_test.cpp` | ✅ Active |
| `delta_stepping.hpp` | `delta_stepping_test.cpp` | ✅ Active |
| `dijkstra.hpp` | `dijkstra_test.cpp` | ✅ Active |
| `jaccard.hpp` | `jaccard_test.cpp` | ✅ Active |
| `jones_plassmann_coloring.hpp` | `jp_coloring_test.cpp` | ✅ Active |
| `k_core.hpp` | `kcore_test.cpp` | ❌ Disabled |
| `k_truss.hpp` | - | ⚠️ Empty/incomplete header |
| `kruskal.hpp` | `kruskal_test.cpp` | ✅ Active |
| `max_flow.hpp` | `max_flow_test.cpp` | ❌ Disabled |
| `maximal_independent_set.hpp` | `mis_test.cpp` | ✅ Active |
| `page_rank.hpp` | `page_rank_test.cpp` | ✅ Active |
| `prim.hpp` | `prim_test.cpp` | ✅ Active |
| `spMatspMat.hpp` | `spMatspMat_test.cpp` | ✅ Active |
| `triangle_count.hpp` | `tc_test.cpp` | ✅ Active |

### Containers (`include/nwgraph/containers/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `aos.hpp` | `aos_test.cpp` | ✅ Active |
| `compressed.hpp` | `compressed_test.cpp` | ✅ Active |
| `soa.hpp` | `soa_test.cpp` | ✅ Active |
| `zip.hpp` | `zip_test.cpp` | ✅ Active |

### Generators (`include/nwgraph/generators/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `configuration_model.hpp` | - | ❌ Header has syntax errors |

### Built-in Graphs (`include/nwgraph/graphs/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `imdb-graph.hpp` | `builtin_graphs_test.cpp` | ✅ Active |
| `karate-graph.hpp` | `builtin_graphs_test.cpp` | ✅ Active |
| `ospf-graph.hpp` | `builtin_graphs_test.cpp` | ✅ Active |

### I/O (`include/nwgraph/io/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `MatrixMarketFile.hpp` | `mmio_test.cpp` | ✅ Active |
| `mmio.hpp` | `mmio_test.cpp` | ✅ Active |

### Utilities (`include/nwgraph/util/`)

| Header | Test File | Status |
|--------|-----------|--------|
| `arrow_proxy.hpp` | `util_test.cpp` | ✅ Active |
| `atomic.hpp` | `atomic_test.cpp` | ✅ Active |
| `AtomicBitVector.hpp` | `atomic_bit_vector_test.cpp` | ✅ Active |
| `defaults.hpp` | - | ✅ Type definitions only |
| `demangle.hpp` | `demangle_test.cpp` | ✅ Active |
| `disjoint_set.hpp` | `disjoint_set_test.cpp` | ✅ Active |
| `intersection_size.hpp` | `intersection_size_test.cpp` | ✅ Active |
| `make_priority_queue.hpp` | `priority_queue_test.cpp` | ✅ Active |
| `parallel_for.hpp` | - | ⚠️ TBB wrapper, not unit tested |
| `permutation_iterator.hpp` | `permutation_iterator_test.cpp` | ✅ Active |
| `print_types.hpp` | - | ✅ Debug utility only |
| `provenance.hpp` | `provenance_test.cpp` | ✅ Active |
| `proxysort.hpp` | `proxy_sort_test.cpp` | ✅ Active |
| `tag_invoke.hpp` | `tag_invoke_test.cpp` | ✅ Active |
| `timer.hpp` | `timer_test.cpp` | ✅ Active |
| `traits.hpp` | `graph_traits_test.cpp` | ✅ Active |
| `tuple_hack.hpp` | `util_test.cpp` | ✅ Active |
| `util_par.hpp` | - | ⚠️ TBB utilities, not unit tested |
| `util.hpp` | `util_test.cpp` | ✅ Active |

### Experimental (`include/nwgraph/experimental/`)

Experimental parallel implementations - not unit tested as they mirror main algorithms.

## Test Categories in CMakeLists.txt

### Container Tests
- `adjacency_test`
- `aos_test`
- `compressed_test`
- `edge_list_test`
- `soa_test`
- `vofos_test`
- `volos_test`
- `vov_test`
- `zip_test`

### Graph Concept and Traits Tests
- `graph_adaptor_test`
- `graph_base_test`
- `graph_concepts_test`
- `graph_traits_test`
- `size_test`

### Algorithm Tests
- `betweenness_centrality_test`
- `bfs_test_0`
- `bfs_test_1`
- `connected_component_test`
- `delta_stepping_test`
- `dijkstra_test`
- `jaccard_test`
- `jp_coloring_test`
- `kruskal_test`
- `mis_test`
- `new_dfs_test`
- `page_rank_test`
- `prim_test`
- `spanning_tree_test`
- `spMatspMat_test`
- `tc_test`

### Adaptor Tests
- `back_edge_test`
- `bfs_edge_range_test`
- `dag_range_test`
- `filtered_bfs_test`
- `random_range_test`
- `range_adaptors_test`
- `reverse_test`
- `vertex_range_test`
- `worklist_test`

### I/O Tests
- `mmio_test`

### Utility Tests
- `atomic_test`
- `atomic_bit_vector_test`
- `build_test`
- `demangle_test`
- `disjoint_set_test`
- `intersection_size_test`
- `permutation_iterator_test`
- `priority_queue_test`
- `provenance_test`
- `proxy_sort_test`
- `tag_invoke_test`
- `timer_test`
- `util_test`

### Built-in Graph Tests
- `builtin_graphs_test`

### Disabled Tests (need investigation)
- `bk_test` - Betweenness centrality
- `dfs_range_test` - Header has vertex_id_type bug
- `kcore_test` - K-core decomposition
- `max_flow_test` - Maximum flow
- `rcm_test` - Reverse Cuthill-McKee

## Running Tests

### Build and Run All Tests

```bash
cd build
cmake ..
make -j$(nproc)
ctest --output-on-failure
```

### Run Specific Test Categories

```bash
ctest -R "container"     # Container tests
ctest -R "algorithm"     # Algorithm tests
ctest -R "adaptor"       # Adaptor tests
ctest -R "util"          # Utility tests
```

### Run Individual Tests

```bash
./test/dijkstra_test.exe
./test/bfs_test_0.exe
./test/graph_concepts_test.exe
```

## Test Data

Test data files are located in `data/` directory:
- `karate.mtx` - Zachary's karate club (34 vertices, 78 edges)
- `football.mtx` - Football network
- Various other Matrix Market files

## Test Utilities

### common/test_header.hpp

Standard includes for all tests:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
// ... other common includes
```

### common/karate.hpp

In-memory karate graph for quick tests without file I/O.

## Coverage Summary

| Category | Total Headers | Tested | Coverage |
|----------|---------------|--------|----------|
| Adaptors | 17 | 14 | 82% |
| Core | 13 | 13 | 100% |
| Algorithms | 18 | 16 | 89% |
| Containers | 4 | 4 | 100% |
| Graphs | 3 | 3 | 100% |
| I/O | 2 | 2 | 100% |
| Utilities | 18 | 15 | 83% |
| **Total** | **75** | **67** | **89%** |

Note: Some headers are not tested due to:
- TBB-specific functionality (requires TBB for testing)
- Broken headers (syntax errors or bugs)
- Type definitions only (no testable behavior)
- Empty/stub implementations (k_truss.hpp)
