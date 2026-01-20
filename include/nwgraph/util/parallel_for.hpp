/**
 * @file parallel_for.hpp
 * @brief Backend-agnostic parallel for and reduce operations.
 *
 * Provides parallel_for and parallel_reduce operations that work with
 * both TBB and HPX backends. The backend is selected at compile time
 * via the NWGRAPH_BACKEND_HPX or NWGRAPH_BACKEND_TBB macros.
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_PARALLEL_FOR_HPP
#define NW_GRAPH_PARALLEL_FOR_HPP

#include "nwgraph/util/backend.hpp"
#include "nwgraph/util/traits.hpp"

#if defined(NWGRAPH_BACKEND_HPX)
  #include <hpx/parallel/algorithms/for_each.hpp>
  #include <hpx/parallel/algorithms/for_loop.hpp>
  #include <hpx/parallel/algorithms/transform_reduce.hpp>
  #include <hpx/include/util.hpp>
#else
  #include <tbb/parallel_for.h>
  #include <tbb/parallel_reduce.h>
#endif

namespace nw {
namespace graph {

/**
 * Inner evaluation function for parallel_for.
 *
 * @tparam Op The type of the operator.
 * @tparam It The type of the iterator.
 * @param op The operator to evaluate.
 * @param i The iterator to evaluate.
 * @return op(i) if `i` is an integral type,
 *         op(unpack(i)...) if `i` is a tuple type,
 *         parallel_for_inner(*i) otherwise.
 */
template <class Op, class It>
auto parallel_for_inner(Op&& op, It&& i) {
  if constexpr (is_tuple_v<std::decay_t<It>>) {
    return std::apply([&](auto&&... args) { return std::forward<Op>(op)(std::forward<decltype(args)>(args)...); }, std::forward<It>(i));
  } else if constexpr (std::is_integral_v<std::decay_t<It>>) {
    return std::forward<Op>(op)(std::forward<It>(i));
  } else {
    if constexpr (is_tuple_v<decltype(*std::forward<It>(i))>) {
      return parallel_for_inner(std::forward<Op>(op), *std::forward<It>(i));
    } else {
      return std::forward<Op>(op)(*std::forward<It>(i));
    }
  }
}

/**
 * Apply an operator to a range sequentially.
 *
 * @tparam Range The type of the range.
 * @tparam Op The type of the operator.
 * @param range The range to process.
 * @param op The operator to evaluate.
 */
template <class Range, class Op>
void parallel_for_sequential(Range&& range, Op&& op) {
  for (auto &&i = range.begin(), e = range.end(); i != e; ++i) {
    parallel_for_inner(op, i);
  }
}

/**
 * Apply an operator to a range sequentially, reducing the result.
 *
 * @tparam Range The type of the range.
 * @tparam Op The type of the operator.
 * @tparam Reduce The type of the reduction.
 * @tparam T The reduced type.
 * @param range The range to process.
 * @param op The operator to evaluate.
 * @param reduce The reduction.
 * @param init The initial value for the reduction.
 * @return The result of `reduce(op(i), ...)` for all `i` in `range`.
 */
template <class Range, class Op, class Reduce, class T>
auto parallel_for_sequential(Range&& range, Op&& op, Reduce&& reduce, T init) {
  for (auto &&i = range.begin(), e = range.end(); i != e; ++i) {
    init = reduce(init, parallel_for_inner(op, i));
  }
  return init;
}

/**
 * NWGraph's parallel_for wrapper.
 *
 * This performs a dynamic check on `range.is_divisible()` before calling into
 * the parallel library, which we have found to be important for
 * performance. If not `range.is_divisible()` then it will perform a
 * synchronous for loop.
 *
 * @tparam Range The type of the range.
 * @tparam Op The type of the operator.
 * @param range The range to process.
 * @param op The operator to evaluate.
 */
template <class Range, class Op>
void parallel_for(Range&& range, Op&& op) {
  backend::init_guard guard;  // Ensure runtime is initialized (HPX needs this)

  if (range.is_divisible()) {
#if defined(NWGRAPH_BACKEND_HPX)
    // HPX uses for_each on the range
    hpx::for_each(hpx::execution::par, range.begin(), range.end(),
                  [&](auto&& elem) { parallel_for_inner(op, elem); });
#else
    // TBB uses parallel_for with splittable ranges
    tbb::parallel_for(std::forward<Range>(range),
                      [&](auto&& sub) { parallel_for_sequential(std::forward<decltype(sub)>(sub), std::forward<Op>(op)); });
#endif
  } else {
    parallel_for_sequential(std::forward<Range>(range), std::forward<Op>(op));
  }
}

