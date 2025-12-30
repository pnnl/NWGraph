# NWGraph Codebase Specification

## Overview

NWGraph is a high-performance, header-only C++20 generic graph library developed by Pacific Northwest National Laboratory (PNNL) and University of Washington. It provides multiple graph algorithms with both sequential and parallel implementations using Intel OneTBB as the parallel backend.

**Key Features:**
- Header-only library (no binary dependencies)
- C++20 concepts and ranges for type-safe generic programming
- Parallel algorithms via Intel TBB
- Multiple graph representations (CSR, CSC, COO, edge lists, adjacency lists)
- Zero-cost abstractions through careful design

## Directory Structure

```
include/nwgraph/
├── adaptors/              # Range adaptors for graph traversal (18 files)
├── algorithms/            # Production-ready algorithms (18 files)
├── containers/            # Data structure containers (5 files)
├── experimental/          # Experimental algorithm variants (8 files)
├── generators/            # Graph generation utilities (1 file)
├── graphs/                # Built-in example graphs (3 files)
├── io/                    # Input/output routines (2 files)
├── util/                  # Utility classes and helpers (19 files)
├── graph_base.hpp         # Base classes for graph types
├── graph_concepts.hpp     # C++20 concept definitions
├── graph_traits.hpp       # Type traits for graphs
├── graph_adaptor.hpp      # Graph adaptor class
├── build.hpp              # Graph building utilities
├── edge_list.hpp          # Edge list data structure
├── adjacency.hpp          # Adjacency list data structure
├── csr.hpp                # CSR type alias
├── csc.hpp                # CSC type alias
├── coo.hpp                # COO type alias
├── access.hpp             # Access patterns (DEPRECATED)
└── compat.hpp             # Compatibility layer (DEPRECATED)
```

## Namespace Organization

```cpp
namespace nw {
  namespace graph {
    // Core types, concepts, algorithms, and data structures

    namespace pagerank {
      // PageRank helpers (trace, time_op)
    }

    namespace _tag_invoke {
      // Tag invoke infrastructure (internal)
    }
  }

  namespace util {
    // Utility classes (life_timer, etc.)
  }
}
```

## Graph Data Structures

### Compressed Sparse Row (CSR)
- **File:** `csr.hpp`
- **Type Alias:** `using csr_graph = adjacency<0>;`
- **Use Case:** Efficient forward adjacency representation

### Compressed Sparse Column (CSC)
- **File:** `csc.hpp`
- **Type Alias:** `using csc_graph = adjacency<1>;`
- **Use Case:** Transposed adjacency (reverse edges)

### Coordinate Format (COO)
- **File:** `coo.hpp`
- **Type Alias:** `using coo_graph = edge_list<directedness::directed>;`
- **Use Case:** Sparse edge storage with attributes

### Edge List
- **File:** `edge_list.hpp`
- **Primary Class:** `index_edge_list<vertex_id, graph_base_t, directedness, ...Attributes>`
- **Storage:** Struct-of-arrays for cache efficiency

### Adjacency List
- **File:** `adjacency.hpp`
- **Primary Class:** `index_adjacency<idx, index_type, vertex_id, ...Attributes>`
- **Storage:** Indexed struct-of-arrays with compressed indexing

## C++20 Concepts

Defined in `graph_concepts.hpp`:

| Concept | Description |
|---------|-------------|
| `graph<G>` | Fundamental graph type with `num_vertices()` |
| `adjacency_list_graph<G>` | Random-access graph with forward adjacency |
| `degree_enumerable_graph<G>` | O(1) degree query support |
| `edge_list_graph<G>` | Forward range of edges with source/target |
| `vertex_list_c<R>` | Forward range of scalar vertices |
| `edge_list_c<R>` | Forward range of edge tuples |

## Production Algorithms

Located in `algorithms/`:

| Algorithm | File | Description |
|-----------|------|-------------|
| BFS | `bfs.hpp` | Breadth-first search (sequential + parallel) |
| Connected Components | `connected_components.hpp` | Label propagation, Afforest |
| PageRank | `page_rank.hpp` | Iterative convergence |
| SSSP | `delta_stepping.hpp` | Delta-stepping shortest paths |
| Dijkstra | `dijkstra.hpp` | Priority queue shortest paths |
| Triangle Counting | `triangle_count.hpp` | Multiple algorithms |
| Betweenness Centrality | `betweenness_centrality.hpp` | Brandes' algorithm |
| Graph Coloring | `jones_plassmann_coloring.hpp` | Jones-Plassmann |
| Maximal Independent Set | `maximal_independent_set.hpp` | DAG-based MIS |
| K-Core | `k_core.hpp` | Iterative peeling |
| K-Truss | `k_truss.hpp` | Triangle-based edge filtering |
| Jaccard Similarity | `jaccard.hpp` | Neighbor-based similarity |
| Maximum Flow | `max_flow.hpp` | Boykov-Kolmogorov |
| MST | `prim.hpp`, `kruskal.hpp` | Minimum spanning tree |
| SpMM | `spMatspMat.hpp` | Sparse matrix multiplication |

