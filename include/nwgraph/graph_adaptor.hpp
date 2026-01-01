/**
 * @file graph_adaptor.hpp
 * @brief Graph adaptor classes and utilities for building graphs from data.
 *
 * This file provides:
 * - Type traits for detecting container capabilities (push_back, push_front)
 * - The graph_adaptor class template for wrapping adjacency lists
 * - Utility functions for constructing graphs from vertex and edge data
 *
 * Example usage:
 * @code
 * #include <nwgraph/graph_adaptor.hpp>
 * #include <nwgraph/vovos.hpp>
 * #include <vector>
 * #include <string>
 *
 * // Build a graph from vertex labels and edge data
 * std::vector<std::string> vertices = {"A", "B", "C", "D"};
 * std::vector<std::tuple<std::string, std::string>> edges = {
 *     {"A", "B"}, {"A", "C"}, {"B", "D"}, {"C", "D"}
 * };
 *
 * // Create plain graph (no edge properties) from labeled data
 * auto graph = nw::graph::make_plain_graph<nw::graph::vov<>>(vertices, edges);
 *
 * // Or with edge weights
 * std::vector<std::tuple<std::string, std::string, double>> weighted_edges = {
 *     {"A", "B", 1.5}, {"A", "C", 2.0}, {"B", "D", 1.0}
 * };
 * auto wgraph = nw::graph::make_property_graph<nw::graph::vov<double>>(
 *     vertices, weighted_edges);
 *
 * // Use graph_adaptor for dynamic edge insertion
 * nw::graph::graph_adaptor<nw::graph::vov<>> dyn_graph(4);
 * dyn_graph.open_for_push_back();
 * dyn_graph.push_back(0, 1);  // Add edge 0 -> 1
 * dyn_graph.push_back(0, 2);  // Add edge 0 -> 2
 * dyn_graph.close_for_push_back();
 * @endcode
 *
 * @see graph_concepts.hpp for the concepts used to constrain graph types
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

#include "nwgraph/graph_base.hpp"
#include "nwgraph/graph_concepts.hpp"

#ifndef NW_GRAPH_GRAPH_ADAPTOR_HPP
#define NW_GRAPH_GRAPH_ADAPTOR_HPP

namespace nw::graph {


namespace has {

/**
 * @brief Type trait to detect if a container supports push_back.
 * @tparam U The container type to check.
 *
 * This trait checks whether the expression v.push_back(t) is valid
 * for a container of type U with its value_type.
 */
template <class U>
class has_push_back {

  template <class W>
    requires(requires(W v, typename W::value_type t) { v.push_back(t); } == true)
  static constexpr bool test() {
    return true;
  }


  template <class W>
    requires(requires(W v, typename W::value_type t) { v.push_back(t); } == false)
  static constexpr bool test() {
    return false;
  }

public:
  static constexpr bool value = test<U>();
};


/**
 * @brief Type trait to detect if a container supports push_front.
 * @tparam W A template template parameter for the container type.
 * @tparam U The element type of the container.
 *
 * This trait checks whether the expression v.push_front(t) is valid
 * for a container of type W<U>.
 */
template <template <class> class W, class U>
class has_push_front {

  template <template <class> class V, class T>
    requires(requires(V<T> v, T t) { v.push_front(t); } == true)
  static constexpr bool test() {
    return true;
  }


  template <template <class> class V, class T>
    requires(requires(V<T> v, T t) { v.push_front(t); } == false)
  static constexpr bool test() {
    return false;
  }

public:
  static constexpr bool value = test<W, U>();
};

}    // namespace has


/// @brief Variable template for has_push_back trait.
template <class V>
bool has_push_back_v = has::has_push_back<V>::value;

/// @brief Variable template for has_push_front trait.
template <template <class> class V, class T>
bool has_push_front_v = has::has_push_front<V, T>::value;


/**
 * @brief Adaptor class that wraps an adjacency list to provide graph operations.
 * @tparam T The underlying adjacency list type (must satisfy min_idx_adjacency_list or idx_adjacency_list).
 *
 * graph_adaptor provides a uniform interface for building and querying graphs
 * backed by different container types. It supports dynamic edge insertion
 * via push_back and push_front operations.
 *
 * @note The underlying type T must satisfy either min_idx_adjacency_list or
 *       idx_adjacency_list concepts.
 */
template <class T>
  requires(min_idx_adjacency_list<T> || idx_adjacency_list<T>)
class graph_adaptor : public unipartite_graph_base, public T {

  using base       = T;
  using graph_base = unipartite_graph_base;

  size_t num_edges_ { 0 };

public:
  /**
   * @brief Construct a graph adaptor with N vertices.
   * @param N The number of vertices (default: 0).
   */
  explicit graph_adaptor(size_t N = 0) : base(N) {
  }

  /// @brief Open the graph for edge insertion via push_back.
  void open_for_push_back() {
    graph_base::is_open = true;
  }

  /// @brief Close the graph after edge insertion.
  void close_for_push_back() {
    graph_base::is_open = true;
  }

