/**
 * @file tuple_hack.hpp
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

#ifndef NWGRAPH_TUPLE_HACK_HPP
#define NWGRAPH_TUPLE_HACK_HPP

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>


namespace nw::graph {

template <class>
struct is_swappable : std::false_type {};

template <class T>
  requires(std::is_lvalue_reference_v<T>)
struct is_swappable<T> : std::true_type {};

// std::vector<bool> hands out proxy references; the standard library provides
// swap overloads for them that swap the referenced bits.
template <>
struct is_swappable<std::vector<bool>::reference> : std::true_type {};

template <class... Ts>
struct is_swappable<std::tuple<Ts...>> : std::conjunction<is_swappable<Ts>...> {};

template <class>
inline constexpr bool is_std_tuple_v = false;

template <class... Ts>
inline constexpr bool is_std_tuple_v<std::tuple<Ts...>> = true;

}    // namespace nw::graph

namespace std {

#if 1
template <class... Ts>
  requires(nw::graph::is_swappable<Ts>::value && ...)
void swap(std::tuple<Ts...>&& x, std::tuple<Ts...>&& y) {
  using std::get;
  // Nested reference tuples are recursed into explicitly: delegating them to
  // std::tuple::swap is unreliable for proxy-reference elements (it may swap
  // the proxies themselves rather than the referenced bits).
  auto swap_one = []<class T>(T& a, T& b) {
    if constexpr (nw::graph::is_std_tuple_v<T>) {
      swap(std::move(a), std::move(b));
    } else {
      using std::swap;
      swap(a, b);
    }
  };
  [&]<std::size_t... i>(std::index_sequence<i...>) { (swap_one(get<i>(x), get<i>(y)), ...); }(std::make_index_sequence<sizeof...(Ts)>());
}
#else
template <class... Ts, std::size_t... Is>
void swap(std::tuple<Ts&...>&& x, std::tuple<Ts&...>&& y, std::index_sequence<Is...>) {

  (std::swap(std::get<Is>(x), std::get<Is>(y)), ...);
}

template <class... Ts>
void swap(std::tuple<Ts&...>&& x, std::tuple<Ts&...>&& y) {

  swap(std::move(x), std::move(y), std::make_index_sequence<sizeof...(Ts)>());
}
#endif
}    // namespace std

#endif    // NWGRAPH_TUPLE_HACK_HPP
