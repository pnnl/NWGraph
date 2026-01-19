# HPX Backend Implementation Plan for NWGraph

## Overview

This document describes the implementation plan for adding HPX (High Performance ParalleX) as an alternative parallel backend for NWGraph, alongside the existing TBB backend.

## Current Parallel Execution Model

NWGraph currently uses Intel TBB for parallelism through several mechanisms:

### 1. TBB Parallel Primitives (in `include/nwgraph/util/parallel_for.hpp`)
```cpp
tbb::parallel_for(range, body)
tbb::parallel_reduce(range, init, body, reduce)
```

### 2. TBB Blocked Ranges (direct usage in algorithms)
```cpp
tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) { ... });
tbb::parallel_reduce(tbb::blocked_range(0ul, N), init, body, reduce);
```

### 3. TBB Concurrent Containers
- `tbb::concurrent_vector<T>` - Used in BFS, betweenness centrality
- `tbb::concurrent_queue<T>` - Used in worklist adaptor
- `tbb::queuing_mutex` - Used in delta stepping

### 4. TBB Range Concept
The `splittable_range_adaptor` (in `adaptors/splittable_range_adaptor.hpp`) implements TBB's Range concept:
- Constructor taking `tbb::split` for range splitting
- `is_divisible()` method
- `size()`, `begin()`, `end()` methods

### 5. std::execution Policies
Used in `build.hpp` for sorting:
```cpp
std::sort(std::execution::par, ...)
std::stable_sort(std::execution::par, ...)
```

## Files Requiring Modification

### Core Files (must modify)
| File | TBB Usage | HPX Equivalent |
|------|-----------|----------------|
| `include/nwgraph/util/parallel_for.hpp` | `tbb::parallel_for`, `tbb::parallel_reduce` | `hpx::for_each`, `hpx::reduce` |
| `include/nwgraph/adaptors/splittable_range_adaptor.hpp` | `tbb::split`, Range concept | HPX Range concept |
| `include/nwgraph/build.hpp` | `tbb::parallel_for`, `tbb::blocked_range` | `hpx::for_loop` |

### Algorithm Files (must modify)
| File | TBB Usage |
|------|-----------|
| `include/nwgraph/algorithms/bfs.hpp` | `tbb::concurrent_vector`, `tbb::blocked_range`, `tbb::parallel_for`, `tbb::parallel_reduce` |
| `include/nwgraph/algorithms/page_rank.hpp` | `tbb::parallel_for`, `tbb::parallel_reduce`, `tbb::blocked_range` |
| `include/nwgraph/algorithms/delta_stepping.hpp` | `tbb::concurrent_vector`, `tbb::queuing_mutex`, `tbb::parallel_for_each` |
| `include/nwgraph/algorithms/betweenness_centrality.hpp` | `tbb::concurrent_vector` |

### Container Files (must modify)
| File | TBB Usage |
|------|-----------|
| `include/nwgraph/containers/compressed.hpp` | `tbb::parallel_for`, `tbb::blocked_range` |
| `include/nwgraph/adaptors/worklist.hpp` | `tbb::concurrent_queue`, `tbb::concurrent_vector` |
| `include/nwgraph/adaptors/edge_range.hpp` | `tbb::split` |
| `include/nwgraph/util/AtomicBitVector.hpp` | `tbb::split` |

## Implementation Strategy

### Phase 1: Backend Selection Infrastructure

Create a compile-time backend selection mechanism using CMake and preprocessor macros.

#### 1.1 CMake Configuration
Add to `CMakeLists.txt`:
```cmake
option(NWGRAPH_USE_HPX "Use HPX for parallel execution" OFF)
option(NWGRAPH_USE_TBB "Use TBB for parallel execution" ON)

if(NWGRAPH_USE_HPX AND NWGRAPH_USE_TBB)
  message(FATAL_ERROR "Cannot enable both HPX and TBB backends simultaneously")
endif()

if(NWGRAPH_USE_HPX)
  find_package(HPX REQUIRED)
  target_compile_definitions(nwgraph INTERFACE NWGRAPH_BACKEND_HPX)
  target_link_libraries(nwgraph INTERFACE HPX::hpx)
endif()

if(NWGRAPH_USE_TBB)
  # existing TBB setup
  target_compile_definitions(nwgraph INTERFACE NWGRAPH_BACKEND_TBB)
endif()
```

