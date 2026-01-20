/**
 * @file concurrent_containers.hpp
 * @brief Backend-agnostic concurrent container abstractions.
 *
 * Provides concurrent queue, vector, and mutex abstractions that work
 * with both TBB and HPX backends. These containers are used internally
 * by parallel graph algorithms.
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

#ifndef NW_GRAPH_CONCURRENT_CONTAINERS_HPP
#define NW_GRAPH_CONCURRENT_CONTAINERS_HPP

#include "nwgraph/util/backend.hpp"

#include <atomic>
#include <cstddef>
#include <mutex>
#include <vector>

#if defined(NWGRAPH_BACKEND_HPX)
  #include <hpx/synchronization/mutex.hpp>
  #include <hpx/synchronization/spinlock.hpp>
  // HPX doesn't have concurrent_vector, we'll use a mutex-protected vector
#else
  #include <tbb/concurrent_queue.h>
  #include <tbb/concurrent_vector.h>
  #include <tbb/queuing_mutex.h>
#endif

namespace nw {
namespace graph {

/**
 * @brief Thread-safe queue for concurrent access.
 *
 * Provides a FIFO queue that can be safely accessed from multiple threads.
 * Uses TBB's concurrent_queue or a mutex-protected std::queue for HPX.
 *
 * @tparam T The element type.
 */
template <typename T>
class concurrent_queue {
public:
  using value_type = T;

  concurrent_queue() = default;

  /**
   * @brief Push an element to the back of the queue.
   * @param item The item to push.
   */
  void push(const T& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    queue_.push(item);
#else
    queue_.push(item);
#endif
  }

  /**
   * @brief Push an element to the back of the queue (move version).
   * @param item The item to push.
   */
  void push(T&& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    queue_.push(std::move(item));
#else
    queue_.push(std::move(item));
#endif
  }

  /**
   * @brief Try to pop an element from the front of the queue.
   * @param item Output parameter for the popped item.
   * @return true if an item was popped, false if queue was empty.
   */
  bool try_pop(T& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    if (queue_.empty()) {
      return false;
    }
    item = std::move(queue_.front());
    queue_.pop();
    return true;
#else
    return queue_.try_pop(item);
#endif
  }

  /**
   * @brief Check if the queue is empty.
   * @return true if empty.
   */
  bool empty() const {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    return queue_.empty();
#else
    return queue_.empty();
#endif
  }

  /**
   * @brief Clear all elements from the queue.
   */
  void clear() {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    std::queue<T> empty;
    std::swap(queue_, empty);
#else
    queue_.clear();
#endif
  }

private:
#if defined(NWGRAPH_BACKEND_HPX)
  std::queue<T> queue_;
  mutable hpx::spinlock mutex_;
#else
  tbb::concurrent_queue<T> queue_;
#endif
};

/**
 * @brief Thread-safe vector for concurrent access.
 *
 * Provides a growable array that can be safely modified from multiple threads.
 * Uses TBB's concurrent_vector or a mutex-protected std::vector for HPX.
 *
 * @note For HPX backend, concurrent push_back operations are thread-safe,
 *       but indexed access ([]) is not synchronized - caller must ensure
 *       thread safety for indexed access.
 *
 * @tparam T The element type.
 */
template <typename T>
class concurrent_vector {
public:
  using value_type = T;
  using size_type = std::size_t;
  using reference = T&;
  using const_reference = const T&;

#if defined(NWGRAPH_BACKEND_HPX)
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
#else
  using iterator = typename tbb::concurrent_vector<T>::iterator;
  using const_iterator = typename tbb::concurrent_vector<T>::const_iterator;
#endif

  concurrent_vector() = default;

  /**
   * @brief Construct with initial size.
   * @param n Initial number of elements.
   */
  explicit concurrent_vector(size_type n) : data_(n) {}

  /**
   * @brief Construct with initial size and value.
   * @param n Initial number of elements.
   * @param val Value to initialize elements with.
   */
  concurrent_vector(size_type n, const T& val) : data_(n, val) {}

  /**
   * @brief Thread-safe push_back.
   * @param item The item to append.
   */
  void push_back(const T& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    data_.push_back(item);
#else
    data_.push_back(item);
#endif
  }

