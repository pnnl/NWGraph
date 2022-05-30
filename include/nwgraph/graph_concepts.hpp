/**
 * @file graph_concepts.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_GRAPH_CONCEPTS_HPP
#define NW_GRAPH_GRAPH_CONCEPTS_HPP

#include <array>
#include <concepts>
#include <iterator>
#include <ranges>
#include <tuple>

#include "nwgraph/graph_traits.hpp"
#include "nwgraph/util/tag_invoke.hpp"

namespace nw::graph {

  /**
   * Forward-declare our CPO tags
   */
DECL_TAG_INVOKE(num_vertices);
DECL_TAG_INVOKE(num_edges);
DECL_TAG_INVOKE(degree);
DECL_TAG_INVOKE(source);
DECL_TAG_INVOKE(target);

/** @file
 * @concept graph
 *
 * @brief Base concept for types fulfilling requirements of a graph.  
 * 
 * The `graph` concept requires the following:
 * - The type `G` must be `copyable` 
 * - `vertex_id_t<G>` must be a valid typename
 * - If `g` is of type `G`, the expression `num_vertices(g)` is a *customization point object*
 *   returning a type convertible `std::ranges::range_difference_t<G>`
 *
 * To enable standard library types to meet the requirements of the `graph` concept, appropriate
 * definitions of `vertex_id_t` and `num_vertices` have been made in the `graph_concepts.hpp`
 * header file.
 *
 * @headerfile nwgraph/graph_concepts.hpp
 *
 */
template <typename G>
concept graph = std::copyable<G> && requires(G g) {
  typename vertex_id_t<G>;
  { num_vertices(g) } -> std::convertible_to<std::ranges::range_difference_t<G>>;
};

template <typename G>
using inner_range_t = std::ranges::range_value_t<G>;

template <class T>
using iterator_t = decltype(std::ranges::begin(std::declval<T&>()));

template <class T>
using const_iterator_t = decltype(std::ranges::cbegin(std::declval<T&>()));

template <typename G>
using inner_iterator_t = iterator_t<inner_range_t<G>>;

template <typename G>
using inner_const_iterator_t = const_iterator_t<inner_range_t<G>>;

template <typename G>
using inner_iterator_t = iterator_t<inner_range_t<G>>;

template <typename G>
using inner_const_iterator_t = const_iterator_t<inner_range_t<G>>;

template <typename G>
using inner_value_t = std::ranges::range_value_t<inner_range_t<G>>;

template <typename G>
using inner_reference_t = std::ranges::range_reference_t<inner_range_t<G>>;


template <size_t N, typename... Ts>
auto nth_cdr(std::tuple<Ts...> t) {
  return [&]<std::size_t... Ns>(std::index_sequence<Ns...>) { return std::tuple{std::get<Ns + N>(t)...}; }
  (std::make_index_sequence<sizeof...(Ts) - N>());
}

template <typename... Ts>
auto props(std::tuple<Ts...> t) {
  return nth_cdr<2>(t);
}

template <typename G>
using attributes_t = decltype(nth_cdr<1>(inner_value_t<G>{}));


