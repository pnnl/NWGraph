/**
 * @file backend.hpp
 * @brief Parallel backend selection and abstraction layer.
 *
 * Provides compile-time selection between TBB and HPX parallel backends.
 * When using HPX, provides lazy initialization of the HPX runtime.
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

// Backend selection: HPX or TBB (default)
// Define NWGRAPH_BACKEND_HPX to use HPX, otherwise TBB is used
#if defined(NWGRAPH_BACKEND_HPX)

  #include <hpx/local/init.hpp>
  #include <hpx/modules/algorithms.hpp>
  #include <hpx/modules/execution.hpp>
  #include <hpx/modules/runtime_local.hpp>
  #include <hpx/parallel/algorithms/for_each.hpp>
  #include <hpx/parallel/algorithms/for_loop.hpp>
  #include <hpx/parallel/algorithms/reduce.hpp>
  #include <hpx/parallel/algorithms/transform_reduce.hpp>

  #define NWGRAPH_PARALLEL_BACKEND "HPX"
  #define NWGRAPH_BACKEND_HPX_ENABLED 1
  #define NWGRAPH_BACKEND_TBB_ENABLED 0

#else
  // Default to TBB
  #ifndef NWGRAPH_BACKEND_TBB
    #define NWGRAPH_BACKEND_TBB
  #endif

  #include <oneapi/tbb.h>
  #include <tbb/blocked_range.h>
  #include <tbb/parallel_for.h>
  #include <tbb/parallel_for_each.h>
  #include <tbb/parallel_reduce.h>

  #define NWGRAPH_PARALLEL_BACKEND "TBB"
  #define NWGRAPH_BACKEND_HPX_ENABLED 0
  #define NWGRAPH_BACKEND_TBB_ENABLED 1

#endif

namespace nw {
namespace graph {

/**
 * @brief Backend configuration and utilities.
 */
namespace backend {

/**
 * @brief Get the name of the current parallel backend.
 * @return String identifying the backend ("TBB" or "HPX").
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
  /**
   * @brief Get singleton instance.
   */
  static hpx_runtime_manager& instance() {
    static hpx_runtime_manager mgr;
    return mgr;
  }

  /**
   * @brief Ensure HPX runtime is initialized.
   *
   * Thread-safe initialization using double-checked locking.
   * If HPX is already running (e.g., user called hpx::init), this is a no-op.
   */
  void ensure_initialized() {
    if (!initialized_.load(std::memory_order_acquire)) {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!initialized_.load(std::memory_order_relaxed)) {
        if (!hpx::is_running()) {
          // Start HPX local runtime (no networking)
          // Using nullptr for argc/argv starts with default settings
          started_by_us_ = true;
          hpx::local::start(nullptr, 0, nullptr);
        }
        initialized_.store(true, std::memory_order_release);
      }
    }
  }

  /**
   * @brief Check if runtime is initialized.
   */
  bool is_initialized() const noexcept {
    return initialized_.load(std::memory_order_acquire);
  }

private:
  hpx_runtime_manager() = default;

  ~hpx_runtime_manager() {
    // Only stop HPX if we started it
    if (started_by_us_ && hpx::is_running()) {
      hpx::local::finalize();
    }
  }

  // Non-copyable, non-movable
  hpx_runtime_manager(const hpx_runtime_manager&) = delete;
  hpx_runtime_manager& operator=(const hpx_runtime_manager&) = delete;

  std::atomic<bool> initialized_{false};
  std::mutex mutex_;
  bool started_by_us_{false};
};

} // namespace detail

/**
 * @brief Ensure the HPX runtime is initialized.
 *
 * Call this before using any HPX parallel operations.
 * Safe to call multiple times - initialization only happens once.
 *
 * @note If the user has already initialized HPX (via hpx::init or hpx_main),
 *       this function does nothing.
 */
inline void ensure_initialized() {
  detail::hpx_runtime_manager::instance().ensure_initialized();
}

/**
 * @brief Check if the backend runtime is initialized.
 */
inline bool is_initialized() noexcept {
  return detail::hpx_runtime_manager::instance().is_initialized();
}

#else // TBB backend

/**
 * @brief Ensure the TBB runtime is initialized.
 *
 * TBB initializes lazily by default, so this is a no-op.
 * Provided for API compatibility with HPX backend.
 */
inline void ensure_initialized() noexcept {
  // TBB initializes automatically on first use - nothing to do
}

/**
 * @brief Check if the backend runtime is initialized.
 *
 * TBB is always considered "initialized" since it auto-initializes.
 */
inline bool is_initialized() noexcept {
  return true;  // TBB auto-initializes
}

#endif // NWGRAPH_BACKEND_HPX

/**
 * @brief RAII guard to ensure backend is initialized.
 *
 * Use this at the start of any parallel operation to ensure
 * the runtime is ready.
 *
 * Example:
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
