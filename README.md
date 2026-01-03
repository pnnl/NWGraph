<!--
SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
SPDX-FileCopyrightText: 2022 University of Washington

SPDX-License-Identifier: BSD-3-Clause
-->

![Build with gcc-11](https://github.com/pnnl/nwgraph/workflows/Build%20with%20gcc-11/badge.svg?branch=master)
![Build with gcc-11](https://github.com/pnnl/nwgraph/workflows/Build%20with%20gcc-11%20(Mac)/badge.svg?branch=master)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/0788903a1d134b47b351e6a346123875)](https://www.codacy.com?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=NWmath/NWgr&amp;utm_campaign=Badge_Grade)

# NWGraph: Northwest Graph Library

NWGraph is a high-performance header-only generic C++ graph library, based on C++20 `concept` and `range` language features. It consists of multiple graph algorithms for well-known graph kernels and supporting data structures. Both sequential and parallel algorithms are available.

## Quick Start

```bash
# Clone the repository
git clone https://github.com/pnnl/nwgraph.git
cd nwgraph

# Build (tests only)
mkdir build && cd build
cmake ..
make -j8

# Run tests
ctest
```

## Requirements

| Requirement | Minimum Version | Notes |
|-------------|-----------------|-------|
| CMake | 3.20+ | Build system |
| C++ Compiler | GCC 11+ or Clang 14+ | C++20 support required |
| oneTBB | 2021+ | Parallel backend |

### Installing Dependencies

**macOS (Homebrew):**
```bash
brew install cmake gcc tbb
```

**Ubuntu/Debian:**
```bash
sudo apt install cmake g++-11 libtbb-dev
```

**Intel oneAPI (all platforms):**
```bash
# Download from:
# https://www.intel.com/content/www/us/en/developer/articles/tool/oneapi-standalone-components.html#onetbb
```

## Building NWGraph

### Basic Build

```bash
mkdir build && cd build
cmake ..
make -j8
```

### Build Everything

```bash
cmake .. -DNWGRAPH_BUILD_TESTS=ON \
         -DNWGRAPH_BUILD_EXAMPLES=ON \
         -DNWGRAPH_BUILD_BENCH=ON
make -j8
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `NWGRAPH_BUILD_TESTS` | ON | Build unit tests |
| `NWGRAPH_BUILD_EXAMPLES` | OFF | Build BGL book examples |
| `NWGRAPH_BUILD_BENCH` | OFF | Build GAP benchmarks |
| `NWGRAPH_BUILD_DOCS` | OFF | Build documentation |
| `CMAKE_BUILD_TYPE` | Release | Build type (Release/Debug) |

### Build Output

```
build/
├── test/                    # Unit tests
├── examples/bgl-book/       # BGL book examples
└── bench/
    ├── gapbs/               # GAP Benchmark Suite (bfs, cc, pr, sssp, bc, tc)
    └── abstraction_penalty/ # Abstraction penalty benchmarks
```

### Compiler Selection

```bash
cmake .. -DCMAKE_CXX_COMPILER=g++-11
```

### TBB Configuration

If CMake cannot find TBB:
```bash
TBBROOT=/opt/intel/oneapi/tbb/2021.5.1 cmake ..
```

## Building Documentation

NWGraph documentation is built using Sphinx with Doxygen integration.

### Documentation Dependencies

```bash
# Install Python dependencies
pip install sphinx sphinx_rtd_theme breathe exhale sphinxcontrib-bibtex

# Install Doxygen
brew install doxygen      # macOS
sudo apt install doxygen  # Ubuntu
```

### Build Documentation

**Option 1: Via CMake (recommended)**
```bash
cmake .. -DNWGRAPH_BUILD_DOCS=ON
make docs
```

**Option 2: Direct Sphinx build**
```bash
cd doc-src/sphinx
pip install -r requirements.txt
make html
```

### Documentation Targets

| Target | Description |
|--------|-------------|
| `make docs` | Build complete documentation (Doxygen + Sphinx) |
| `make docs-html` | Build HTML only (faster, uses cached Doxygen) |
| `make docs-clean` | Clean built documentation |
| `make docs-open` | Build and open in browser (macOS/Linux) |

Documentation output: `doc-src/sphinx/_build/html/index.html`

## Running Benchmarks

### Build Benchmarks

```bash
cmake .. -DNWGRAPH_BUILD_BENCH=ON
make bench -j8
```

### Quick Test

```bash
./bench/gapbs/bfs -f ../test/data/karate.mtx -n 3 --verify
./bench/gapbs/cc -f ../test/data/karate.mtx --verify
```

### GAP Benchmark Suite

| Kernel | Description | Best Version |
|--------|-------------|--------------|
| bfs | Breadth-First Search | `--version 11` |
| sssp | Single-Source Shortest Path | `--version 12` |
| pr | PageRank | `--version 11` |
| cc | Connected Components | `--version 7` |
| bc | Betweenness Centrality | `--version 5` |
| tc | Triangle Counting | `--version 4` |

### Download GAP Graphs

```bash
# List available graphs
./scripts/download_gap_graphs.sh --list

# Download road network (smallest, ~1GB)
./scripts/download_gap_graphs.sh road

# Download all real-world graphs
./scripts/download_gap_graphs.sh all
```

### Example Benchmark Commands

```bash
# BFS with direction-optimizing algorithm
./bench/gapbs/bfs -f data/graphs/road.gr --version 11 -n 64

# PageRank with 1000 iterations
./bench/gapbs/pr -f data/graphs/web.mtx -i 1000

# Triangle counting with relabeling
./bench/gapbs/tc -f data/graphs/twitter.el --relabel --upper --version 4
```

## Running Examples

### Build Examples

```bash
cmake .. -DNWGRAPH_BUILD_EXAMPLES=ON
make -j8
```

### Run BGL Book Examples

```bash
./examples/bgl-book/ch3_toposort      # Topological sort
./examples/bgl-book/ch4_kevin_bacon   # Six degrees of Kevin Bacon
./examples/bgl-book/ch5_dijkstra      # Dijkstra's algorithm
./examples/bgl-book/ch6_kruskal       # Kruskal's MST
```

## Project Organization

```
NWGraph/
├── include/nwgraph/           # Header-only library
│   ├── adaptors/              # Range adaptors (bfs_range, dfs_range, etc.)
│   ├── algorithms/            # Graph algorithms
│   ├── containers/            # Graph containers
│   ├── io/                    # I/O utilities
│   └── graph_concepts.hpp     # C++20 concepts
├── test/                      # Unit tests (Catch2)
├── examples/bgl-book/         # BGL book examples
├── bench/
│   ├── gapbs/                 # GAP Benchmark Suite
│   └── abstraction_penalty/   # Abstraction penalty benchmarks
├── doc-src/sphinx/            # Documentation source
└── scripts/                   # Utility scripts
```

## Supported File Formats

- **Matrix Market (.mtx)** - Standard sparse matrix format
- **Edge List (.el)** - Simple edge pairs
- **DIMACS (.gr)** - 9th DIMACS Challenge format
- **NWGraph Binary (.nw)** - Fast native format

## References

Lumsdaine, Andrew, et al. "NWGraph: A Library of Generic Graph Algorithms and Data Structures in C++ 20." In 36th European Conference on Object-Oriented Programming (ECOOP) 2022.

## License

BSD-3-Clause