#### 1.2 Backend Header
Create `include/nwgraph/util/backend.hpp`:
```cpp
#ifndef NW_GRAPH_BACKEND_HPP
#define NW_GRAPH_BACKEND_HPP

#if defined(NWGRAPH_BACKEND_HPX)
  #include <hpx/algorithm.hpp>
  #include <hpx/execution.hpp>
  #include <hpx/parallel/algorithms/for_loop.hpp>
  #include <hpx/parallel/algorithms/reduce.hpp>
  #define NWGRAPH_PARALLEL_BACKEND "HPX"
#elif defined(NWGRAPH_BACKEND_TBB)
  #include <oneapi/tbb.h>
  #include <tbb/parallel_for.h>
  #include <tbb/parallel_reduce.h>
  #define NWGRAPH_PARALLEL_BACKEND "TBB"
#else
  #error "No parallel backend selected. Define NWGRAPH_BACKEND_HPX or NWGRAPH_BACKEND_TBB"
#endif

namespace nw::graph::backend {

#if defined(NWGRAPH_BACKEND_HPX)
  using execution_policy = hpx::execution::parallel_policy;
  inline constexpr auto par = hpx::execution::par;
  inline constexpr auto seq = hpx::execution::seq;
#elif defined(NWGRAPH_BACKEND_TBB)
  // TBB doesn't have execution policies in the same way
  // We use this for std::execution compatibility
  using execution_policy = std::execution::parallel_unsequenced_policy;
  inline constexpr auto par = std::execution::par_unseq;
  inline constexpr auto seq = std::execution::seq;
#endif

} // namespace nw::graph::backend

#endif // NW_GRAPH_BACKEND_HPP
```

### Phase 2: Parallel Primitives Abstraction

#### 2.1 Update `parallel_for.hpp`

```cpp
#ifndef NW_GRAPH_PARALLEL_FOR_HPP
#define NW_GRAPH_PARALLEL_FOR_HPP

#include "nwgraph/util/backend.hpp"
#include "nwgraph/util/traits.hpp"

#if defined(NWGRAPH_BACKEND_HPX)
  #include <hpx/parallel/algorithms/for_each.hpp>
  #include <hpx/parallel/algorithms/reduce.hpp>
  #include <hpx/parallel/algorithms/for_loop.hpp>
#elif defined(NWGRAPH_BACKEND_TBB)
  #include <tbb/parallel_for.h>
  #include <tbb/parallel_reduce.h>
#endif

namespace nw::graph {

// ... existing parallel_for_inner and parallel_for_sequential unchanged ...

template <class Range, class Op>
void parallel_for(Range&& range, Op&& op) {
#if defined(NWGRAPH_BACKEND_HPX)
  if (range.size() > range.cutoff()) {
    hpx::for_each(hpx::execution::par, range.begin(), range.end(),
                  [&](auto&& i) { parallel_for_inner(op, i); });
  } else {
    parallel_for_sequential(std::forward<Range>(range), std::forward<Op>(op));
  }
#elif defined(NWGRAPH_BACKEND_TBB)
  if (range.is_divisible()) {
    tbb::parallel_for(std::forward<Range>(range),
                      [&](auto&& sub) { parallel_for_sequential(sub, op); });
  } else {
    parallel_for_sequential(std::forward<Range>(range), std::forward<Op>(op));
  }
#endif
}

template <class Range, class Op, class Reduce, class T>
auto parallel_reduce(Range&& range, Op&& op, Reduce&& reduce, T init) {
#if defined(NWGRAPH_BACKEND_HPX)
  if (range.size() > range.cutoff()) {
    return hpx::reduce(hpx::execution::par, range.begin(), range.end(), init,
                       reduce, [&](auto&& i) { return parallel_for_inner(op, i); });
  } else {
    return parallel_for_sequential(range, op, reduce, init);
  }
#elif defined(NWGRAPH_BACKEND_TBB)
  if (range.is_divisible()) {
    return tbb::parallel_reduce(
        std::forward<Range>(range), init,
        [&](auto&& sub, auto partial) { return parallel_for_sequential(sub, op, reduce, partial); },
        reduce);
  } else {
    return parallel_for_sequential(range, op, reduce, init);
  }
#endif
}

// New: blocked parallel for (index-based)
template <class Op>
void parallel_for_blocked(std::size_t begin, std::size_t end, Op&& op) {
#if defined(NWGRAPH_BACKEND_HPX)
  hpx::for_loop(hpx::execution::par, begin, end, std::forward<Op>(op));
#elif defined(NWGRAPH_BACKEND_TBB)
  tbb::parallel_for(tbb::blocked_range(begin, end),
                    [&](auto&& r) {
                      for (auto i = r.begin(); i != r.end(); ++i) {
                        op(i);
                      }
                    });
#endif
}

// New: blocked parallel reduce (index-based)
template <class T, class Op, class Reduce>
T parallel_reduce_blocked(std::size_t begin, std::size_t end, T init, Op&& op, Reduce&& reduce) {
#if defined(NWGRAPH_BACKEND_HPX)
  return hpx::transform_reduce(hpx::execution::par,
                               hpx::util::counting_iterator(begin),
                               hpx::util::counting_iterator(end),
                               init, std::forward<Reduce>(reduce), std::forward<Op>(op));
#elif defined(NWGRAPH_BACKEND_TBB)
  return tbb::parallel_reduce(
      tbb::blocked_range(begin, end), init,
      [&](auto&& r, auto partial) {
        for (auto i = r.begin(); i != r.end(); ++i) {
          partial = reduce(partial, op(i));
        }
        return partial;
      },
      reduce);
#endif
}

} // namespace nw::graph

#endif // NW_GRAPH_PARALLEL_FOR_HPP
```

