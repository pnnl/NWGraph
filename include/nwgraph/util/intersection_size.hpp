/**
 * @file intersection_size.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *
 */

#ifndef NW_GRAPH_INTERSECTION_SIZE_HPP
#define NW_GRAPH_INTERSECTION_SIZE_HPP

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#include <dpstd/numeric>
#else
#include <algorithm>
#include <execution>
#include <numeric>
#endif
#include <type_traits>

namespace nw {
namespace graph {
/// Basic helper used for all of the inner set intersections.
///
/// This wraps `std::set_intersection` to produce the size of the set rather
/// than the set itself, and also handles the fact that our iterator value types
/// are tuples where we only care about the first element for ordering.
///
/// @tparam           A The type of the first iterator.
/// @tparam           B The type of the second iterator.
/// @tparam           C The type of the third iterator.
/// @tparam           D The type of the fourth iterator.
/// @tparam ExecutionPolicy The type of the parallel execution policy.
///
/// @param            i The beginning of the first range.
/// @param           ie The end of the first range.
/// @param            j The beginning of the second range.
/// @param           je The end of the second range.
/// @param           ep The parallel execution policy.
///
/// @returns            The size of the intersected set.
template <class A, class B, class C, class D, class ExecutionPolicy>
std::size_t intersection_size(A i, B&& ie, C j, D&& je, ExecutionPolicy&& ep) {
  // Custom comparator because we know our iterator operator* produces tuples
  // and we only care about the first value.
  static constexpr auto lt = [](auto&& x, auto&& y) { return std::get<0>(x) < std::get<0>(y); };

  // Use our own trivial loop for the intersection size when the execution
  // policy is sequential, otherwise rely on std::set_intersection.
  //
  // @todo We really don't need set intersection. You'd hope that it would be
  //       efficient with the output counter, but it just isn't. Parallelizing
  //       the intersection size seems non-trivial though.
  if constexpr (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
    std::size_t n = 0;
    while (i != ie && j != je) {
      if (lt(*i, *j)) {
        ++i;
      } else if (lt(*j, *i)) {
        ++j;
      } else {
        ++n;
        ++i;
        ++j;
      }
    }
    return n;
    (void)ep;
  } else {
    return std::set_intersection(std::forward<ExecutionPolicy>(ep), std::forward<A>(i), std::forward<B>(ie), std::forward<C>(j),
                                 std::forward<D>(je), nw::graph::counter{}, lt);
  }
}

/// A convenience overload for `intersection_size`.
///
/// This overload takes two ranges and an execution policy, and forwards to the
/// base `intersection_size` implementation.
///
/// @tparam           R The type of the first range.
/// @tparam           S The type of the second range.
/// @tparam ExecutionPolicy The type of the parallel execution policy.
/// @tparam          _0 SFINAE to disambiguate from other 3 argument versions.
///
/// @param            i The first range.
/// @param            j The second range.
/// @param           ep The parallel execution policy.
///
/// @returns            The size of the intersected set.
template <class R, class S, class ExecutionPolicy,
          std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>, void**> = nullptr>
std::size_t intersection_size(R&& i, S&& j, ExecutionPolicy&& ep) {
  return intersection_size(i.begin(), i.end(), j.begin(), j.end(), std::forward<ExecutionPolicy>(ep));
}

/// A convenience overload for `intersection_size`.
///
/// This overload takes two iterators defining the first range, and a second
/// range, and forwards to the base `intersection_size` implementation.
///
/// @tparam           A The type of the first iterator.
/// @tparam           B The type of the second iterator.
/// @tparam       Range The type of the second range.
/// @tparam ExecutionPolicy The type of the parallel execution policy.
/// @tparam          _0 SFINAE to disambiguate from other 4 argument versions.
///
/// @param            i The beginning of the first range.
/// @param           ie The end of the first range.
/// @param            j The second range.
/// @param           ep The parallel execution policy.
///
/// @returns            The size of the intersected set.
template <class A, class B, class Range, class ExecutionPolicy,
          std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>, void**> = nullptr>
std::size_t intersection_size(A&& i, B&& ie, Range&& j, ExecutionPolicy&& ep) {
  return intersection_size(std::forward<A>(i), std::forward<B>(ie), j.begin(), j.end(), std::forward<ExecutionPolicy>(ep));
}

/// A convenience overload for `intersection_size`.
///
/// This overload takes two ranges as begin/end iterator pairs, and forwards to
/// the base `intersection_size` with a sequential execution policy.
///
/// @tparam           A The type of the first iterator.
/// @tparam           B The type of the second iterator.
/// @tparam           C The type of the third iterator.
/// @tparam           D The type of the fourth iterator.
/// @tparam          _0 SFINAE to disambiguate from other 4 argument versions.
///
/// @param            i The beginning of the first range.
/// @param           ie The end of the first range.
/// @param            j The beginning of the second range.
/// @param           je The end of the second range.
///
/// @returns            The size of the intersected set.
template <class A, class B, class C, class D, std::enable_if_t<!std::is_execution_policy_v<std::decay_t<D>>, void**> = nullptr>
std::size_t intersection_size(A&& i, B&& ie, C&& j, D&& je) {
  return intersection_size(std::forward<A>(i), std::forward<B>(ie), std::forward<C>(j), std::forward<D>(je), std::execution::seq);
}

/// A convenience overload for `intersection_size`.
///
/// This overload takes two ranges and and forwards to the base
/// `intersection_size` implementation with a sequential execution policy.
///
/// @tparam           R The type of the first range.
/// @tparam           S The type of the second range.
///
/// @param            i The first range.
/// @param            j The second range.
///
/// @returns            The size of the intersected set.
template <class R, class S>
std::size_t intersection_size(R&& i, S&& j) {
  return intersection_size(i.begin(), i.end(), j.begin(), j.end(), std::execution::seq);
}

/// A convenience overload for `intersection_size`.
///
/// This overload takes two iterators defining the first range, and a second
/// range, and forwards to the base `intersection_size` implementation with a
/// sequential execution policy.
///
/// @tparam           A The type of the first iterator.
/// @tparam           B The type of the second iterator.
/// @tparam       Range The type of the second range.
/// @tparam          _0 SFINAE to disambiguate from other 3 argument versions.
///
/// @param            i The beginning of the first range.
/// @param           ie The end of the first range.
/// @param            j The second range.
///
/// @returns            The size of the intersected set.
template <class A, class B, class Range, std::enable_if_t<!std::is_execution_policy_v<std::decay_t<Range>>, void**> = nullptr>
std::size_t intersection_size(A&& i, B&& ie, Range&& j) {
  return intersection_size(std::forward<A>(i), std::forward<B>(ie), j.begin(), j.end(), std::execution::seq);
}
}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_INTERSECTION_SIZE_HPP
