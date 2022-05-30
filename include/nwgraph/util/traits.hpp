/**
 * @file traits.hpp
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

#ifndef NW_GRAPH_TRAITS_HPP
#define NW_GRAPH_TRAITS_HPP

#include <atomic>
#include <iterator>
#include <tbb/blocked_range.h>
#include <tuple>

namespace nw {
namespace graph {

template <class>
inline constexpr bool is_atomic_v = false;
template <class T>
inline constexpr bool is_atomic_v<std::atomic<T>> = true;
template <class T>
inline constexpr bool is_atomic_v<std::atomic_ref<T>> = true;

/// Simple type trait that we can use to remove `std::atomic` from types.
template <class T>
struct remove_atomic {
  using type = T;
};
template <class T>
struct remove_atomic<std::atomic<T>> {
  using type = T;
};
template <class T>
struct remove_atomic<std::atomic_ref<T>> {
  using type = T;
};
template <class T>
using remove_atomic_t = typename remove_atomic<T>::type;

template <class>
inline constexpr bool is_tbb_range_v = false;
template <class T>
inline constexpr bool is_tbb_range_v<tbb::blocked_range<T>> = true;

template <class>
inline constexpr bool is_tuple_v = false;
template <class... Ts>
inline constexpr bool is_tuple_v<std::tuple<Ts...>> = true;

template <class, class = void>
struct is_iterator : std::false_type {};
template <class T>
struct is_iterator<T*, void> : std::true_type {};
template <class T>
struct is_iterator<const T*, void> : std::true_type {};
template <class T>
struct is_iterator<T, std::enable_if_t<!std::is_same_v<typename std::iterator_traits<T>::value_type, void>>> : std::true_type {};

template <class T>
inline constexpr bool is_iterator_v = is_iterator<T>::value;

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_TRAITS_HPP
