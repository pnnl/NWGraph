/**
 * @file scrap.cpp
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

#if 0

  // Better solution:
  //   Create tmp array
  //   Copy sorting index array to tmp array
  //   Copy other arrays to cs.to_be_indexed_
  //   Sort everything in to_be_indexed with tmp
  //     Need a custom swap -- pass in ?  ADL ? 
  //   Or zip

  using vertex_id_type = typename decltype(std::get<idx>(el))::value_type;  /* vertex_id_t<edge_list_t> */
  auto perm = nw::util::proxysort(std::get<idx>(el), std::less<vertex_id_type>());

  // Copy other (permuted) indices
  const int kdx = (idx + 1) % 2;
  permute(std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(el)), std::get<0>(cs.to_be_indexed_), perm);
  
  // Copy (permuted) properties
  if constexpr (std::tuple_size<typename edge_list_t::attributes_t>::value > 0) {
    permute_helper(el, cs, std::make_integer_sequence<size_t, std::tuple_size<typename edge_list_t::attributes_t>::value>(), perm);
  }

#endif


// #include "nwgraph/graph_base.hpp"

#if 0

template <typename T>
using inner_range = std::ranges::range_value_t<T>;

template <typename T>
using inner_value = std::ranges::range_value_t<inner_range<T>>;

template <typename T>
using vertex_id_type = nw::graph::vertex_id_type;

template <typename T>
using index_t = nw::graph::vertex_id_type;

template <typename T>
concept Adjacence =
  requires (T graph, inner_value<T> vertex) {
    { target (vertex) } -> std::convertible_to<vertex_id_type<T>>;
  };

template <typename T>
concept Incidence =
  requires (T graph, inner_value<T> edge) {
    { source (edge) } -> std::convertible_to<vertex_id_type<T>>;
    { target (edge) } -> std::convertible_to<vertex_id_type<T>>;
  };

template <typename T>
concept Graph = 
  std::ranges::random_access_range<T> &&
  std::ranges::forward_range<inner_range<T>> &&
  std::convertible_to<vertex_id_type<T>, index_t<T>>;

template <typename T>
concept IncidenceGraph = Graph<T> && Incidence<T>;

template <typename T>
concept AdjacenceGraph = Graph<T> && Adjacence<T>;


namespace nw {
namespace graph {

template <typename E>
concept edge_list_c = std::ranges::random_access_range<E>&& requires(E e) {
  typename E::vertex_id_type;
  typename E::attributes_t;
  typename E::element;
  e.size();
  { std::is_same_v<decltype(E::edge_directedness), directedness> };

};

}    // namespace graph
}    // namespace nw
#endif

template <typename G>
auto num_edges(const G& g) {
  return g.num_edges();
}

template <typename G>
auto begin(const G& g) {
  return g.begin();
}

template <typename G>
auto num_vertices(const G& g) {
  return g.num_vertices();
}