/**
 * NWGraph's parallel_for wrapper that supports reductions.
 *
 * This performs the designated reduction over the range. It will perform the
 * reduction in parallel if `range.is_divisible()`, otherwise it will perform a
 * synchronous sequential reduction.
 *
 * @tparam Range The type of the range.
 * @tparam Op The type of the operator.
 * @tparam Reduce The type of the reduction.
 * @tparam T The reduced type.
 * @param range The range to process.
 * @param op The operator to evaluate.
 * @param reduce The reduction.
 * @param init The initial value for the reduction.
 * @return The result of `reduce(op(i), ...)` for all `i` in `range`.
 */
template <class Range, class Op, class Reduce, class T>
auto parallel_reduce(Range&& range, Op&& op, Reduce&& reduce, T init) {
  backend::init_guard guard;  // Ensure runtime is initialized (HPX needs this)

  if (range.is_divisible()) {
#if defined(NWGRAPH_BACKEND_HPX)
    // HPX uses transform_reduce
    return hpx::transform_reduce(hpx::execution::par,
                                  range.begin(), range.end(),
                                  init,
                                  std::forward<Reduce>(reduce),
                                  [&](auto&& elem) { return parallel_for_inner(op, elem); });
#else
    // TBB uses parallel_reduce with splittable ranges
    return tbb::parallel_reduce(
        std::forward<Range>(range), init,
        [&](auto&& sub, auto partial) { return parallel_for_sequential(std::forward<decltype(sub)>(sub), op, reduce, partial); }, reduce);
#endif
  } else {
    return parallel_for_sequential(std::forward<Range>(range), std::forward<Op>(op), std::forward<Reduce>(reduce), init);
  }
}

/**
 * @brief Index-based parallel for loop.
 *
 * Executes op(i) for each i in [begin, end) in parallel.
 *
 * @tparam Op The type of the operator (must be callable with std::size_t).
 * @param begin The start index (inclusive).
 * @param end The end index (exclusive).
 * @param op The operator to evaluate for each index.
 */
template <class Op>
void parallel_for_each(std::size_t begin, std::size_t end, Op&& op) {
  backend::init_guard guard;  // Ensure runtime is initialized

#if defined(NWGRAPH_BACKEND_HPX)
  hpx::for_loop(hpx::execution::par, begin, end, std::forward<Op>(op));
#else
  tbb::parallel_for(tbb::blocked_range<std::size_t>(begin, end),
                    [&](const auto& r) {
                      for (auto i = r.begin(); i != r.end(); ++i) {
                        op(i);
                      }
                    });
#endif
}

/**
 * @brief Index-based parallel reduce.
 *
 * Computes reduce(op(begin), op(begin+1), ..., op(end-1)) in parallel.
 *
 * @tparam T The type of the result.
 * @tparam Op The type of the operator (must be callable with std::size_t).
 * @tparam Reduce The type of the reduction operator.
 * @param begin The start index (inclusive).
 * @param end The end index (exclusive).
 * @param init The initial value for the reduction.
 * @param op The operator to evaluate for each index.
 * @param reduce The reduction operator.
 * @return The reduced result.
 */
template <class T, class Op, class Reduce>
T parallel_reduce_each(std::size_t begin, std::size_t end, T init, Op&& op, Reduce&& reduce) {
  backend::init_guard guard;  // Ensure runtime is initialized

#if defined(NWGRAPH_BACKEND_HPX)
  return hpx::transform_reduce(hpx::execution::par,
                                hpx::util::counting_iterator<std::size_t>(begin),
                                hpx::util::counting_iterator<std::size_t>(end),
                                init,
                                std::forward<Reduce>(reduce),
                                std::forward<Op>(op));
#else
  return tbb::parallel_reduce(
      tbb::blocked_range<std::size_t>(begin, end), init,
      [&](const auto& r, auto partial) {
        for (auto i = r.begin(); i != r.end(); ++i) {
          partial = reduce(partial, op(i));
        }
        return partial;
      },
      reduce);
#endif
}

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_PARALLEL_FOR_HPP
