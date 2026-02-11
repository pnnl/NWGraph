# NWGraph Parallel Backends

NWGraph supports three parallel backend configurations, selected at compile time:

| Backend | CMake Option | Runtime | Description |
|---------|-------------|---------|-------------|
| **TBB** (default) | _(none, or `-DNWGRAPH_BACKEND_HPX=OFF`)_ | Intel oneTBB | `std::execution` policies backed by TBB's work-stealing scheduler |
| **HPX** | `-DNWGRAPH_BACKEND_HPX=ON` | HPX runtime | HPX execution policies with distributed parallelism support |
| **None** | `-DNWGRAPH_BACKEND_NONE=ON` | _(none)_ | Sequential fallback; no external parallel runtime needed |

Only one backend can be active at a time.

## Compiler Requirements

NWGraph requires a compiler with C++20 support and (for TBB and HPX backends) `std::execution` policy support.

**macOS**: Use g++ from Homebrew, not Apple Clang. Apple Clang lacks `<execution>` header support.

```bash
brew install gcc@13
# Then pass -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-13 to cmake
```

**Linux**: g++-11 or later. Most distributions ship a suitable version.

**Important**: Both HPX and NWGraph must be built with the same compiler. If you build HPX from source with g++-13, build NWGraph with g++-13.

## Building NWGraph

### With TBB Backend (Default)

```bash
mkdir build && cd build

# Linux (system g++):
cmake .. -DCMAKE_BUILD_TYPE=Release

# macOS (Homebrew g++):
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-13

make -j$(nproc)
```

TBB is found automatically if installed system-wide. For custom locations:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DTBB_ROOT=/path/to/tbb
# or: export TBB_ROOT=/path/to/tbb (also accepts TBBROOT)
```

### With HPX Backend

HPX must be installed before building. See [Installing HPX](#installing-hpx) below.

```bash
mkdir build-hpx && cd build-hpx

# Linux:
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DNWGRAPH_BACKEND_HPX=ON \
         -DHPX_ROOT=/path/to/hpx

# macOS:
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DNWGRAPH_BACKEND_HPX=ON \
         -DHPX_ROOT=~/usr/local/hpx \
         -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-13

make -j$(nproc)
```

`HPX_ROOT` can also be set as an environment variable:
```bash
export HPX_ROOT=~/usr/local/hpx
```

### Without Any Parallel Backend

For environments where neither TBB nor HPX is available (e.g., minimal containers, quick compilation checks):

```bash
mkdir build-seq && cd build-seq
cmake .. -DCMAKE_BUILD_TYPE=Release -DNWGRAPH_BACKEND_NONE=ON
make -j$(nproc)
```

All parallel algorithms fall back to sequential execution. No `<execution>` header or parallel runtime is required.

### Build Options Summary

| Option | Default | Description |
|--------|---------|-------------|
| `NWGRAPH_BACKEND_HPX` | `OFF` | Use HPX backend |
| `NWGRAPH_BACKEND_NONE` | `OFF` | No parallel backend (sequential) |
| `TBB_ROOT` | _(auto)_ | Path to TBB installation |
| `HPX_ROOT` | _(auto)_ | Path to HPX installation |
| `NWGRAPH_BUILD_TESTS` | `ON` | Build unit tests |
| `NWGRAPH_BUILD_BENCH` | `OFF` | Build benchmarks |
| `NWGRAPH_BUILD_DOCS` | `OFF` | Build documentation |
| `NWGRAPH_BUILD_EXAMPLES` | `OFF` | Build examples |

## Installing HPX

### From Homebrew (macOS)

```bash
brew install hpx
```

### From Source

```bash
git clone https://github.com/STEllAR-GROUP/hpx.git
cd hpx && mkdir build && cd build

# macOS:
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-13 \
         -DCMAKE_INSTALL_PREFIX=~/usr/local/hpx

# Linux:
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=~/usr/local/hpx

make -j$(nproc)
make install
```

## Execution Policy Compatibility Layer

NWGraph provides a backend-agnostic API in `nw::graph::execution` that maps to the active backend:

```cpp
#include "nwgraph/util/execution_policy.hpp"

// Policy instances — same names regardless of backend
nw::graph::execution::seq        // sequential
nw::graph::execution::par        // parallel
nw::graph::execution::par_unseq  // parallel unsequenced

