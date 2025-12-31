# NWGraph Examples Specification

## Overview

Examples demonstrate usage patterns and real-world applications of the NWGraph library. Unlike unit tests (which verify correctness), examples show how to use the library effectively. Examples should be well-documented, self-contained, and progressively introduce library concepts.

## Directory Structure

```
examples/
├── bgl-book/               # Examples from "The Boost Graph Library" book
│   ├── ch3_toposort.cpp    # Chapter 3: Topological sort
│   ├── ch4_kevin_bacon.cpp # Chapter 4: BFS (Six Degrees of Kevin Bacon)
│   ├── ch5_dijkstra.cpp    # Chapter 5: Shortest paths
│   ├── ch5_bellman_ford.cpp
│   ├── ch6_kruskal.cpp     # Chapter 6: MST algorithms
│   ├── ch6_prim.cpp
│   ├── ch7_connected.cpp   # Chapter 7: Connected components
│   ├── ch7_strongly_connected.cpp
│   ├── ch8_maxflow.cpp     # Chapter 8: Maximum flow
│   ├── ch9_knights_tour.cpp
│   └── CMakeLists.txt
│
├── imdb/                   # IMDB database examples
│   ├── imdb.cpp            # Core IMDB processing
│   ├── oracle.cpp          # Oracle of Bacon implementation
│   ├── dblp.cpp            # DBLP academic network
│   ├── dns.cpp             # DNS network analysis
│   └── CMakeLists.txt
│
├── algorithms/             # Algorithm demonstrations (TO BE CREATED)
│   ├── bfs_example.cpp     # Breadth-first search
│   ├── dijkstra_example.cpp # Dijkstra's shortest paths
│   ├── pagerank_example.cpp # PageRank centrality
│   ├── triangle_count_example.cpp # Triangle counting
│   ├── connected_components_example.cpp
│   ├── mst_example.cpp     # Minimum spanning tree
│   └── CMakeLists.txt
│
├── tutorial/               # Progressive tutorial examples (TO BE CREATED)
│   ├── 01_edge_list.cpp    # Creating edge lists
│   ├── 02_adjacency.cpp    # Converting to adjacency representation
│   ├── 03_io.cpp           # Reading/writing graphs
│   ├── 04_traversal.cpp    # BFS and DFS traversal
│   ├── 05_algorithms.cpp   # Running algorithms
│   ├── 06_parallel.cpp     # Parallel execution with TBB
│   └── CMakeLists.txt
│
└── CMakeLists.txt          # Master examples CMake file
```

## BGL Book Examples

Examples adapted from "The Boost Graph Library" by Jeremy G. Siek, Lie-Quan Lee, and Andrew Lumsdaine (Addison-Wesley, 2002). These examples demonstrate how NWGraph provides similar functionality to BGL using modern C++20 idioms.

### Status

| File | Status | Description |
|------|--------|-------------|
| `ch3_toposort.cpp` | Implemented | Topological sort for dependency ordering |
| `ch4_kevin_bacon.cpp` | Implemented | BFS for computing "Bacon numbers" |
| `ch4_loop_detection.cpp` | Implemented | DFS cycle detection (back edge finding) |
| `ch5_dijkstra.cpp` | Implemented | Dijkstra's SSSP using OSPF network data |
| `ch5_bellman_ford.cpp` | Implemented | Bellman-Ford SSSP with negative cycle detection |
| `ch6_kruskal.cpp` | Implemented | Kruskal's MST algorithm |
| `ch6_prim.cpp` | Implemented | Prim's MST algorithm |
| `ch7_connected.cpp` | Implemented | BFS-based connected components |
| `ch7_strongly_connected.cpp` | Implemented | Tarjan's SCC algorithm for web page analysis |
| `ch8_maxflow.cpp` | Implemented | Edmonds-Karp maximum flow algorithm |
| `ch9_knights_tour.cpp` | Implemented | Implicit graph with Warnsdorff's heuristic |

### Implementation Requirements

Each BGL book example should:
1. Include proper copyright and license headers
2. Use NWGraph's modern C++20 API (not old BGL-style paths)
3. Include clear comments explaining the algorithm
4. Read data from command-line arguments or bundled data files
5. Print meaningful output demonstrating the algorithm's results

### Data Files

