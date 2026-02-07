# NWGraph HPX Backend

NWGraph supports two parallel backends for executing graph algorithms:
- **TBB** (Intel Threading Building Blocks) - the current default backend
- **HPX** (High Performance ParalleX) - a modern C++ runtime system for parallelism and concurrency

HPX is an actively developed, standards-conforming runtime system that provides unified syntax for local and distributed parallelism. NWGraph collaborates with the [HPX team at LSU](https://github.com/STEllAR-GROUP/hpx) to ensure seamless integration.

## Prerequisites

### HPX Installation

HPX must be installed before building NWGraph with HPX support. See the [HPX GitHub repository](https://github.com/STEllAR-GROUP/hpx) for installation instructions.

Quick install on macOS with Homebrew:
```bash
brew install hpx
```

Or build from source:
```bash
git clone https://github.com/STEllAR-GROUP/hpx.git
cd hpx
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

## Building NWGraph

### With TBB Backend (Default)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### With HPX Backend

```bash
mkdir build-hpx && cd build-hpx
cmake .. -DCMAKE_BUILD_TYPE=Release -DNWGRAPH_BACKEND_HPX=ON
make -j$(nproc)
```

If HPX is installed in a non-standard location, specify the path:
```bash
# Example: HPX installed in ~/usr/local/hpx
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DNWGRAPH_BACKEND_HPX=ON \
         -DHPX_ROOT=~/usr/local/hpx
```

Or use environment variables:
```bash
export HPX_ROOT=~/usr/local/hpx
cmake .. -DCMAKE_BUILD_TYPE=Release -DNWGRAPH_BACKEND_HPX=ON
```

Common HPX installation locations:
- `~/usr/local/hpx` - User-local installation
- `/usr/local` - System-wide installation (Homebrew or `make install`)
- `/opt/hpx` - Custom system installation

### Custom TBB Location

For TBB backend with custom installation:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DTBB_ROOT=/path/to/tbb
```

Or use environment variables (Intel's convention):
```bash
export TBB_ROOT=/opt/intel/oneapi/tbb/latest   # or TBBROOT
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Path Configuration Summary

| Variable | Description |
|----------|-------------|
| `TBB_ROOT` | CMake variable or environment variable for TBB installation |
| `TBBROOT` | Alternative environment variable (Intel convention) |
| `HPX_ROOT` | CMake variable or environment variable for HPX installation |
| `HPX_DIR` | Direct path to HPX CMake config directory |

### Building with Tests

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DNWGRAPH_BUILD_TESTS=ON
make -j$(nproc)
ctest --output-on-failure
```

### Building Documentation

Prerequisites:
- Python 3 with virtual environment support
- Doxygen

**Note:** Building documentation requires several Python packages. Using a virtual environment is recommended to avoid polluting your system Python.

Set up virtual environment and install dependencies:
```bash
# Create and activate virtual environment
python3 -m venv .venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate

# Install documentation dependencies
pip install -r doc-src/sphinx/requirements.txt sphinx
```

Build documentation:
```bash
# Make sure virtual environment is activated
source .venv/bin/activate

mkdir build-docs && cd build-docs
cmake .. -DNWGRAPH_BUILD_DOCS=ON
make docs          # Build complete documentation
make docs-open     # Build and open in browser (macOS/Linux)
```

Documentation targets:
- `make docs` - Build complete documentation (Doxygen + Sphinx)
- `make docs-html` - Build HTML only (faster, uses cached Doxygen)
- `make docs-clean` - Clean built documentation
- `make docs-open` - Build and open in browser

## Backend Selection

The parallel backend is selected at compile time via the `NWGRAPH_BACKEND_HPX` CMake option:

| Option | Backend | Description |
|--------|---------|-------------|
| `OFF` (default) | TBB | Uses Intel TBB for parallelism with `std::execution` policies |
| `ON` | HPX | Uses HPX runtime for parallelism with future-proof C++ standards conformance |

## Running Applications

### TBB Backend

Applications built with the TBB backend use standard environment variables:

```bash
# Set number of threads
export TBB_NUM_THREADS=8

# Run application
./my_graph_app -f graph.mtx
```

### HPX Backend

HPX applications accept runtime configuration via command-line arguments:

```bash
# Run with specific number of threads
./my_graph_app -f graph.mtx --hpx:threads=8

# Run with all available cores
./my_graph_app -f graph.mtx --hpx:threads=all

# Show HPX runtime options
./my_graph_app --hpx:help
```

Common HPX runtime options:
- `--hpx:threads=N` - Number of OS threads to use
- `--hpx:cores=N` - Number of cores to use
- `--hpx:bind=<policy>` - Thread binding policy (none, compact, scatter, balanced)
- `--hpx:print-bind` - Print thread binding information

## Programmatic Thread Control

### TBB Backend

```cpp
#include <tbb/global_control.h>

// Limit to 4 threads
tbb::global_control gc(tbb::global_control::max_allowed_parallelism, 4);
```

### HPX Backend

```cpp
#include "nwgraph/util/backend.hpp"

// Set thread count before any parallel operations
nw::graph::backend::set_num_threads(4);

// Use ThreadLimiter for scoped thread control
{
    nw::graph::backend::ThreadLimiter limiter(4);
    // Parallel operations here use 4 threads
}
// Thread count restored after scope
```

## Execution Policy Compatibility

NWGraph provides a compatibility layer that abstracts execution policies across backends. The `nw::graph::execution` namespace provides:

```cpp
#include "nwgraph/util/execution_policy.hpp"

// Execution policy types (map to std:: or hpx:: based on backend)
nw::graph::execution::sequenced_policy
nw::graph::execution::parallel_policy
nw::graph::execution::parallel_unsequenced_policy

// Execution policy instances
nw::graph::execution::seq
nw::graph::execution::par
nw::graph::execution::par_unseq

// Default policy for NWGraph algorithms
nw::graph::default_execution_policy
```

## When to Use Each Backend

### TBB Backend
- Shared-memory multicore systems
- When using `std::execution` policies extensively in your codebase
- Rapid prototyping with familiar STL-style interfaces

### HPX Backend
- Distributed computing scenarios (cluster/HPC environments)
- When scaling beyond a single node with MPI + threads
- Future-oriented development with C++ standards-conforming parallelism
- Applications requiring asynchronous task-based parallelism
- Integration with HPX-based applications and libraries

HPX provides a unified programming model that works identically for local and distributed execution, making it easier to scale applications from laptops to supercomputers without code changes.

## Running Tests

### TBB Backend Tests

```bash
cd build
ctest --output-on-failure -R ".*"
```

### HPX Backend Tests

```bash
cd build-hpx
ctest --output-on-failure -R ".*"
```

## Benchmarks

NWGraph includes scalability benchmarks to compare backend performance:

```bash
# Build benchmarks
cmake .. -DNWGRAPH_BUILD_BENCH=ON
make parallel_for_scaling parallel_reduce_scaling

# Run with TBB
./bench/scalability/parallel_for_scaling -t 8 -n 5 -s 10000000

# Run with HPX
./bench/scalability/parallel_for_scaling -t 8 -n 5 -s 10000000 --hpx:threads=8
```

## Troubleshooting

### HPX Runtime Not Initialized
If you see errors about HPX runtime not being initialized, ensure your `main()` function is wrapped with HPX initialization or use the lazy initialization provided by NWGraph:

```cpp
#include "nwgraph/util/backend.hpp"

int main(int argc, char* argv[]) {
    // Automatic initialization on first parallel operation
    nw::graph::backend::init_guard guard;

    // Your code here
}
```

### Thread Binding Issues
On NUMA systems, thread binding can significantly impact performance:

```bash
# Scatter threads across NUMA nodes
./my_app --hpx:threads=8 --hpx:bind=scatter

# Compact binding (fill cores sequentially)
./my_app --hpx:threads=8 --hpx:bind=compact
```

## References

- [HPX GitHub Repository](https://github.com/STEllAR-GROUP/hpx)
- [HPX Documentation](https://hpx-docs.stellar-group.org/latest/html/index.html)
- [HPX: A Task Based Programming Model in a Global Address Space](https://stellar-group.org/research/)
- [Intel TBB GitHub Repository](https://github.com/oneapi-src/oneTBB)
- [NWGraph Documentation](https://pnnl.github.io/NWGraph/)