// Algorithm wrappers
nw::graph::par_sort(policy, first, last, comp);
nw::graph::par_for_each(policy, first, last, func);
nw::graph::par_copy(policy, first, last, dest);
nw::graph::par_unique(policy, first, last, pred);
nw::graph::par_fill(policy, first, last, value);
nw::graph::par_equal(policy, first1, last1, first2);
nw::graph::par_inclusive_scan(policy, first, last, dest);
nw::graph::par_transform(policy, first, last, dest, op);
nw::graph::par_stable_sort(policy, first, last, comp);
```

These wrappers dispatch to `hpx::`, `std::` (with TBB), or plain sequential `std::` depending on the backend.

## Backend-Specific Features

The compatibility layer covers the common `std::execution`-style algorithms. Each backend also provides features that **have no cross-backend equivalent**. Code using these must be guarded:

```cpp
#include "nwgraph/util/backend.hpp"

// Compile-time checks
#if NWGRAPH_BACKEND_TBB_ENABLED
  // TBB-specific code
#endif

#if NWGRAPH_BACKEND_HPX_ENABLED
  // HPX-specific code
#endif

// Runtime checks
if constexpr (nw::graph::backend::is_tbb()) { ... }
if constexpr (nw::graph::backend::is_hpx()) { ... }
if constexpr (nw::graph::backend::is_parallel()) { ... }
```

### TBB-Specific Features (no std/HPX equivalent)

| Feature | API | Notes |
|---------|-----|-------|
| Thread count control | `tbb::global_control(tbb::global_control::max_allowed_parallelism, N)` | Scoped; no `std::` equivalent |
| Parallel reduce | `tbb::parallel_reduce(tbb::blocked_range<T>(...), ...)` | `std::reduce` exists but TBB's version supports arbitrary blocked ranges and join semantics |
| Parallel for with ranges | `tbb::parallel_for(tbb::blocked_range<T>(...), ...)` | Explicit chunking control via grain size |
| Task groups | `tbb::task_group` | Fork-join task parallelism |
| Flow graphs | `tbb::flow::graph` | Dataflow parallelism |
| Scalable allocator | `tbb::scalable_allocator<T>` | Thread-safe memory allocation |

### HPX-Specific Features (no std/TBB equivalent)

| Feature | API | Notes |
|---------|-----|-------|
| Thread count control | `--hpx:threads=N` (command line) or `backend::set_num_threads(N)` (before first use) | Must be set before HPX runtime starts |
| Parallel for_loop | `hpx::for_loop(policy, begin, end, f)` | Index-based parallel loop |
| Futures / async | `hpx::async()`, `hpx::future<T>` | Asynchronous task execution |
| Distributed parallelism | `hpx::find_here()`, remote actions | Execution across multiple nodes |
| Thread binding | `--hpx:bind=scatter\|compact\|balanced` | NUMA-aware thread placement |
| Transform reduce | `hpx::transform_reduce(policy, ...)` | Fused transform + reduce |

### Features Common to the Compatibility Layer

| Feature | TBB | HPX | None |
|---------|-----|-----|------|
| `par_sort` | `std::sort` (parallel via TBB) | `hpx::sort` | `std::sort` (sequential) |
| `par_for_each` | `std::for_each` (parallel via TBB) | `hpx::for_each` | `std::for_each` (sequential) |
| `par_copy` | `std::copy` (parallel via TBB) | `hpx::copy` | `std::copy` (sequential) |
| `par_unique` | `std::unique` (parallel via TBB) | `hpx::unique` | `std::unique` (sequential) |
| `par_fill` | `std::fill` (parallel via TBB) | `hpx::fill` | `std::fill` (sequential) |
| `par_equal` | `std::equal` (parallel via TBB) | `hpx::equal` | `std::equal` (sequential) |
| `par_inclusive_scan` | `std::inclusive_scan` (parallel via TBB) | `hpx::inclusive_scan` | `std::inclusive_scan` (sequential) |
| `par_transform` | `std::transform` (parallel via TBB) | `hpx::transform` | `std::transform` (sequential) |

## Known Shortcomings

### HPX Backend

- **Custom iterator compatibility**: HPX parallel algorithms have compatibility issues with NWGraph's custom iterators (e.g., `soa_iterator`). Some algorithms may fail to compile when used with these types. The triangle counting benchmark (`tc.cpp`) cannot currently run with the HPX backend for this reason.
- **Runtime initialization overhead**: HPX requires runtime initialization before any parallel operation. NWGraph handles this lazily via `backend::init_guard`, but the first parallel call incurs startup cost.
- **No `parallel_reduce` in compatibility layer**: HPX provides `hpx::reduce` and `hpx::transform_reduce`, but these don't map 1:1 to TBB's `parallel_reduce` with `blocked_range` and join semantics. Code using `tbb::parallel_reduce` must be rewritten for HPX.
- **Thread count is immutable after startup**: Once the HPX runtime starts, the thread count cannot be changed. `backend::set_num_threads()` must be called before the first parallel operation.

### TBB Backend

- **`parallel_reduce` is TBB-specific**: TBB's `parallel_reduce` with `blocked_range` and custom join is a powerful pattern with no `std::execution` equivalent. Code using it is not portable to HPX or the sequential backend without rewriting.
- **Thread control is TBB-specific**: `tbb::global_control` for limiting parallelism has no `std::` equivalent.
- **libstdc++ dependency**: On Linux with g++, `std::execution` parallel policies dispatch to TBB internally. Without TBB linked, `std::execution::par` may silently execute sequentially.

### Sequential Backend (None)

- **No parallelism**: All `par_*` wrappers execute sequentially. The `par` and `par_unseq` policies are accepted syntactically but do not provide parallel execution.
- **No thread control**: `backend::set_num_threads()` is a no-op.
- **No `parallel_reduce`**: Code guarded by `#if NWGRAPH_BACKEND_TBB_ENABLED` or `#if NWGRAPH_BACKEND_HPX_ENABLED` will be skipped entirely.

