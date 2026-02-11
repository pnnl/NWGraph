/**
 * @file execution_policy.hpp
 * @brief Execution policy compatibility layer for NWGraph parallel backends.
 *
 * NWGraph supports three parallel backend configurations:
 *
 *   1. HPX backend (NWGRAPH_BACKEND_HPX defined)
 *      - Uses hpx::execution policies and hpx:: parallel algorithms
 *      - Requires HPX runtime initialization (handled by backend::init_guard)
 *      - Provides distributed parallelism beyond shared memory
 *
 *   2. TBB backend (NWGRAPH_BACKEND_TBB defined, the default)
 *      - Uses std::execution policies backed by TBB's parallel runtime
 *      - TBB provides the work-stealing scheduler that makes std::execution::par
 *        actually parallel (with g++ / libstdc++)
 *      - Best shared-memory performance for regular workloads
 *
 *   3. No parallel backend (neither defined)
 *      - Uses std::execution policies if <execution> is available
 *      - Falls back to sequential-only wrappers otherwise
 *      - Useful for building without TBB or HPX dependencies
 *
 * All three configurations expose the same interface:
 *   - nw::graph::execution::{seq, par, par_unseq}  (policy instances)
 *   - nw::graph::par_sort, par_for_each, par_copy, etc. (algorithm wrappers)
 *
 * Known shortcomings:
 *   - HPX parallel algorithms have compatibility issues with NWGraph's custom
 *     iterators (e.g., soa_iterator). Some algorithms like par_unique may fail
 *     at compile time with HPX when used with these iterator types.
 *   - The "no backend" configuration with std::execution policies may still
 *     execute sequentially if no parallel runtime (TBB) is linked.
 *   - TBB's parallel_reduce has no std::execution equivalent; code using
 *     tbb::parallel_reduce directly must be guarded with #if NWGRAPH_BACKEND_TBB_ENABLED.
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

#ifndef NW_GRAPH_EXECUTION_POLICY_HPP
#define NW_GRAPH_EXECUTION_POLICY_HPP

#include <algorithm>
#include <numeric>

// =============================================================================
// Backend 1: HPX
// =============================================================================
#if defined(NWGRAPH_BACKEND_HPX)

  #include <hpx/execution.hpp>
  #include <hpx/algorithm.hpp>
  #include <hpx/numeric.hpp>
  #include "nwgraph/util/backend.hpp"

  namespace nw {
  namespace graph {
  namespace execution {
    using sequenced_policy              = hpx::execution::sequenced_policy;
    using parallel_policy               = hpx::execution::parallel_policy;
    using parallel_unsequenced_policy   = hpx::execution::parallel_unsequenced_policy;

    inline constexpr auto seq       = hpx::execution::seq;
    inline constexpr auto par       = hpx::execution::par;
    inline constexpr auto par_unseq = hpx::execution::par_unseq;
  }  // namespace execution

  using default_execution_policy = execution::parallel_unsequenced_policy;

  // --- Algorithm wrappers (HPX) ---
  // Each ensures the HPX runtime is initialized via backend::init_guard.

  template <typename ExPolicy, typename Iter, typename Comp>
  void par_sort(ExPolicy&& policy, Iter first, Iter last, Comp comp) {
    backend::init_guard guard;
    hpx::sort(std::forward<ExPolicy>(policy), first, last, comp);
  }

  template <typename ExPolicy, typename Iter>
  void par_sort(ExPolicy&& policy, Iter first, Iter last) {
    backend::init_guard guard;
    hpx::sort(std::forward<ExPolicy>(policy), first, last);
  }

  template <typename ExPolicy, typename Iter, typename Comp>
  void par_stable_sort(ExPolicy&& policy, Iter first, Iter last, Comp comp) {
    backend::init_guard guard;
    hpx::stable_sort(std::forward<ExPolicy>(policy), first, last, comp);
  }

  template <typename ExPolicy, typename InIter, typename OutIter>
  OutIter par_copy(ExPolicy&& policy, InIter first, InIter last, OutIter dest) {
    backend::init_guard guard;
    return hpx::copy(std::forward<ExPolicy>(policy), first, last, dest);
  }

  template <typename ExPolicy, typename Iter, typename Func>
  void par_for_each(ExPolicy&& policy, Iter first, Iter last, Func func) {
    backend::init_guard guard;
    hpx::for_each(std::forward<ExPolicy>(policy), first, last, func);
  }

  template <typename ExPolicy, typename Iter, typename Pred>
  Iter par_unique(ExPolicy&& policy, Iter first, Iter last, Pred pred) {
    backend::init_guard guard;
    return hpx::unique(std::forward<ExPolicy>(policy), first, last, pred);
  }

  template <typename ExPolicy, typename InIter, typename OutIter>
  OutIter par_inclusive_scan(ExPolicy&& policy, InIter first, InIter last, OutIter dest) {
    backend::init_guard guard;
    return hpx::inclusive_scan(std::forward<ExPolicy>(policy), first, last, dest);
  }

  template <typename ExPolicy, typename Iter1, typename Iter2>
  bool par_equal(ExPolicy&& policy, Iter1 first1, Iter1 last1, Iter2 first2) {
    backend::init_guard guard;
    return hpx::equal(std::forward<ExPolicy>(policy), first1, last1, first2);
  }

  template <typename ExPolicy, typename Iter, typename T>
  void par_fill(ExPolicy&& policy, Iter first, Iter last, const T& value) {
    backend::init_guard guard;
    hpx::fill(std::forward<ExPolicy>(policy), first, last, value);
  }

  template <typename ExPolicy, typename InIter, typename OutIter, typename UnaryOp>
  OutIter par_transform(ExPolicy&& policy, InIter first, InIter last, OutIter dest, UnaryOp op) {
    backend::init_guard guard;
    return hpx::transform(std::forward<ExPolicy>(policy), first, last, dest, op);
  }

  }  // namespace graph
  }  // namespace nw

// =============================================================================
// Backend 2: TBB (default) — std::execution backed by TBB runtime
// =============================================================================
#elif defined(NWGRAPH_BACKEND_TBB)

  #include <execution>

  namespace nw {
  namespace graph {
  namespace execution {
    using sequenced_policy              = std::execution::sequenced_policy;
    using parallel_policy               = std::execution::parallel_policy;
    using parallel_unsequenced_policy   = std::execution::parallel_unsequenced_policy;

    inline constexpr auto seq       = std::execution::seq;
    inline constexpr auto par       = std::execution::par;
    inline constexpr auto par_unseq = std::execution::par_unseq;
  }  // namespace execution

  using default_execution_policy = execution::parallel_unsequenced_policy;

  // --- Algorithm wrappers (TBB / std::execution) ---

  template <typename ExPolicy, typename Iter, typename Comp>
  void par_sort(ExPolicy&& policy, Iter first, Iter last, Comp comp) {
    std::sort(std::forward<ExPolicy>(policy), first, last, comp);
  }

  template <typename ExPolicy, typename Iter>
  void par_sort(ExPolicy&& policy, Iter first, Iter last) {
    std::sort(std::forward<ExPolicy>(policy), first, last);
  }

  template <typename ExPolicy, typename Iter, typename Comp>
  void par_stable_sort(ExPolicy&& policy, Iter first, Iter last, Comp comp) {
    std::stable_sort(std::forward<ExPolicy>(policy), first, last, comp);
  }

  template <typename ExPolicy, typename InIter, typename OutIter>
  OutIter par_copy(ExPolicy&& policy, InIter first, InIter last, OutIter dest) {
    return std::copy(std::forward<ExPolicy>(policy), first, last, dest);
  }

  template <typename ExPolicy, typename Iter, typename Func>
  void par_for_each(ExPolicy&& policy, Iter first, Iter last, Func func) {
    std::for_each(std::forward<ExPolicy>(policy), first, last, func);
  }

  template <typename ExPolicy, typename Iter, typename Pred>
  Iter par_unique(ExPolicy&& policy, Iter first, Iter last, Pred pred) {
    return std::unique(std::forward<ExPolicy>(policy), first, last, pred);
  }

  template <typename ExPolicy, typename InIter, typename OutIter>
  OutIter par_inclusive_scan(ExPolicy&& policy, InIter first, InIter last, OutIter dest) {
    return std::inclusive_scan(std::forward<ExPolicy>(policy), first, last, dest);
  }

  template <typename ExPolicy, typename Iter1, typename Iter2>
  bool par_equal(ExPolicy&& policy, Iter1 first1, Iter1 last1, Iter2 first2) {
    return std::equal(std::forward<ExPolicy>(policy), first1, last1, first2);
  }

  template <typename ExPolicy, typename Iter, typename T>
  void par_fill(ExPolicy&& policy, Iter first, Iter last, const T& value) {
    std::fill(std::forward<ExPolicy>(policy), first, last, value);
  }

  template <typename ExPolicy, typename InIter, typename OutIter, typename UnaryOp>
  OutIter par_transform(ExPolicy&& policy, InIter first, InIter last, OutIter dest, UnaryOp op) {
    return std::transform(std::forward<ExPolicy>(policy), first, last, dest, op);
  }

  }  // namespace graph
  }  // namespace nw

// =============================================================================
// Backend 3: No parallel backend — sequential fallback
// =============================================================================
#else

  // No TBB, no HPX. Provide sequential-only execution.
  // std::execution may or may not be available depending on the compiler.

  namespace nw {
  namespace graph {
  namespace execution {

    // Define our own minimal policy types for the sequential-only case.
    struct sequenced_policy {};
    struct parallel_policy {};
    struct parallel_unsequenced_policy {};

    inline constexpr sequenced_policy              seq{};
    inline constexpr parallel_policy               par{};        // will execute sequentially
    inline constexpr parallel_unsequenced_policy   par_unseq{};  // will execute sequentially

  }  // namespace execution

  using default_execution_policy = execution::sequenced_policy;

  // --- Algorithm wrappers (sequential fallback) ---
  // All policies are accepted but execution is always sequential.

  template <typename ExPolicy, typename Iter, typename Comp>
  void par_sort(ExPolicy&&, Iter first, Iter last, Comp comp) {
    std::sort(first, last, comp);
  }

  template <typename ExPolicy, typename Iter>
  void par_sort(ExPolicy&&, Iter first, Iter last) {
    std::sort(first, last);
  }

  template <typename ExPolicy, typename Iter, typename Comp>
  void par_stable_sort(ExPolicy&&, Iter first, Iter last, Comp comp) {
    std::stable_sort(first, last, comp);
  }

  template <typename ExPolicy, typename InIter, typename OutIter>
  OutIter par_copy(ExPolicy&&, InIter first, InIter last, OutIter dest) {
    return std::copy(first, last, dest);
  }

  template <typename ExPolicy, typename Iter, typename Func>
  void par_for_each(ExPolicy&&, Iter first, Iter last, Func func) {
    std::for_each(first, last, func);
  }

  template <typename ExPolicy, typename Iter, typename Pred>
  Iter par_unique(ExPolicy&&, Iter first, Iter last, Pred pred) {
    return std::unique(first, last, pred);
  }

  template <typename ExPolicy, typename InIter, typename OutIter>
  OutIter par_inclusive_scan(ExPolicy&&, InIter first, InIter last, OutIter dest) {
    return std::inclusive_scan(first, last, dest);
  }

  template <typename ExPolicy, typename Iter1, typename Iter2>
  bool par_equal(ExPolicy&&, Iter1 first1, Iter1 last1, Iter2 first2) {
    return std::equal(first1, last1, first2);
  }

  template <typename ExPolicy, typename Iter, typename T>
  void par_fill(ExPolicy&&, Iter first, Iter last, const T& value) {
    std::fill(first, last, value);
  }

  template <typename ExPolicy, typename InIter, typename OutIter, typename UnaryOp>
  OutIter par_transform(ExPolicy&&, InIter first, InIter last, OutIter dest, UnaryOp op) {
    return std::transform(first, last, dest, op);
  }

  }  // namespace graph
  }  // namespace nw

#endif

#endif  // NW_GRAPH_EXECUTION_POLICY_HPP
