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


namespace nw::graph {

template <class>
struct is_swappable : std::false_type { };

template <class T> requires(std::is_lvalue_reference_v<T>)
struct is_swappable<T> : std::true_type { };

template <class... Ts>
struct is_swappable<std::tuple<Ts...>> : std::conjunction<is_swappable<Ts>...> {};

} // namespace nw::graph

namespace std {

#if 1
template <class... Ts> requires (nw::graph::is_swappable<Ts>::value && ...)
void swap(std::tuple<Ts...>&& x, std::tuple<Ts...>&& y) 
{
    using std::swap;
    using std::get;
    [&]<std::size_t... i>(std::index_sequence<i...>) {
        (swap(get<i>(x), get<i>(y)), ...);
    }(std::make_index_sequence<sizeof...(Ts)>());
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
}

#endif // NWGRAPH_TUPLE_HACK_HPP
