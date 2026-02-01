# NWGraph HPX Backend

NWGraph supports two parallel backends for executing graph algorithms:
- **TBB** (Intel Threading Building Blocks) - the default backend
- **HPX** (High Performance ParalleX) - an alternative backend for distributed computing scenarios

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
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DNWGRAPH_BACKEND_HPX=ON \
         -DHPX_DIR=/path/to/hpx/lib/cmake/HPX
```

## Backend Selection

The parallel backend is selected at compile time via the `NWGRAPH_BACKEND_HPX` CMake option:

| Option | Backend | Description |
|--------|---------|-------------|
| `OFF` (default) | TBB | Uses Intel TBB for parallelism with `std::execution` policies |
| `ON` | HPX | Uses HPX runtime for parallelism |

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

## Performance Considerations

### TBB Backend
- Lower overhead for local parallelism
- Better suited for shared-memory multicore systems
- Consistent scaling behavior
- Work-stealing scheduler adapts well to irregular workloads

### HPX Backend
- Designed for distributed computing (MPI + threads)
- Supports remote procedure calls and distributed data structures
- Higher overhead for purely local parallelism
- Better suited when scaling beyond a single node

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
- [Intel TBB GitHub Repository](https://github.com/oneapi-src/oneTBB)
- [NWGraph Documentation](https://pnnl.github.io/NWGraph/)
