/**
 * @file ranges_and_concepts_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *   liux238
 *
 */

#if 0
template <typename G>
concept AdjacencyGraphAA = 
  requires(G g, size_t i) {
    typename G::iterator;
    { g.begin() } -> std::convertible_to<typename G::iterator>;
    { g.end()   } -> std::convertible_to<typename G::iterator>;
    { g[i]      } -> std::convertible_to<typename G::iterator::value_type>;
    { g.size()  } -> std::convertible_to<size_t>;
};

#include <ranges>


template <typename G>
concept AdjacencyGraph = std::ranges::random_access_range<G>;


template< class T >
concept _range = requires(T& t) {
  t.begin();
  t.end();
};

template< class T >
concept _sized_range = _range<T> &&
  requires(T& t) {
    t.size();
  };

template<class T>
concept _input_range =
  _range<T> ; // && std::input_iterator<typename T::iterator>;

template<class T>
concept _forward_range =
  _input_range<T> ; // && std::forward_iterator<typename T::iterator>;

template<class T>
concept _bidirectional_range =
  _forward_range<T> ; // && std::bidirectional_iterator<typename T::iterator>;

template<class T>
concept _random_access_range =
  _bidirectional_range<T> ; //  && std::random_access_iterator<typename T::iterator>;

  // outer_iterator ... failing is_constructible<>


template<typename G>
concept AdjacencyGraph = ranges::random_access_range<G> && 
  ranges::forward_range<typename G::iterator::value_type> &&
  std::is_convertible_v<typename G::iterator::value_type::iterator::value_type, typename G::index_type>;
  // value_type of inner_range is a tuple
  //   edge? -- tuple is source, target, edge properties
  //   vertex? -- tuple is target, vertex properties -- or are vertex properties always indexable (random_access_range)?
  

  // In original Boost.Graph
  //   AdjacencyGraph => inner container is vertices (rarely used)
  //   IncidenceGraph => inner container is edges
  //     with Incidence Graph -- one can get neighbor vertices and vertex properties if vertex properties are indexable
  
template<typename G>
concept IncidenceGraph = ranges::random_access_range<G> && 
  ranges::forward_range<typename G::iterator::value_type> &&
  std::is_same_v<typename G::iterator::value_type::iterator::value_type, std::tuple<vertex_id_type,vertex_id_type, EdgeProps...>>;
  // Still need to be able store this as CSR
  // Adapt to edge_range


template<typename G>
concept TopologyGraph = ranges::random_access_range<G> && 
  ranges::forward_range<typename G::iterator::value_type> &&
  std::is_same_v<typename G::iterator::value_type::iterator::value_type, std::tuple<vertex_id_type, VertProp??...>>;

  //   I. Conceptify interfaces -> IncidenceGraph
  //      A. const 
  //  II. Refactor algorithms to use IncidenceGraph
  // III. Maybe not use std::get<0> and std::get<1> for accessing edge information (source/target?)
  //  IV. Maybe rather than begin()/end() -> neighbors()? or out_edges()?
#endif

#include "aolos.hpp"
#include "compressed.hpp"
#include "nwgraph/util/intersection_size.hpp"
#include "vovos.hpp"
#include <concepts>
#include <ranges>

template <typename T>
using inner_range = std::ranges::range_value_t<T>;

template <typename T>
using inner_value = std::ranges::range_value_t<inner_range<T>>;

template <typename T>
using vertex_id_type = nw::graph::vertex_id_type;

template <typename T>
using index_t = nw::graph::vertex_id_type;

template <typename T>
concept Adjacence = requires(T graph, inner_value<T> vertex) {
  { target(vertex) }
  ->std::convertible_to<vertex_id_type<T>>;
};

template <typename T>
concept Incidence = requires(T graph, inner_value<T> edge) {
  { source(edge) }
  ->std::convertible_to<vertex_id_type<T>>;
  { target(edge) }
  ->std::convertible_to<vertex_id_type<T>>;
};

template <typename T>
concept Graph =
    std::ranges::random_access_range<T>&& std::ranges::forward_range<inner_range<T>>&& std::convertible_to<vertex_id_type<T>, index_t<T>>;

template <typename T>
concept IncidenceGraph = Graph<T>&& Incidence<T>;

template <typename T>
concept AdjacenceGraph = Graph<T>&& Adjacence<T>;

template <Graph GraphT>
size_t triangle_count(const GraphT& A) {
  size_t triangles = 0;
  auto   first     = A.begin();
  auto   last      = A.end();

  for (auto G = first; first != last; ++first) {
    for (auto v = (*first).begin(); v != (*first).end(); ++v) {
      triangles += nw::graph::intersection_size(*first, G[std::get<0>(*v)]);
    }
  }

  return triangles;
}

template <typename Graph_t>
auto bfs(const Graph_t& graph, vertex_id_type<Graph_t> root) requires Graph<Graph_t> {

  std::deque<vertex_id_type<Graph_t>>  q1, q2;
  std::vector<vertex_id_type<Graph_t>> level(graph.size(), std::numeric_limits<vertex_id_type<Graph_t>>::max());
  std::vector<vertex_id_type<Graph_t>> parents(graph.size(), std::numeric_limits<vertex_id_type<Graph_t>>::max());
  size_t                               lvl = 0;

  q1.push_back(root);
  level[root]   = lvl++;
  parents[root] = root;


  while (!q1.empty()) {

    std::for_each(q1.begin(), q1.end(), [&](vertex_id_type<Graph_t> u) {
      std::for_each(graph[u].begin(), graph[u].end(), [&](auto&& x) {
        vertex_id_type<Graph_t> v = std::get<0>(x);
        if (level[v] == std::numeric_limits<vertex_id_type<Graph_t>>::max()) {
          q2.push_back(v);
          level[v]   = lvl;
          parents[v] = u;
        }
      });
    });
    std::swap(q1, q2);
    q2.clear();
    ++lvl;
  }
  return parents;
}

template <typename T>
auto foo(const T& A) requires Graph<T> {

  auto _b = std::ranges::begin(A);
  auto _e = std::ranges::end(A);
  auto _f = A[0];

  auto _b_b = std::ranges::begin(*_b);
  auto _b_e = std::ranges::end(*_b);

  for (auto& j : A) {
    for (auto& k : j) {
    }
  }

  for (auto& j : A[0]) {
  }
}

int main() {

  triangle_count(nw::graph::vov<>(5));
  triangle_count(nw::graph::adj_list<>(5));

  bfs(nw::graph::vov<>(5), 0);
  bfs(nw::graph::adj_list<>(5), 0);

  foo(nw::graph::vector_of_vector_of_structs(5));
  foo(nw::graph::vov<>(5));

  foo(nw::graph::array_of_list_of_structs(5));
  foo(nw::graph::adj_list(5));

  // foo<nw::graph::adjacency<0>>();

  return 0;
}