  /**
   * @brief Add an edge to vertex i with the given attributes.
   * @tparam Attributes Types of edge attributes.
   * @param i Source vertex index.
   * @param attrs Edge attributes (target vertex, weights, etc.).
   *
   * If vertex i doesn't exist, the graph is automatically expanded.
   */
  template <class... Attributes>
  void push_back(size_t i, Attributes... attrs) {
    if (i >= base::size()) {
      for (size_t j = base::size(); j <= i; ++j) {
        base::emplace_back();
      }
    }
    base::operator[](i).emplace_back(attrs...);
  }

  /**
   * @brief Add an edge to the front of vertex i's adjacency list.
   * @tparam Attributes Types of edge attributes.
   * @param i Source vertex index.
   * @param attrs Edge attributes (target vertex, weights, etc.).
   *
   * If vertex i doesn't exist, the graph is automatically expanded.
   */
  template <class... Attributes>
  void push_front(size_t i, Attributes... attrs) {
    if (i >= base::size()) {
      for (size_t j = base::size(); j <= i; ++j) {
        base::emplace_back();
      }
    }
    base::operator[](i).emplace_back(attrs...);
  }

  /// @brief Return the number of vertices in the graph.
  auto size() const {
    return base::size();
  }

  /// @brief Return the number of vertices in the graph.
  auto num_vertices() const {
    return base::size();
  }

  /// @brief Return the number of edges in the graph.
  auto num_edgees() const {
    return num_edges_;
  }
};


/**
 * @brief Extract edge properties from a tuple, skipping the first element.
 * @tparam Ts Tuple element types.
 * @param t Edge tuple (source, target, properties...).
 * @return Tuple containing elements starting from index 1.
 */
template <typename... Ts>
auto graph_edge(std::tuple<Ts...> t) {
  return nth_cdr<1>(t);
}

/**
 * @brief Fill a plain graph (no edge properties) from an edge list.
 * @tparam EdgeList Type of the edge list container.
 * @tparam Adjacency Type of the adjacency list to fill.
 * @param edge_list The source edge list.
 * @param adj The adjacency list to populate.
 * @param directed If false, adds reverse edges for undirected graphs.
 * @param idx Index direction: 0 for forward (u->v), 1 for reverse (v->u).
 */
template <class EdgeList, class Adjacency>
void push_back_plain_fill(const EdgeList& edge_list, Adjacency& adj, bool directed, size_t idx) {
  const size_t jdx = (idx + 1) % 2;

  for (auto&& e : edge_list) {

    if (idx == 0) {
      std::apply([&](size_t u, size_t v) { adj[u].emplace_back(v); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v) { adj[v].emplace_back(u); }, e);
      }
    } else {
      std::apply([&](size_t u, size_t v) { adj[v].emplace_back(u); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v) { adj[u].emplace_back(v); }, e);
      }
    }
  }
}

/**
 * @brief Fill a graph with edge properties from an edge list.
 * @tparam EdgeList Type of the edge list container.
 * @tparam Adjacency Type of the adjacency list to fill.
 * @param edge_list The source edge list with properties.
 * @param adj The adjacency list to populate.
 * @param directed If false, adds reverse edges for undirected graphs.
 * @param idx Index direction: 0 for forward (u->v), 1 for reverse (v->u).
 */
template <class EdgeList, class Adjacency>
void push_back_fill(const EdgeList& edge_list, Adjacency& adj, bool directed, size_t idx) {
  const size_t jdx = (idx + 1) % 2;

  for (auto&& e : edge_list) {

    if (idx == 0) {
      std::apply([&](size_t u, size_t v, auto... props) { adj[u].emplace_back(v, props...); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v, auto... props) { adj[v].emplace_back(u, props...); }, e);
      }
    } else {
      std::apply([&](size_t u, size_t v, auto... props) { adj[v].emplace_back(u, props...); }, e);
      if (!directed) {
        std::apply([&](size_t u, size_t v, auto... props) { adj[u].emplace_back(v, props...); }, e);
      }
    }
  }
}


/**
 * @brief Create a plain edge list (no properties) by mapping vertex keys to indices.
 * @tparam EdgeList The type of edge list to create.
 * @tparam M Map type for vertex key to index mapping.
 * @tparam E Range type of the input edges.
 * @param map Vertex key to index mapping.
 * @param edges Input edges with vertex keys.
 * @return Edge list with vertex indices instead of keys.
 */
template <edge_list_graph EdgeList, class M, std::ranges::random_access_range E>
auto make_plain_edges(M& map, const E& edges) {
  EdgeList index_edges;

  for (auto&& e : edges) {
    std::apply([&](auto&& u, auto&& v, auto... props_) { index_edges.push_back(std::make_tuple(map[u], map[v])); }, e);
  }

  return index_edges;
}

/**
 * @brief Create an edge list with properties by mapping vertex keys to indices.
 * @tparam EdgeList The type of edge list to create.
 * @tparam M Map type for vertex key to index mapping.
 * @tparam E Range type of the input edges.
 * @param map Vertex key to index mapping.
 * @param edges Input edges with vertex keys and properties.
 * @return Edge list with vertex indices and copied properties.
 */