### Phase 3: Concurrent Containers Abstraction

Create `include/nwgraph/util/concurrent_containers.hpp`:

```cpp
#ifndef NW_GRAPH_CONCURRENT_CONTAINERS_HPP
#define NW_GRAPH_CONCURRENT_CONTAINERS_HPP

#include "nwgraph/util/backend.hpp"

#if defined(NWGRAPH_BACKEND_HPX)
  #include <hpx/concurrency/concurrent_queue.hpp>
  // HPX doesn't have concurrent_vector, use mutex-protected vector or atomics
  #include <mutex>
  #include <vector>
#elif defined(NWGRAPH_BACKEND_TBB)
  #include <tbb/concurrent_vector.h>
  #include <tbb/concurrent_queue.h>
  #include <tbb/queuing_mutex.h>
#endif

namespace nw::graph {

// Concurrent queue abstraction
template <typename T>
class concurrent_queue {
#if defined(NWGRAPH_BACKEND_HPX)
  hpx::concurrency::ConcurrentQueue<T> queue_;
#elif defined(NWGRAPH_BACKEND_TBB)
  tbb::concurrent_queue<T> queue_;
#endif

public:
  void push(const T& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    queue_.push(item);
#elif defined(NWGRAPH_BACKEND_TBB)
    queue_.push(item);
#endif
  }

  bool try_pop(T& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    return queue_.try_pop(item);
#elif defined(NWGRAPH_BACKEND_TBB)
    return queue_.try_pop(item);
#endif
  }

  bool empty() const {
#if defined(NWGRAPH_BACKEND_HPX)
    return queue_.empty();
#elif defined(NWGRAPH_BACKEND_TBB)
    return queue_.empty();
#endif
  }
};

// Concurrent vector abstraction
// Note: HPX doesn't have a direct equivalent, so we provide a simple thread-safe wrapper
template <typename T>
class concurrent_vector {
#if defined(NWGRAPH_BACKEND_HPX)
  std::vector<T> data_;
  mutable std::mutex mutex_;
#elif defined(NWGRAPH_BACKEND_TBB)
  tbb::concurrent_vector<T> data_;
#endif

public:
  using value_type = T;
  using size_type = std::size_t;

  concurrent_vector() = default;
  explicit concurrent_vector(size_type n) : data_(n) {}
  concurrent_vector(size_type n, const T& val) : data_(n, val) {}

  void push_back(const T& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<std::mutex> lock(mutex_);
    data_.push_back(item);
#elif defined(NWGRAPH_BACKEND_TBB)
    data_.push_back(item);
#endif
  }

  size_type size() const {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.size();
#elif defined(NWGRAPH_BACKEND_TBB)
    return data_.size();
#endif
  }

  bool empty() const {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.empty();
#elif defined(NWGRAPH_BACKEND_TBB)
    return data_.empty();
#endif
  }

  void clear() {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<std::mutex> lock(mutex_);
    data_.clear();
#elif defined(NWGRAPH_BACKEND_TBB)
    data_.clear();
#endif
  }

  T& operator[](size_type i) {
#if defined(NWGRAPH_BACKEND_HPX)
    // Note: No lock for indexed access - caller must ensure thread safety
    return data_[i];
#elif defined(NWGRAPH_BACKEND_TBB)
    return data_[i];
#endif
  }

  const T& operator[](size_type i) const {
#if defined(NWGRAPH_BACKEND_HPX)
    return data_[i];
#elif defined(NWGRAPH_BACKEND_TBB)
    return data_[i];
#endif
  }

  auto begin() { return data_.begin(); }
  auto end() { return data_.end(); }
  auto begin() const { return data_.begin(); }
  auto end() const { return data_.end(); }
};

// Mutex abstraction
class queuing_mutex {
#if defined(NWGRAPH_BACKEND_HPX)
  hpx::mutex mutex_;
#elif defined(NWGRAPH_BACKEND_TBB)
  tbb::queuing_mutex mutex_;
#endif

public:
  class scoped_lock {
#if defined(NWGRAPH_BACKEND_HPX)
    std::unique_lock<hpx::mutex> lock_;
  public:
    scoped_lock(queuing_mutex& m) : lock_(m.mutex_) {}
#elif defined(NWGRAPH_BACKEND_TBB)
    tbb::queuing_mutex::scoped_lock lock_;
  public:
    scoped_lock(queuing_mutex& m) : lock_(m.mutex_) {}
#endif
  };
};

} // namespace nw::graph

#endif // NW_GRAPH_CONCURRENT_CONTAINERS_HPP
```