  /**
   * @brief Thread-safe push_back (move version).
   * @param item The item to append.
   */
  void push_back(T&& item) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    data_.push_back(std::move(item));
#else
    data_.push_back(std::move(item));
#endif
  }

  /**
   * @brief Get the number of elements.
   * @return Number of elements.
   */
  size_type size() const {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    return data_.size();
#else
    return data_.size();
#endif
  }

  /**
   * @brief Check if empty.
   * @return true if no elements.
   */
  bool empty() const {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    return data_.empty();
#else
    return data_.empty();
#endif
  }

  /**
   * @brief Clear all elements.
   */
  void clear() {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    data_.clear();
#else
    data_.clear();
#endif
  }

  /**
   * @brief Resize the vector.
   * @param n New size.
   */
  void resize(size_type n) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    data_.resize(n);
#else
    data_.resize(n);
#endif
  }

  /**
   * @brief Resize with value.
   * @param n New size.
   * @param val Value for new elements.
   */
  void resize(size_type n, const T& val) {
#if defined(NWGRAPH_BACKEND_HPX)
    std::lock_guard<hpx::spinlock> lock(mutex_);
    data_.resize(n, val);
#else
    data_.resize(n, val);
#endif
  }

  /**
   * @brief Access element by index (not thread-safe for writes).
   * @param i Index.
   * @return Reference to element.
   */
  reference operator[](size_type i) {
    return data_[i];
  }

  /**
   * @brief Access element by index (const version).
   * @param i Index.
   * @return Const reference to element.
   */
  const_reference operator[](size_type i) const {
    return data_[i];
  }

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }
  const_iterator begin() const { return data_.begin(); }
  const_iterator end() const { return data_.end(); }
  const_iterator cbegin() const { return data_.cbegin(); }
  const_iterator cend() const { return data_.cend(); }

  /**
   * @brief Get pointer to underlying data.
   * @return Pointer to first element.
   */
  T* data() { return data_.data(); }
  const T* data() const { return data_.data(); }

private:
#if defined(NWGRAPH_BACKEND_HPX)
  std::vector<T> data_;
  mutable hpx::spinlock mutex_;
#else
  tbb::concurrent_vector<T> data_;
#endif
};

/**
 * @brief Mutex with queuing semantics for fair scheduling.
 *
 * Uses TBB's queuing_mutex or HPX's mutex with similar semantics.
 */
class queuing_mutex {
public:
  queuing_mutex() = default;

  // Non-copyable
  queuing_mutex(const queuing_mutex&) = delete;
  queuing_mutex& operator=(const queuing_mutex&) = delete;

  /**
   * @brief RAII-style scoped lock.
   */
  class scoped_lock {
  public:
    /**
     * @brief Acquire lock on construction.
     * @param m The mutex to lock.
     */
    explicit scoped_lock(queuing_mutex& m)
#if defined(NWGRAPH_BACKEND_HPX)
      : lock_(m.mutex_)
#else
      : lock_(m.mutex_)
#endif
    {}

    // Non-copyable
    scoped_lock(const scoped_lock&) = delete;
    scoped_lock& operator=(const scoped_lock&) = delete;

  private:
#if defined(NWGRAPH_BACKEND_HPX)
    std::unique_lock<hpx::mutex> lock_;
#else
    tbb::queuing_mutex::scoped_lock lock_;
#endif
  };

private:
#if defined(NWGRAPH_BACKEND_HPX)
  hpx::mutex mutex_;
#else
  tbb::queuing_mutex mutex_;
#endif
};

/**
 * @brief Simple spinlock for lightweight synchronization.
 */
class spinlock {
public:
  spinlock() = default;

  // Non-copyable
  spinlock(const spinlock&) = delete;
  spinlock& operator=(const spinlock&) = delete;

  void lock() {
#if defined(NWGRAPH_BACKEND_HPX)
    lock_.lock();
#else
    while (flag_.test_and_set(std::memory_order_acquire)) {
      // Spin
    }
#endif
  }

  void unlock() {
#if defined(NWGRAPH_BACKEND_HPX)
    lock_.unlock();
#else
    flag_.clear(std::memory_order_release);
#endif
  }

  bool try_lock() {
#if defined(NWGRAPH_BACKEND_HPX)
    return lock_.try_lock();
#else
    return !flag_.test_and_set(std::memory_order_acquire);
#endif
  }

private:
#if defined(NWGRAPH_BACKEND_HPX)
  hpx::spinlock lock_;
#else
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
#endif
};

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_CONCURRENT_CONTAINERS_HPP
