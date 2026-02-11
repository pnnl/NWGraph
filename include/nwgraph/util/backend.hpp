/**
 * @file backend.hpp
 * @brief Parallel backend selection and abstraction layer.
 *
 * Provides compile-time selection among three parallel backend configurations:
 *
 *   1. HPX backend (NWGRAPH_BACKEND_HPX defined)
 *      - Lazy initialization of the HPX runtime
 *      - Thread control via HPX command-line arguments
 *
 *   2. TBB backend (NWGRAPH_BACKEND_TBB defined, the default)
 *      - Thread control via tbb::global_control
 *      - Auto-initializes on first use
 *
 *   3. No parallel backend (neither defined)
 *      - All operations execute sequentially
 *      - No external runtime dependencies
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */

#ifndef NW_GRAPH_BACKEND_HPP
#define NW_GRAPH_BACKEND_HPP

#include <cstddef>

// =============================================================================
// Backend 1: HPX
// =============================================================================
#if defined(NWGRAPH_BACKEND_HPX)

  #include <hpx/algorithm.hpp>
  #include <hpx/execution.hpp>
  #include <hpx/init.hpp>
  #include <hpx/runtime.hpp>
  #include <hpx/include/parallel_for_each.hpp>
  #include <hpx/include/parallel_for_loop.hpp>
  #include <hpx/include/parallel_reduce.hpp>
  #include <hpx/include/parallel_transform_reduce.hpp>

  #define NWGRAPH_PARALLEL_BACKEND "HPX"
  #define NWGRAPH_BACKEND_HPX_ENABLED 1
  #define NWGRAPH_BACKEND_TBB_ENABLED 0

// =============================================================================
// Backend 2: TBB (default)
// =============================================================================
#elif defined(NWGRAPH_BACKEND_TBB)

  #include <oneapi/tbb.h>
  #include <tbb/blocked_range.h>
  #include <tbb/parallel_for.h>
  #include <tbb/parallel_for_each.h>
  #include <tbb/parallel_reduce.h>

  #define NWGRAPH_PARALLEL_BACKEND "TBB"
  #define NWGRAPH_BACKEND_HPX_ENABLED 0
  #define NWGRAPH_BACKEND_TBB_ENABLED 1

// =============================================================================
// Backend 3: No parallel backend
// =============================================================================
#else

  #define NWGRAPH_PARALLEL_BACKEND "sequential"
  #define NWGRAPH_BACKEND_HPX_ENABLED 0
  #define NWGRAPH_BACKEND_TBB_ENABLED 0

#endif

namespace nw {
namespace graph {

/**
 * @brief Backend configuration and utilities.
 */
namespace backend {

/**
 * @brief Get the name of the current parallel backend.
 * @return String identifying the backend ("HPX", "TBB", or "sequential").
 */
inline constexpr const char* name() noexcept {
  return NWGRAPH_PARALLEL_BACKEND;
}

/**
 * @brief Check if TBB backend is enabled.
 */
inline constexpr bool is_tbb() noexcept {
  return NWGRAPH_BACKEND_TBB_ENABLED;
}

/**
 * @brief Check if HPX backend is enabled.
 */
inline constexpr bool is_hpx() noexcept {
  return NWGRAPH_BACKEND_HPX_ENABLED;
}

/**
 * @brief Check if any parallel backend is enabled.
 */
inline constexpr bool is_parallel() noexcept {
  return NWGRAPH_BACKEND_HPX_ENABLED || NWGRAPH_BACKEND_TBB_ENABLED;
}

// =============================================================================
// HPX runtime management
// =============================================================================
#if defined(NWGRAPH_BACKEND_HPX)

namespace detail {

/**
 * @brief HPX runtime state manager for lazy initialization.
 *
 * Implements the lazy initialization pattern for HPX runtime.
 * The runtime is started on first use and automatically stopped
 * when the program exits.
 */
class hpx_runtime_manager {
public:
  static hpx_runtime_manager& instance() {
    static hpx_runtime_manager mgr;
    return mgr;
  }

  void set_num_threads(std::size_t n) {
    if (!initialized_.load(std::memory_order_acquire)) {
      num_threads_ = n;
    }
  }

  std::size_t get_num_threads() const noexcept {
    return num_threads_;
  }

  void ensure_initialized() {
    if (!initialized_.load(std::memory_order_acquire)) {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!initialized_.load(std::memory_order_relaxed)) {
        if (!hpx::is_running()) {
          started_by_us_ = true;

          std::vector<std::string> args_storage;
          args_storage.push_back("nwgraph");

          if (num_threads_ > 0) {
            args_storage.push_back("--hpx:threads=" + std::to_string(num_threads_));
          }

          std::vector<char*> argv;
          for (auto& s : args_storage) {
            argv.push_back(const_cast<char*>(s.c_str()));
          }
          argv.push_back(nullptr);

          int argc = static_cast<int>(argv.size() - 1);
          hpx::local::start(nullptr, argc, argv.data());
        }
        initialized_.store(true, std::memory_order_release);
      }
    }
  }

  bool is_initialized() const noexcept {
    return initialized_.load(std::memory_order_acquire);
  }

private:
  hpx_runtime_manager() = default;

  ~hpx_runtime_manager() {
    if (started_by_us_ && hpx::is_running()) {
      hpx::local::finalize();
    }
  }

  hpx_runtime_manager(const hpx_runtime_manager&) = delete;
  hpx_runtime_manager& operator=(const hpx_runtime_manager&) = delete;

  std::atomic<bool> initialized_{false};
  std::mutex mutex_;
  bool started_by_us_{false};
  std::size_t num_threads_{0};
};

} // namespace detail

inline void set_num_threads(std::size_t n) {
  detail::hpx_runtime_manager::instance().set_num_threads(n);
}

inline void ensure_initialized() {
  detail::hpx_runtime_manager::instance().ensure_initialized();
}

inline bool is_initialized() noexcept {
  return detail::hpx_runtime_manager::instance().is_initialized();
}

// =============================================================================
// TBB runtime management
// =============================================================================
#elif defined(NWGRAPH_BACKEND_TBB)

inline void set_num_threads(std::size_t /*n*/) noexcept {
  // TBB thread count is controlled via tbb::global_control
}

inline void ensure_initialized() noexcept {
  // TBB initializes automatically on first use
}

inline bool is_initialized() noexcept {
  return true;
}

// =============================================================================
// No-backend runtime management
// =============================================================================
#else

inline void set_num_threads(std::size_t /*n*/) noexcept {
  // No parallel backend — nothing to configure
}

inline void ensure_initialized() noexcept {
  // No parallel backend — nothing to initialize
}

inline bool is_initialized() noexcept {
  return true;
}

#endif

/**
 * @brief RAII guard to ensure backend is initialized.
 *
 * Use at the start of any parallel operation to ensure the runtime is ready.
 * For TBB and sequential backends this is a no-op.
 *
 * @code
 * void my_parallel_algorithm() {
 *   backend::init_guard guard;
 *   // ... parallel operations ...
 * }
 * @endcode
 */
class init_guard {
public:
  init_guard() {
    ensure_initialized();
  }
};

} // namespace backend
} // namespace graph
} // namespace nw

#endif // NW_GRAPH_BACKEND_HPP