- `kevin-bacon.dat` - Actor-movie-actor connections for Bacon number computation
- `makefile-dependencies.mmio` - Makefile dependencies for topological sort
- `makefile-target-names.dat` - Target names for topological sort output

## IMDB Examples

Real-world examples using Internet Movie Database data to demonstrate graph analysis at scale.

### Files

| File | Description |
|------|-------------|
| `imdb.cpp` | Core IMDB graph loading and processing |
| `oracle.cpp` | "Oracle of Bacon" - computing shortest paths between actors |
| `dblp.cpp` | DBLP academic collaboration network analysis |
| `dns.cpp` | DNS network topology analysis |
| `oracle+dblp.cpp` | Combined analysis across multiple networks |
| `main.cpp` | Main driver program |

### Implementation Requirements

- Use NWGraph's Matrix Market I/O for graph loading
- Demonstrate BFS for shortest path queries
- Show how to handle large graphs efficiently
- Include timing and performance metrics

## Algorithm Examples

Standalone examples demonstrating each major NWGraph algorithm. Each example should be self-contained and runnable.

### Required Examples

| Example | Algorithm | Header | Status |
|---------|-----------|--------|--------|
| `bfs_example.cpp` | Breadth-First Search | `algorithms/bfs.hpp` | TO DO |
| `dijkstra_example.cpp` | Dijkstra's SSSP | `algorithms/dijkstra.hpp` | TO DO |
| `delta_stepping_example.cpp` | Delta-Stepping SSSP | `algorithms/delta_stepping.hpp` | TO DO |
| `pagerank_example.cpp` | PageRank | `algorithms/page_rank.hpp` | TO DO |
| `triangle_count_example.cpp` | Triangle Counting | `algorithms/triangle_count.hpp` | TO DO |
| `connected_components_example.cpp` | Connected Components | `algorithms/connected_components.hpp` | TO DO |
| `kruskal_example.cpp` | Kruskal's MST | `algorithms/kruskal.hpp` | TO DO |
| `prim_example.cpp` | Prim's MST | `algorithms/prim.hpp` | TO DO |
| `betweenness_example.cpp` | Betweenness Centrality | `algorithms/betweenness_centrality.hpp` | TO DO |
| `coloring_example.cpp` | Graph Coloring | `algorithms/jones_plassmann_coloring.hpp` | TO DO |
| `mis_example.cpp` | Maximal Independent Set | `algorithms/maximal_independent_set.hpp` | TO DO |
| `jaccard_example.cpp` | Jaccard Similarity | `algorithms/jaccard.hpp` | TO DO |

### Example Structure

Each algorithm example should follow this structure:

```cpp
/**
 * @file dijkstra_example.cpp
 * @brief Demonstrates Dijkstra's shortest path algorithm using NWGraph
 *
 * This example shows how to:
 * - Load a weighted graph from Matrix Market format
 * - Run Dijkstra's algorithm from a source vertex
 * - Interpret and display the results
 */

#include <iostream>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/algorithms/dijkstra.hpp"

using namespace nw::graph;

int main(int argc, char* argv[]) {
    // 1. Parse command-line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <graph.mtx> [source_vertex]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    size_t source = (argc > 2) ? std::stoul(argv[2]) : 0;

    // 2. Load the graph
    std::cout << "Loading graph from " << filename << "..." << std::endl;
    auto edges = read_mm<directedness::directed, double>(filename);
    adjacency<0, double> graph(edges);

    std::cout << "Graph has " << num_vertices(graph) << " vertices" << std::endl;

    // 3. Run the algorithm
    std::cout << "Running Dijkstra from vertex " << source << "..." << std::endl;
    auto [distances, predecessors] = dijkstra(graph, source);

    // 4. Display results
    std::cout << "\nShortest distances from vertex " << source << ":" << std::endl;
    for (size_t v = 0; v < distances.size(); ++v) {
        std::cout << "  Vertex " << v << ": " << distances[v] << std::endl;
    }

    return 0;
}
```

## Tutorial Examples

Progressive examples that teach NWGraph concepts step-by-step.

### 01_edge_list.cpp - Creating Graphs

```cpp
// Demonstrates:
// - Creating an edge_list manually
// - Using push_back for dynamic construction
// - Initializer list construction
// - Directed vs undirected graphs
```

