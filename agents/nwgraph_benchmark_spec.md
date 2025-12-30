# NWGraph Benchmark Specification

## Overview

NWGraph provides two benchmark suites:
1. **bench/** - Primary graph algorithm benchmarks
2. **apb/** - Abstraction Penalty Benchmarks (APB) measuring overhead of programming abstractions

## Benchmark Infrastructure

### Framework

- **Language:** C++20
- **CLI Parser:** DOCOPT library
- **Parallel Backend:** Intel OneTBB
- **Timing:** `std::chrono::high_resolution_clock`

### Core Components

**common.hpp** - Shared utilities:
- `load_graph<Directedness, Attributes>()` - Graph loading
- `build_adjacency<Adj, ExecutionPolicy>()` - Graph construction
- `build_degrees()` - Parallel degree calculation
- `build_random_sources()` - Random source generation
- `time_op()` / `time_op_verify()` - Timing with optional verification
- `Times<Extra>` - Statistics collection (min/max/avg)
- Thread management via TBB

**Log.hpp** - Structured result logging:
- UUID, hostname, date, git info
- Compiler details (name, version)
- Build type, TBB settings
- Tabular output format

## Primary Benchmarks (bench/)

### Directory Structure

```
bench/
├── CMakeLists.txt       # Build configuration
├── common.hpp           # Shared utilities
├── Log.hpp              # Logging infrastructure
├── config.h.in          # CMake configuration template
├── bfs.cpp              # Breadth-First Search
├── cc.cpp               # Connected Components
├── pr.cpp               # PageRank
├── sssp.cpp             # Single-Source Shortest Path
├── tc.cpp               # Triangle Counting
├── bc.cpp               # Betweenness Centrality
├── ProcessEdgelist.cpp  # Format conversion utility
└── SourceVerifier.cpp   # Source verification utility
```

### Benchmarked Algorithms

| Algorithm | File | Default Version | Fastest Version |
|-----------|------|-----------------|-----------------|
| BFS | `bfs.cpp` | v0 | v11 (direction-optimizing) |
| Connected Components | `cc.cpp` | v0 | v7 (Afforest) |
| PageRank | `pr.cpp` | v11 | v11 |
| SSSP | `sssp.cpp` | v0 | v12 (delta-stepping) |
| Triangle Counting | `tc.cpp` | v0 | v4 |
| Betweenness Centrality | `bc.cpp` | v0 | v5 |

### Algorithm Parameters

#### BFS
```bash
bfs.exe -f graph.mtx [OPTIONS] [THREADS]...

Options:
  -n NUM        Number of trials (default: 1)
  --version ID  Algorithm version (default: 0, fastest: 11)
  --seed NUM    Random seed for sources
  --sources F   Load sources from file
  -a NUM        Alpha parameter (default: 15)
  -b NUM        Beta parameter (default: 18)
  -B NUM        Number of bins (default: 32)
  -v            Enable verification
  -V            Verbose output
  --log FILE    Log results to file
```

#### Connected Components
```bash
cc.exe -f graph.mtx [OPTIONS] [THREADS]...

Options:
  --version ID    Algorithm version (default: 0, fastest: 7)
  --relabel       Enable vertex relabeling
  --direction D   ascending or descending
  --clean         Clean graph before processing
```

#### PageRank
```bash
pr.exe -f graph.mtx [OPTIONS] [THREADS]...

Options:
  --version ID  Algorithm version (default: 11)
  -i NUM        Iterations (default: 20)
  -t NUM        Convergence tolerance (default: 1e-4)
  -n NUM        Number of trials
```

#### SSSP (Delta-Stepping)
```bash
sssp.exe -f graph.mtx [OPTIONS] [THREADS]...

Options:
  --version ID  Algorithm version (default: 0, fastest: 12)
  --delta NUM   Delta parameter (default: 2)
  --sources F   Load sources from file
  -n NUM        Number of trials
```

#### Triangle Counting
```bash
tc.exe -f graph.mtx [OPTIONS] [THREADS]...

Options:
  --version ID  Algorithm version (default: 0, fastest: 4)
  --relabel     Enable vertex relabeling
  --lower       Lower triangular ordering
  --upper       Upper triangular ordering
  --format F    Container format (CSR, etc.)
  --heuristic   Automatic optimization selection
```

#### Betweenness Centrality
```bash
bc.exe -f graph.mtx [OPTIONS] [THREADS]...

Options:
  --version ID  Algorithm version (default: 0, fastest: 5)
  --seed NUM    Random seed
  --sources F   Load sources from file
  -n NUM        Number of trials
```

## Abstraction Penalty Benchmarks (apb/)

### Purpose

APBs measure the performance overhead of:
1. C++20 ranges and range-based loops
2. Different container implementations
3. Execution policy abstractions

### Directory Structure

```
apb/
├── CMakeLists.txt    # Build configuration
├── plain.cpp         # Plain range iteration
├── spmv.cpp          # Sparse matrix-vector multiplication
├── bfs.cpp           # BFS abstraction overhead
├── dijkstra.cpp      # Weighted shortest path
├── exec.cpp          # Execution policy benchmarks
├── dfs.cpp           # DFS overhead
├── tbb.cpp           # TBB integration overhead
└── containers.cpp    # Container format comparison
```

### Iteration Abstraction Benchmarks

Tests different programming patterns for traversing graph structures:

1. **Raw pointer-based loops** (baseline)
2. **Iterator-based for loops**
3. **Range-based for with auto**
4. **Range-based for with auto&&**
5. **Structured bindings** (C++17)
6. **std::for_each with lambdas**
7. **Counting iterators**

**Example output (spmv.exe):**
```
edge_range
raw for loop 5.234 ms
iterator based for loop 5.287 ms
range based for loop auto 5.301 ms
range based for loop auto && 5.234 ms
std for_each auto 6.123 ms
std for_each auto && 5.289 ms
counting iterator 5.312 ms
```

### Container Format Benchmarks

Compares graph storage implementations:

| Format | Description | Space |
|--------|-------------|-------|
| CSR | Compressed Sparse Row | O(V + E) |
| VOV | Vector of Vectors | Higher fragmentation |
| VOL | Vector of Lists | Poor cache locality |
| VOF | Vector of Forward Lists | Minimal node overhead |

```bash
containers.exe -f graph.mtx --format CSR --format VOV --format VOL --format VOF
```

### Execution Policy Benchmarks

Tests parallel execution overhead:
- Sequential (`std::execution::seq`)
- Parallel (`std::execution::par`)
- Parallel unsequenced (`std::execution::par_unseq`)

## Input Data

### Supported Formats

1. **Matrix Market (.mtx)** - Standard sparse matrix format
2. **NW Binary** - Native serialized format (faster loading)

### Standard Datasets (GAP Benchmark)

| Dataset | Description |
|---------|-------------|
| GAP-twitter | Large social network |
| GAP-web | Web graph |
| GAP-urand | Random graph |
| GAP-road | Road network |
| GAP-kron | Kronecker synthetic |

Each includes:
- Main graph file (e.g., `GAP-twitter.mtx`)
- Sources file (e.g., `GAP-twitter_sources.mtx`)

### Source Vertex Files

Pre-determined source vertices for reproducibility:
- Matrix Market format vectors
- Typically 64 sources for BFS/SSSP
- 4 sources for BC

## Building Benchmarks

```bash
mkdir build && cd build
cmake .. -DNWGRAPH_BUILD_BENCH=ON -DNWGRAPH_BUILD_APBS=ON
make -j4
```

**Output locations:**
- `build/bench/*.exe` - Primary benchmarks
- `build/apb/*.exe` - APB benchmarks

## Output Formats

### Console Output

```
File                    Version   Threads  Min                 Avg                 Max
karate.mtx              0         4        0.001234            0.001289            0.001345
karate.mtx              11        4        0.000456            0.000501            0.000567
```

### Log File Format

Fixed-width columns with scientific notation:

| Column | Description |
|--------|-------------|
| UUID | Unique run identifier |
| Library | "NWGRAPH" |
| Branch | Git branch |
| Revision | Git commit hash |
| CXX | Compiler name |
| CXX_ID | Compiler ID |
| CXX_VER | Compiler version |
| Build | Release/Debug |
| TBBMALLOC | TBB allocator setting |
| Date | YYYY-MM-DD |
| Host | Hostname |
| Benchmark | Algorithm name |
| Version | Algorithm version |
| Threads | Thread count |
| Graph | Graph filename |
| Time(s) | Execution time |

## Utility Programs

### ProcessEdgelist

Format conversion:
```bash
process_edge_list -d output.nw input.mtx   # Directed
process_edge_list -u output.nw input.mtx   # Undirected
```

### SourceVerifier

Verify source files:
```bash
sources -f graph.mtx -s sources.mtx -i 1 -n 64 --seed 27491095
```

## Common Options

All benchmarks support:

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help |
| `-f FILE` | Input graph file |
| `-n NUM` | Number of trials |
| `--version ID` | Algorithm version |
| `--log FILE` | Log results |
| `--log-header` | Add header to log |
| `-v, --verify` | Verify correctness |
| `-V, --verbose` | Verbose output |
| `-d, --debug` | Debug mode |
| `[THREADS]...` | Thread count(s) |

## Performance Metrics

### Per-Algorithm Metrics

| Algorithm | Metrics |
|-----------|---------|
| BFS/SSSP/BC | Time per trial, source vertex |
| PageRank | Total time, top-10 vertices |
| Triangle Counting | Time, triangle count |
| Connected Components | Time, component count |

### APB Metrics

- Time per iteration method (ms)
- Overhead percentage vs. baseline
- Container format comparison

## Performance Targets

Based on PVLDB paper results:
- Abstract iteration overhead: < 5% vs. raw loops
- Container format impact: 5-20% depending on algorithm
- Execution policy overhead: minimal with optimization

## Example Benchmark Session

```bash
# Build
cd build
cmake .. -DNWGRAPH_BUILD_BENCH=ON
make -j4

# Run BFS with direction-optimizing version
./bench/bfs.exe -f /path/to/graph.mtx --version 11 -n 10 --log results.txt

# Run PageRank with 1000 iterations
./bench/pr.exe -f /path/to/graph.mtx -i 1000 --version 11

# Run APB SpMV comparison
./apb/spmv.exe -f /path/to/graph.mtx

# Run container comparison
./apb/containers.exe -f /path/to/graph.mtx
```

## References

- **Library Paper:** Lumsdaine et al., "NWGraph: A Library of Generic Graph Algorithms and Data Structures in C++20," ECOOP 2022
- **Benchmark Paper:** PVLDB paper (referenced in README)
- **GAP Benchmark Suite:** Azad et al., IISWC 2020