### Phase 4: Range Adaptor Updates

#### 4.1 Update `splittable_range_adaptor.hpp`

```cpp
#ifndef NW_GRAPH_SPLITTABLE_RANGE_ADAPTOR_HPP
#define NW_GRAPH_SPLITTABLE_RANGE_ADAPTOR_HPP

#include <cstddef>
#include <ranges>
#include "nwgraph/util/backend.hpp"

#if defined(NWGRAPH_BACKEND_TBB)
  #include <oneapi/tbb.h>
#endif

namespace nw::graph {

// Split tag for range splitting
struct split_tag {};

template <class Iterator>
class splittable_range_adaptor : public std::ranges::view_base {
  Iterator    begin_;
  Iterator    end_;
  std::size_t cutoff_ = 16384;

public:
  using iterator   = Iterator;
  using value_type = typename iterator::value_type;

  // ... existing constructors ...

#if defined(NWGRAPH_BACKEND_TBB)
  // TBB split constructor
  splittable_range_adaptor(splittable_range_adaptor& rhs, tbb::split)
      : begin_(rhs.begin_), end_(rhs.begin_ += rhs.size() / 2), cutoff_(rhs.cutoff_) {}
#endif

  // Generic split constructor (for HPX or other backends)
  splittable_range_adaptor(splittable_range_adaptor& rhs, split_tag)
      : begin_(rhs.begin_), end_(rhs.begin_ += rhs.size() / 2), cutoff_(rhs.cutoff_) {}

  // ... existing methods ...

  std::size_t cutoff() const { return cutoff_; }
};

} // namespace nw::graph

#endif
```

### Phase 5: Algorithm Updates

Each algorithm file needs conditional compilation. Example for `bfs.hpp`:

```cpp
// Replace direct TBB usage:
// OLD:
tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) { ... });

// NEW:
nw::graph::parallel_for_blocked(0ul, N, [&](auto i) { ... });

// Replace concurrent_vector:
// OLD:
std::vector<tbb::concurrent_vector<vertex_id_type>> q1(n);

// NEW:
std::vector<nw::graph::concurrent_vector<vertex_id_type>> q1(n);
```

## Implementation Order

### Step 1: Infrastructure (1-2 hours)
1. Create `include/nwgraph/util/backend.hpp`
2. Update `CMakeLists.txt` with HPX option
3. Create `cmake/FindHPX.cmake` if needed

### Step 2: Parallel Primitives (2-3 hours)
1. Update `include/nwgraph/util/parallel_for.hpp`
2. Add `parallel_for_blocked` and `parallel_reduce_blocked`
3. Test with simple cases

### Step 3: Concurrent Containers (2-3 hours)
1. Create `include/nwgraph/util/concurrent_containers.hpp`
2. Test concurrent_queue, concurrent_vector, queuing_mutex

### Step 4: Range Adaptors (1-2 hours)
1. Update `include/nwgraph/adaptors/splittable_range_adaptor.hpp`
2. Update `include/nwgraph/adaptors/edge_range.hpp`
3. Update `include/nwgraph/util/AtomicBitVector.hpp`