### 02_adjacency.cpp - Graph Representations

```cpp
// Demonstrates:
// - Converting edge_list to adjacency (CSR)
// - Forward (adjacency<0>) vs transpose (adjacency<1>) views
// - Iterating over vertices and neighbors
// - num_vertices(), degree computation
```

### 03_io.cpp - Graph I/O

```cpp
// Demonstrates:
// - Reading Matrix Market files with read_mm<>
// - Writing graphs to files
// - Handling weighted vs unweighted graphs
```

### 04_traversal.cpp - Graph Traversal

```cpp
// Demonstrates:
// - BFS using bfs_range
// - DFS traversal patterns
// - Level-synchronous BFS
// - Edge classification (tree, back, forward, cross edges)
```

### 05_algorithms.cpp - Running Algorithms

```cpp
// Demonstrates:
// - Running BFS, PageRank, Triangle Counting
// - Interpreting algorithm outputs
// - Choosing between algorithm variants
```

### 06_parallel.cpp - Parallel Execution

```cpp
// Demonstrates:
// - Enabling TBB parallelism
// - Parallel BFS and PageRank
// - Performance considerations
```

## Separation of Concerns

### Examples vs Tests

| Aspect | Examples | Unit Tests |
|--------|----------|------------|
| **Purpose** | Demonstrate usage | Verify correctness |
| **Audience** | Library users | Library developers |
| **Output** | Human-readable | Pass/fail assertions |
| **Framework** | None (standalone) | Catch2 |
| **Location** | `examples/` | `test/` |
| **Naming** | `*_example.cpp` | `*_test.cpp` |

### Migrating test/*_eg.cpp Files

The `test/*_eg.cpp` files are demonstration programs that should be:
1. **Migrated to examples/**: If they demonstrate useful patterns
2. **Converted to tests**: If they verify specific functionality
3. **Removed**: If they're redundant with other examples/tests

Current `test/*_eg.cpp` files (34 total):
- Many demonstrate single features and could become tutorial examples
- Some overlap with algorithm examples
- Some are stubs or incomplete

## Build Integration

### CMakeLists.txt Structure

```cmake
# examples/CMakeLists.txt
add_subdirectory(bgl-book)
add_subdirectory(imdb)
add_subdirectory(algorithms)
add_subdirectory(tutorial)

# examples/algorithms/CMakeLists.txt
macro(nwgraph_add_example EXENAME)
  add_executable(${EXENAME} ${EXENAME}.cpp)
  target_link_libraries(${EXENAME} nwgraph)
  target_compile_definitions(${EXENAME} PRIVATE DATA_DIR="${CMAKE_SOURCE_DIR}/data/")
endmacro()

nwgraph_add_example(bfs_example)
nwgraph_add_example(dijkstra_example)
# ... etc
```

### Build Option

```cmake
option(NWGRAPH_BUILD_EXAMPLES "Build example programs" OFF)
```

## Documentation Integration

Examples should be referenced in:
1. **Sphinx documentation**: Include examples in user guide
2. **Doxygen**: Reference examples in API documentation
3. **README.md**: Quick-start examples

### Example Documentation Comments

```cpp
/**
 * @example dijkstra_example.cpp
 * @brief Demonstrates Dijkstra's shortest path algorithm
 *
 * This example shows how to compute single-source shortest paths
 * using Dijkstra's algorithm on a weighted directed graph.
 *
 * @see nw::graph::dijkstra
 * @see nw::graph::algorithms/dijkstra.hpp
 */
```

## Quality Requirements

All examples must:

1. **Compile cleanly** with `-Wall -Wextra -pedantic`
2. **Run successfully** on test data in `data/` directory
3. **Include proper headers** (copyright, license, authors)
4. **Use modern C++20** features consistently
5. **Be well-commented** explaining what each section does
6. **Handle errors gracefully** (file not found, invalid input)
7. **Print meaningful output** showing algorithm results

## Implementation Priority

1. **High Priority**: BGL book examples (complete stubs)
2. **High Priority**: Tutorial examples (teach fundamentals)
3. **Medium Priority**: Algorithm examples (comprehensive coverage)
4. **Medium Priority**: IMDB examples (clean up existing)
5. **Lower Priority**: Migrate useful `test/*_eg.cpp` files