/**
 * @concept adjacency_list_graph
 *
 * @headerfile nwgraph/graph_concepts.hpp
 *
 * @brief Concept for types fulfilling requirements of a graph in adjacency format.
 * 
 * The `adjacency_list_graph` concept requires the following:
 * - The type `G` must meet the requirements of `graph`
 * - The type `G` must meet the requirements of `std::ranges::random_access_range`
 * - The *inner range* of `G`, i.e., the type given by `std::ranges::range_value_t<G>` 
 *   must meet the requirements of `std::ranges::forward_range`
 * - The type `vertex_id_t<G>` must be convertible to `std::ranges::range_difference_t` of the
 *   inner range.
 * - If `g` is of type 'G' and `u` is an element of type `vertex_id_t<G>`, then `g[u]` is 
 *   a valid expression returning a type convertible to the inner range of `G`.
 * - If `g` is of `graph` type `G` and `e` is of the value type of the inner range, then
 *   the *customization point object* `target(g, e)` returns a type convertible to 
 *   `vertex_id_t<G>`
 *
 * To enable standard library types to meet the requirements of the `adjacency_list_graph` concept,
 * appropriate definitions of `target` have been made in the `graph_concepts.hpp` header file.
 *
 * **Example:** Standard library components meeting the requirements of `adjacency_list_graph`
 *
 *     #include <forward_list>
 *     #include <list>
 *     #include <tuple>
 *     #include <vector>
 *
 *     int main() {
 *       static_assert(nw::graph::adjacency_list_graph<std::vector<std::forward_list<std::tuple<int, int, double>>>);
 *       static_assert(nw::graph::adjacency_list_graph<std::vector<std::list<std::tuple<int, int, double>>>);
 *       static_assert(nw::graph::adjacency_list_graph<std::vector<std::vector<std::tuple<int, int, double>>>);
 *     }
 *
 *     template <typename G>
 *     concept adjacency_list_graph = graph<G>;
 *
 *     template <adjacency_list_graph Graph, typename score_t = float, typename accum_t = size_t>
 *     std::vector<score_t> brandes_bc(const Graph& G, bool normalize = true) {
 *
 */
template <typename G>
concept adjacency_list_graph = graph<G>
  && std::ranges::random_access_range<G>
  && std::ranges::forward_range<inner_range_t<G>>
  && std::convertible_to<vertex_id_t<G>,std::ranges::range_difference_t<G>>
  && requires(G g, vertex_id_t<G> u, inner_reference_t<G> e) {
  { g[u] } -> std::convertible_to<inner_range_t<G>>;
  { target(g, e) } -> std::convertible_to<vertex_id_t<G>>;
};

/**
 * @concept degree_enumerable_graph
 *
 * @headerfile nwgraph/graph_concepts.hpp
 *
 * @brief Concept for types fulfilling requirements of a graph whose neighborhood size can be queried in constant time.
 * 
 * The `degree_enumerable_graph` concept requires the following:
 * - The type `G` must meet the requirements o `adjacency_list_graph` 
 * - If `g` is of graph type `G` and `u` is of the vertex id type,
 *   the *customization point object* `degree(g[u])` returns a type
 *   convertible to the difference type of `G`.  The `degree` operation
 *   is guaranteed to be constant time.
 */
template <typename G>
concept degree_enumerable_graph = adjacency_list_graph<G>
  && requires (G g, vertex_id_t<G> u) {
  { degree(g[u]) } -> std::convertible_to<std::ranges::range_difference_t<G>>;
};

/**
 * @concept edge_list_graph
 *
 * @headerfile nwgraph/graph_concepts.hpp
 *
 * @brief Concept for types fulfilling requirements of a graph in edge list format.
 * 
 * The `edge_list_graph` concept requires the following:
 * - The type `G` must meet the requirements of `graph`
 * - The type `G` must meet the requirements of `std::ranges::forward_range`
 * - If `g` is of type 'G' and `e` is an element of type `vertex_id_t<G>`, then
 *   `source(g, e)` returns a type convertible to the vertex type of G.
 * - If `g` is of type 'G' and `e` is an element of type `vertex_id_t<G>`, then
 *   `target(g, e)` returns a type convertible to the vertex type of G.
 *
 * To enable standard library types to meet the requirements of the `edge_list_graph` concept,
 * appropriate definitions of `source` and `target` have been made in the `graph_concepts.hpp` 
 * header file.
 *
 * **Example:** Standard library components meeting the requirements of `edge_list_graph`
 *
 *     #include <forward_list>
 *     #include <list>
 *     #include <tuple>
 *     #include <vector>
 *
 *     int main() {
 *       static_assert(nw::graph::edge_list_graph<std::forward_list<std::tuple<int, int, double>>);
 *       static_assert(nw::graph::edge_list_graph<std::list<std::tuple<int, int, double>>);
 *       static_assert(nw::graph::edge_list_graph<std::vector<std::tuple<int, int, double>>);
 *     }
 */
