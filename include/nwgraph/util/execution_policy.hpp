/**
 * @file execution_policy.hpp
 * @brief Execution policy compatibility layer for HPX and standard library.
 *
 * Provides a unified interface for parallel execution policies that works
 * with both HPX and TBB/standard library backends. When HPX is enabled,
 * maps standard execution policies to HPX equivalents.
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

#if defined(NWGRAPH_BACKEND_HPX)
  // HPX provides its own execution policies in hpx::execution namespace
  #include <hpx/execution.hpp>
  #include <hpx/algorithm.hpp>
  #include <hpx/numeric.hpp>
  #include "nwgraph/util/backend.hpp"

  namespace nw {
  namespace graph {
  namespace execution {
    // Map to HPX execution policies
    using sequenced_policy = hpx::execution::sequenced_policy;
    using parallel_policy = hpx::execution::parallel_policy;
    using parallel_unsequenced_policy = hpx::execution::parallel_unsequenced_policy;

    // Execution policy instances
    inline constexpr auto seq = hpx::execution::seq;
    inline constexpr auto par = hpx::execution::par;
    inline constexpr auto par_unseq = hpx::execution::par_unseq;
  }  // namespace execution

  // Default execution policy for NWGraph algorithms
  using default_execution_policy = execution::parallel_unsequenced_policy;

  // Wrapper functions for parallel algorithms using HPX
  // Each function ensures HPX runtime is initialized via backend::init_guard
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

  template <typename ExPolicy, typename InIter, typename OutIter, typename Pred>
  OutIter par_unique(ExPolicy&& policy, InIter first, InIter last, Pred pred) {
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

#else
  // TBB/standard library backend (requires compiler with std::execution support, e.g., g++)
  #include <execution>

  namespace nw {
  namespace graph {
  namespace execution {
    // Map to standard execution policies
    using sequenced_policy = std::execution::sequenced_policy;
    using parallel_policy = std::execution::parallel_policy;
    using parallel_unsequenced_policy = std::execution::parallel_unsequenced_policy;

    // Execution policy instances
    inline constexpr auto seq = std::execution::seq;
    inline constexpr auto par = std::execution::par;
    inline constexpr auto par_unseq = std::execution::par_unseq;
  }  // namespace execution

  // Default execution policy for NWGraph algorithms
  using default_execution_policy = execution::parallel_unsequenced_policy;

  // Wrapper functions for parallel algorithms using standard library
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

  template <typename ExPolicy, typename InIter, typename OutIter, typename Pred>
  OutIter par_unique(ExPolicy&& policy, InIter first, InIter last, Pred pred) {
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

#endif

#endif  // NW_GRAPH_EXECUTION_POLICY_HPP