## Experimental Algorithms

Located in `experimental/algorithms/`:

Experimental variants of core algorithms with alternative implementations:
- `bfs.hpp` - Direction-optimizing BFS
- `connected_components.hpp` - Alternative CC implementations
- `delta_stepping.hpp` - Experimental shortest path variants
- `page_rank.hpp` - Alternative PageRank formulations
- `triangle_count.hpp` - Experimental counting strategies
- `betweenness_centrality.hpp` - Experimental BC variants (1637 lines)
- `jones_plassmann_coloring.hpp` - Experimental coloring (has known bugs)
- `jaccard.hpp` - Experimental similarity variants

## Range Adaptors

Located in `adaptors/`:

| Adaptor | Purpose |
|---------|---------|
| `neighbor_range.hpp` | Range over vertex neighbors |
| `edge_range.hpp` | Range over graph edges |
| `vertex_range.hpp` | Range over vertices |
| `bfs_range.hpp` | Level-synchronous BFS traversal |
| `bfs_edge_range.hpp` | BFS returning edges |
| `dfs_range.hpp` | Depth-first traversal |
| `dag_range.hpp` | DAG-specific traversal |
| `back_edge_range.hpp` | Back edge detection |
| `filtered_bfs_range.hpp` | BFS with filtering |
| `cyclic_range_adaptor.hpp` | Cyclic iteration |
| `plain_range.hpp` | Simple vertex range |
| `random_range.hpp` | Random-order access |
| `reverse.hpp` | Reverse iteration |
| `worklist.hpp` | Work queue for parallel algorithms |
| `splittable_range_adaptor.hpp` | TBB-compatible ranges |

## Utilities

Located in `util/`:

| Utility | Purpose |
|---------|---------|
| `tag_invoke.hpp` | C++20 customization point pattern |
| `defaults.hpp` | Default types (uint32_t for vertices) |
| `parallel_for.hpp` | TBB-based parallel loops |
| `atomic.hpp` | Atomic operations |
| `AtomicBitVector.hpp` | Thread-safe bit vector |
| `timer.hpp` | High-resolution timing (life_timer RAII) |
| `disjoint_set.hpp` | Union-find data structure |
| `proxysort.hpp` | Proxy-based sorting |

## Key Type Definitions

```cpp
// Default types (util/defaults.hpp)
using default_vertex_id_type = uint32_t;
using default_index_t = uint32_t;

// Graph directedness (graph_base.hpp)
enum class succession { successor, predecessor };
enum class directedness { undirected, directed };

// Type traits pattern
template <typename G>
using vertex_id_t = typename graph_traits<G>::vertex_id_type;
```

## Customization Point Objects (CPOs)

NWGraph uses the tag_invoke pattern for extensibility:

- `num_vertices(g)` - Get vertex count
- `num_edges(g)` - Get edge count
- `degree(inner_range)` - Get neighborhood size
- `source(g, e)` - Get edge source vertex
- `target(g, e)` - Get edge target vertex

## Dependencies

**Required:**
- C++20 compiler (GCC 11+, Clang 13+, MSVC 2019+)
- Intel OneTBB (>= 2021)
- CMake (>= 3.18.2)

**Optional:**
- TBBMalloc (via `NWGRAPH_USE_TBBMALLOC`)
- SYCL/DPSTD (for GPU acceleration)

## Build Configuration

```bash
mkdir build && cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-11
make -j4
```

**CMake Options:**

| Option | Default | Purpose |
|--------|---------|---------|
| `NWGRAPH_BUILD_TESTS` | ON | Build test suite |
| `NWGRAPH_BUILD_BENCH` | OFF | Build benchmarks |
| `NWGRAPH_BUILD_APBS` | OFF | Build abstraction penalty benchmarks |
| `NWGRAPH_BUILD_EXAMPLES` | OFF | Build IMDB examples |
| `NWGRAPH_USE_TBBMALLOC` | OFF | Link TBB allocator |

## File I/O

**Matrix Market Format Support:**
- Files: `io/MatrixMarketFile.hpp`, `io/mmio.hpp`
- Supports coordinate format, real/pattern matrices
- Memory-mapped file loading for performance

## Example Usage

```cpp
#include <nwgraph/edge_list.hpp>
#include <nwgraph/csr.hpp>
#include <nwgraph/algorithms/bfs.hpp>

using namespace nw::graph;

// Create edge list
edge_list<directedness::directed> edges(num_vertices);
edges.open_for_push_back();
edges.push_back({0, 1});
edges.push_back({1, 2});
edges.close_for_push_back();

// Convert to CSR
adjacency<0> csr(edges);

// Run BFS
auto parents = bfs(csr, 0);
```

## Known Issues

1. **Deprecated headers:** `access.hpp` and `compat.hpp` are deprecated
2. **Buggy experimental code:** `experimental/algorithms/jones_plassmann_coloring.hpp` has known bugs
3. **45 instances of `#if 0`** disabled code blocks throughout the codebase
4. **Graph data headers** (`graphs/*.hpp`) lack proper include guards
5. **Debug output** (`std::cout/std::cerr`) present in some library headers
