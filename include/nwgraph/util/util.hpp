/**
 * @file util.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese	
 *
 */

#ifndef NW_GRAPH_UTIL_HPP
#define NW_GRAPH_UTIL_HPP

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <numeric>
#include <tuple>
#include <type_traits>

#include "nwgraph/util/traits.hpp"

namespace nw {
namespace graph {

template <typename T = std::size_t>

class counting_output_iterator : public std::iterator<std::output_iterator_tag, std::ptrdiff_t> {
public:
  counting_output_iterator(T& count) : count{count} {}

  counting_output_iterator& operator++() { return *this; }
  counting_output_iterator& operator++(int) { return *this; }
  counting_output_iterator& operator*() { return *this; }
  counting_output_iterator& operator[](std::ptrdiff_t) { return *this; }

  // counting_output_iterator& operator=(const counting_output_iterator&) = delete;
  // counting_output_iterator& operator=(counting_output_iterator&&) = delete;

  template <typename U>
  auto& operator=(U) {
    count++;
    return *this;
  }

  auto get_count() { return count; }

private:
  T& count;
};

/// Trivial function object to forward to std::max.
///
/// This is mainly useful in BGL as a convenience in reductions.
struct max {
  template <class U, class V>
  constexpr auto operator()(U&& lhs, V&& rhs) const {
    return std::max(std::forward<U>(lhs), std::forward<V>(rhs));
  }
};

/// Trivial function object to forward to std::min.
///
/// This is mainly useful in BGL as a convenience in reductions.
struct min {
  template <class U, class V>
  constexpr auto operator()(U&& lhs, V&& rhs) const {
    return std::min(std::forward<U>(lhs), std::forward<V>(rhs));
  }
};

/// This simple counter can be used as an output iterator.
///
/// It differs from the counting output iterator above as it provides an inline
/// value for the count, which can simplify the point of use relative to the
/// counting_output_iterator.
///
/// @tparam           T The underlying type of the counter.
template <class T = std::size_t>
struct counter : public std::iterator<std::output_iterator_tag, std::ptrdiff_t> {
  T count;

  counter(T init = {}) : count(init) {}

  constexpr operator T() const { return count; }

  counter& operator++() { return *this; }
  counter& operator++(int) { return *this; }
  counter& operator*() { return *this; }

  template <class U>
  decltype(auto) operator=(U) {
    ++count;
    return *this;
  }
};

/// Get and permute a subset of indices of a tuple (like get<0, 2, ...>(t))
///
/// @tparam       Is... The indices to select (can repeat).
/// @tparam       Tuple The tuple type from which to select.
/// @tparam          _2 Concept constraint that Tuple is a tuple.
///
/// @param            t The tuple from which to select.
///
/// @returns            A tuple composed of the proper elements from `t`.
template <std::size_t... Is, class Tuple, class = std::enable_if_t<is_tuple_v<std::decay_t<Tuple>>>>
constexpr auto select(Tuple&& t) -> std::tuple<std::tuple_element_t<Is, std::decay_t<Tuple>>...> {
  static_assert(((Is < std::tuple_size_v<std::decay_t<Tuple>>)&&...), "tuple index out of range during select");
  return {std::forward<std::tuple_element_t<Is, std::decay_t<Tuple>>>(std::get<Is>(std::forward<Tuple>(t)))...};
}

/// Meta-function to get the type of a tuple after selection (see `select`).
template <class Tuple, std::size_t... Is>
using select_t = decltype(select<Is...>(std::declval<Tuple>()));

template <typename vertex_id_type>
struct null_vertex_s {
  constexpr static vertex_id_type value = std::numeric_limits<vertex_id_type>::max();
};

template <typename vertex_id_type>
inline constexpr auto null_vertex_v() {
  return null_vertex_s<vertex_id_type>::value;
}

template <typename InputIterator, typename RandomAccessIterator,
          typename = std::enable_if_t<nw::graph::is_tuple_v<typename InputIterator::value_type>>>
void histogram(InputIterator first, InputIterator last, RandomAccessIterator o_first, RandomAccessIterator o_last, size_t idx = 0) {
  std::fill(o_first, o_last, 0);
  std::for_each(first, last, [&](auto& i) { o_first[std::get<idx>(i)]++; });
};

template <typename InputIterator, typename RandomAccessIterator>
void histogram(InputIterator first, InputIterator last, RandomAccessIterator o_first, RandomAccessIterator o_last) {
  std::fill(o_first, o_last, 0);
  std::for_each(first, last, [&](auto& i) { o_first[i]++; });
};

template <typename T>
constexpr typename std::underlying_type<T>::type idx(T value) {
  return static_cast<typename std::underlying_type<T>::type>(value);
}

template <typename OuterIter>
auto get_source(OuterIter& outer) {
  return outer.get_index();
};

template <typename InnerIter>
auto get_target(InnerIter& inner) {
  return std::get<0>(*inner);
};

template <size_t Idx, typename Iterator>
auto property(Iterator& inner) {
  return std::get<Idx>(*inner);
}

template <size_t Idx, typename Iterator>
auto property_ptr(Iterator& inner) {
  return &std::get<Idx>(*inner);
}

/// The log_2 of an integer is the inverse of pow2... essentially the number of
/// left shift bits we need to shift out of the value to get to 0.
static inline constexpr int log2(uint64_t val) {
  assert(val);
  return ((sizeof(val) * 8 - 1) - __builtin_clzl(val));
}

/// http://stackoverflow.com/questions/3272424/compute-fast-log-base-2-ceiling
static inline constexpr int ceil_log2(uint32_t val) {
  assert(val);
  return ((sizeof(val) * 8 - 1) - __builtin_clz(val)) + (!!(val & (val - 1)));
}

static inline constexpr int ceil_log2(int32_t val) {
  assert(0 < val);
  return ceil_log2(uint32_t(val));
}

/// http://stackoverflow.com/questions/3272424/compute-fast-log-base-2-ceiling
static inline constexpr int ceil_log2(uint64_t val) {
  assert(val);
  return ((sizeof(val) * 8 - 1) - __builtin_clzl(val)) + (!!(val & (val - 1)));
}

/// Raise 2^exp, when exp is an integer.
template <class T = uint64_t>
static constexpr T pow2(int exp) {
  static_assert(std::is_integral_v<T>, "pow2 only returns integer types");
  assert(0 <= exp and exp < (int)(8 * sizeof(T) - 1));
  return (T(1) << exp);
}

/// Create a blocked range pair.
///
/// Given a number N, and a number of blocks n, and an id, create a block range
/// for this id.
template <class T, class U, class V>
static constexpr std::pair<T, T> block(T N, U n, V id) {
  auto r     = N % n;                                 // remainder for block size
  auto b     = N / n;                                 // basic block size
  auto begin = id * b + std::min(id, r);              // block min
  auto end   = (id + 1) * b + std::min(id + 1, r);    // block max
  assert(id != 0 || begin == 0);                      // first range should start at 0
  assert(id != (N - 1) || end == N);                  // last range should end at N
  return std::pair(begin, end);
}
}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_UTIL_HPP