### General

- **Only one backend at a time**: You cannot mix TBB and HPX in the same build.
- **Backend-specific code requires guards**: Any use of `tbb::*` or `hpx::*` APIs directly must be wrapped in `#if NWGRAPH_BACKEND_TBB_ENABLED` / `#if NWGRAPH_BACKEND_HPX_ENABLED` preprocessor guards.

## Running Applications

### Thread Control

**TBB:**
```cpp
#include <tbb/global_control.h>

// Scoped thread limiter
tbb::global_control gc(tbb::global_control::max_allowed_parallelism, 4);
```

**HPX** (command line):
```bash
./my_app --hpx:threads=8
./my_app --hpx:threads=all
./my_app --hpx:bind=scatter    # NUMA-aware placement
```

**HPX** (programmatic, before first parallel operation):
```cpp
#include "nwgraph/util/backend.hpp"
nw::graph::backend::set_num_threads(4);
```

### HPX Runtime Options

```bash
--hpx:threads=N       # Number of OS threads
--hpx:cores=N         # Number of cores
--hpx:bind=<policy>   # Thread binding (none, compact, scatter, balanced)
--hpx:print-bind      # Print thread binding information
--hpx:help            # Show all HPX options
```

## Running Tests

```bash
# TBB backend
cd build && ctest --output-on-failure

# HPX backend
cd build-hpx && ctest --output-on-failure

# Sequential backend
cd build-seq && ctest --output-on-failure
```

## Benchmarks

```bash
cmake .. -DNWGRAPH_BUILD_BENCH=ON
make parallel_for_scaling parallel_reduce_scaling

# TBB
./bench/scalability/parallel_for_scaling -t 8 -n 5 -s 10000000

# HPX
./bench/scalability/parallel_for_scaling -t 8 -n 5 -s 10000000 --hpx:threads=8
```

## Troubleshooting

### HPX Runtime Not Initialized
If you see HPX runtime errors, ensure `backend::init_guard` is used:
```cpp
#include "nwgraph/util/backend.hpp"

int main() {
    nw::graph::backend::init_guard guard;
    // ... code ...
}
```

### Apple Clang Build Failures
Apple Clang does not support `<execution>`. Use Homebrew g++:
```bash
cmake .. -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-13
```

### TBB Not Found
```bash
# Homebrew
brew install tbb
# Or specify path
cmake .. -DTBB_ROOT=/opt/homebrew
```

### HPX Not Found
Specify the full path (not `~`, use `$HOME`):
```bash
cmake .. -DHPX_ROOT=$HOME/usr/local/hpx
```

## References

- [HPX GitHub Repository](https://github.com/STEllAR-GROUP/hpx)
- [HPX Documentation](https://hpx-docs.stellar-group.org/latest/html/index.html)
- [Intel oneTBB GitHub Repository](https://github.com/oneapi-src/oneTBB)
- [NWGraph Documentation](https://pnnl.github.io/NWGraph/)