### Step 5: Core Algorithms (4-6 hours)
Update in this order (simplest to most complex):
1. `include/nwgraph/algorithms/page_rank.hpp`
2. `include/nwgraph/build.hpp`
3. `include/nwgraph/containers/compressed.hpp`
4. `include/nwgraph/algorithms/bfs.hpp`
5. `include/nwgraph/algorithms/delta_stepping.hpp`
6. `include/nwgraph/algorithms/betweenness_centrality.hpp`

### Step 6: Testing (2-3 hours)
1. Add HPX-specific test configuration
2. Run existing tests with HPX backend
3. Performance comparison benchmarks

## HPX-Specific Considerations

### HPX Initialization
HPX requires explicit initialization. Options:
1. **Main wrapper**: Require HPX main
2. **Local runtime**: Use `hpx::local::init()` for library use
3. **Lazy init**: Initialize on first parallel operation

Recommended approach - lazy initialization:
```cpp
// In backend.hpp
#if defined(NWGRAPH_BACKEND_HPX)
namespace nw::graph::detail {
  inline void ensure_hpx_initialized() {
    static bool initialized = []() {
      if (!hpx::is_running()) {
        hpx::local::start(nullptr, 0, nullptr);
      }
      return true;
    }();
    (void)initialized;
  }
}
#endif
```

### HPX Execution Policies
HPX provides richer execution policies:
- `hpx::execution::par` - Parallel execution
- `hpx::execution::seq` - Sequential execution
- `hpx::execution::par_unseq` - Parallel unsequenced
- `hpx::execution::task` - Asynchronous execution (returns future)

### HPX Parallel Algorithms
Key HPX parallel algorithms to use:
- `hpx::for_each` - Parallel for_each
- `hpx::for_loop` - Index-based parallel loop
- `hpx::transform_reduce` - Parallel transform and reduce
- `hpx::reduce` - Parallel reduction

## Testing Strategy

### Unit Tests
Create `test/hpx_backend_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/util/concurrent_containers.hpp"

TEST_CASE("parallel_for_blocked", "[hpx]") {
  std::atomic<int> sum{0};
  nw::graph::parallel_for_blocked(0, 1000, [&](auto i) {
    sum += i;
  });
  REQUIRE(sum == 499500);
}

TEST_CASE("concurrent_vector", "[hpx]") {
  nw::graph::concurrent_vector<int> vec;
  // ... tests ...
}
```

### Integration Tests
Run existing algorithm tests with HPX backend:
```bash
cmake -DNWGRAPH_USE_HPX=ON -DNWGRAPH_USE_TBB=OFF ..
make
ctest
```

### Benchmark Comparison
```bash
# Build with TBB
cmake -DNWGRAPH_USE_TBB=ON ..
make bench
./bench/gapbs/bfs -f data/road.gr -n 10 > tbb_results.txt

# Build with HPX
cmake -DNWGRAPH_USE_HPX=ON ..
make bench
./bench/gapbs/bfs -f data/road.gr -n 10 > hpx_results.txt
```

## Dependencies

### HPX Requirements
- HPX 1.9.0 or later
- C++20 support
- CMake 3.18+

### Installation
```bash
# Ubuntu/Debian
sudo apt install libhpx-dev

# From source
git clone https://github.com/STEllAR-GROUP/hpx.git
cd hpx && mkdir build && cd build
cmake .. -DHPX_WITH_EXAMPLES=OFF -DHPX_WITH_TESTS=OFF
make -j8 && sudo make install
```

## Open Questions

1. **Concurrent vector**: HPX lacks a direct `concurrent_vector` equivalent. Options:
   - Use mutex-protected `std::vector` (simple but slower)
   - Use `hpx::lcos::local::channel` for producer-consumer patterns
   - Implement lock-free vector

2. **Range splitting**: TBB has built-in range splitting via `tbb::split`. HPX uses different patterns. Need to decide on abstraction.

3. **Initialization**: How to handle HPX runtime initialization in a header-only library?

4. **Mixed backends**: Should we support runtime backend selection, or compile-time only?

## References

- [HPX Documentation](https://hpx-docs.stellar-group.org/)
- [HPX Parallel Algorithms](https://hpx-docs.stellar-group.org/latest/html/libs/algorithms/api.html)
- [TBB to HPX Migration Guide](https://github.com/STEllAR-GROUP/hpx/wiki/Migration-Guide)