template <edge_list_graph EdgeList, class M, std::ranges::random_access_range E>
auto make_property_edges(M& map, const E& edges) {
  EdgeList index_edges;

  for (auto&& e : edges) {
    std::apply([&](auto&& u, auto&& v, auto... props_) { index_edges.push_back(std::make_tuple(map[u], map[v], props_...)); }, e);
  }

  return index_edges;
}


/**
 * @brief Create a plain adjacency list graph from vertex and edge data.
 * @tparam Graph The adjacency list graph type to create.
 * @tparam V Range type for vertices.
 * @tparam E Range type for edges.
 * @param vertices The vertex collection (used to determine graph size).
 * @param edges Edge list as pairs of vertex keys.
 * @param directed If false, adds reverse edges for undirected graphs.
 * @param idx Index direction: 0 for forward, 1 for reverse.
 * @return A graph of type Graph populated from the edge data.
 */
template <adjacency_list_graph Graph, std::ranges::random_access_range V, std::ranges::random_access_range E>
auto make_plain_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {
  auto vertex_map  = make_index_map(vertices);
  auto index_edges = make_plain_edges(vertex_map, edges);

  Graph G(size(vertices));
  push_back_plain_fill(index_edges, G, directed, idx);

  return G;
}

/**
 * @brief Create an index-based adjacency list graph from vertex and edge data.
 * @tparam Graph The adjacency list graph type to create.
 * @tparam V Range type for vertices.
 * @tparam E Range type for edges.
 * @param vertices The vertex collection.
 * @param edges Edge list as pairs of vertex keys with index properties.
 * @param directed If false, adds reverse edges for undirected graphs.
 * @param idx Index direction: 0 for forward, 1 for reverse.
 * @return A graph of type Graph populated from the edge data.
 */
template <adjacency_list_graph Graph, std::ranges::random_access_range V, std::ranges::random_access_range E>
auto make_index_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {

  auto vertex_map  = make_index_map(vertices);
  auto index_edges = make_index_edges(vertex_map, edges);

  Graph G(size(vertices));

  push_back_fill(index_edges, G, directed, idx);

  return G;
}

/**
 * @brief Create a property graph from vertex and edge data.
 * @tparam Graph The adjacency list graph type to create.
 * @tparam V Range type for vertices.
 * @tparam E Range type for edges with properties.
 * @param vertices The vertex collection.
 * @param edges Edge list with vertex keys and edge properties.
 * @param directed If false, adds reverse edges for undirected graphs.
 * @param idx Index direction: 0 for forward, 1 for reverse.
 * @return A graph of type Graph populated from the edge data with properties.
 */
template <adjacency_list_graph Graph, std::ranges::random_access_range V, std::ranges::forward_range E>
auto make_property_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {

  auto vertex_map     = make_index_map(vertices);
  auto property_edges = make_property_edges(vertex_map, edges);

  Graph G(size(vertices));

  push_back_fill(property_edges, G, directed, idx);

  return G;
}

/**
 * @brief Create a plain bipartite graph from two vertex sets and edges.
 * @tparam Graph The adjacency list graph type to create.
 * @tparam V1 Range type for left partition vertices.
 * @tparam V2 Range type for right partition vertices.
 * @tparam E Range type for edges.
 * @param left_vertices Vertices in the left partition.
 * @param right_vertices Vertices in the right partition.
 * @param edges Edge list connecting left and right vertices.
 * @param idx Index direction: 0 for left-to-right, 1 for right-to-left.
 * @return A graph of type Graph representing one side of the bipartite structure.
 */
template <adjacency_list_graph Graph, std::ranges::random_access_range V1, std::ranges::random_access_range V2,
          std::ranges::random_access_range E>
auto make_plain_bipartite_graph(const V1& left_vertices, const V2& right_vertices, const E& edges, size_t idx = 0) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(size(left_vertices));
  push_back_plain_fill(index_edges, G, true, idx);

  return G;
}

/**
 * @brief Create both adjacency structures for a bipartite graph.
 * @tparam Graph The adjacency list graph type to create.
 * @tparam V Range type for vertices.
 * @tparam E Range type for edges.
 * @param left_vertices Vertices in the left partition.
 * @param right_vertices Vertices in the right partition.
 * @param edges Edge list connecting left and right vertices.
 * @return A tuple (G, H) where G is left-to-right and H is right-to-left.
 */
template <adjacency_list_graph Graph, std::ranges::random_access_range V, std::ranges::random_access_range E>
auto make_bipartite_graphs(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list<>(left_vertices, right_vertices, edges);

  Graph G(size(left_vertices));
  Graph H(size(right_vertices));

  push_back_fill(index_edges, G, true, 0);
  push_back_fill(index_edges, H, true, 1);

  return make_tuple(G, H);
}


}    // namespace nw::graph


#endif    // NW_GRAPH_GRAPH_ADAPTOR_HPP