template <typename G>
concept edge_list_graph = graph<G>
  && std::ranges::forward_range<G>
  && requires(G g, std::ranges::range_reference_t<G> e) {
  { source(g, e) } -> std::convertible_to<vertex_id_t<G>>;
  { target(g, e) } -> std::convertible_to<vertex_id_t<G>>;
};

// Some default traits for common classes of graphs

template <template <class> class Outer, template <class> class Inner, std::integral Index, typename... Attributes>
requires std::ranges::random_access_range<Outer<Inner<std::tuple<Index, Attributes...>>>> &&
      std::ranges::forward_range<Inner<std::tuple<Index, Attributes...>>>
struct graph_traits<Outer<Inner<std::tuple<Index, Attributes...>>>> {
  using vertex_id_type = Index;
};

template <template <class> class Outer, template <class> class Inner, std::integral Index>
requires std::ranges::random_access_range<Outer<Inner<Index>>> && std::ranges::forward_range<Inner<Index>>
struct graph_traits<Outer<Inner<Index>>> {
  using vertex_id_type = Index;
};


// The following concepts are to capture some general concrete graphs that are used: 
// range of range of vertex and range of range of tuples

template <typename R>
concept vertex_list_c = std::ranges::forward_range<R> && !std::is_compound_v<std::ranges::range_value_t<R>>;

template <typename R>
concept edge_list_c = std::ranges::forward_range<R> && requires(std::ranges::range_value_t<R> e) {
  std::get<0>(e);
};

template <typename R>
concept property_edge_list_c = std::ranges::forward_range<R> && requires(std::ranges::range_value_t<R> e) {
  std::get<1>(e);
};

//This concept is for CPO definition. It is not a graph concept,
// comparing with adjacency_list_graph concept.
template <typename G>
concept min_idx_adjacency_list = 
     std::ranges::random_access_range<G>
  && vertex_list_c<inner_range_t<G>>
  && std::is_convertible_v<inner_value_t<G>, std::ranges::range_difference_t<G>>
  && requires(G g, inner_value_t<G> u) {
     { g[u] } -> std::convertible_to<inner_range_t<G>>;
};

//This concept is for CPO definition. It is not a graph concept,
// comparing with adjacency_list_graph concept.
template <typename G>
concept idx_adjacency_list = 
     std::ranges::random_access_range<G>
  && edge_list_c<inner_range_t<G>>
  && std::is_convertible_v<std::tuple_element_t<0, inner_value_t<G>>, std::ranges::range_difference_t<G>>
  && requires(G g, std::tuple_element_t<0, inner_value_t<G>> u) {
  { g[u] } -> std::convertible_to<inner_range_t<G>>;
};


// Based on the above concepts, we define concept-based overloads for vertex_id_type and some CPOs

// Graph traits
template <min_idx_adjacency_list G>
struct graph_traits<G> {
  using vertex_id_type = inner_value_t<G>;
};

template <idx_adjacency_list G>
struct graph_traits<G> {
  using vertex_id_type = std::tuple_element_t<0, inner_value_t<G>>;
};

// target CPO
template <idx_adjacency_list T, class U>
auto& tag_invoke(const target_tag, const T& graph, const U& e) {
  return std::get<0>(e);
}

template <min_idx_adjacency_list T, class U>
auto& tag_invoke(const target_tag, const T& graph, const U& e) {
  return e;
}

// num_vertices CPO
template <idx_adjacency_list T>
auto tag_invoke(const num_vertices_tag, const T& graph) {
  return (vertex_id_t<T>) graph.size();
}

template <min_idx_adjacency_list T>
auto tag_invoke(const num_vertices_tag, const T& graph) {
  return (vertex_id_t<T>) graph.size();
}




}    // namespace nw::graph

#endif    //  NW_GRAPH_GRAPH_CONCEPTS_HPP
