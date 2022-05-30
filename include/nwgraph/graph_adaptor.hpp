
#include "nwgraph/graph_base.hpp"
#include "nwgraph/graph_concepts.hpp"

#ifndef NW_GRAPH_GRAPH_ADAPTOR_HPP
#define NW_GRAPH_GRAPH_ADAPTOR_HPP

namespace nw::graph {


namespace has {
template <class U>
class has_push_back {

  template <class W>
  requires(requires(W v, typename W::value_type t) { v.push_back(t); } == true) static constexpr bool test() {
    return true;
  }


  template <class W>
  requires(requires(W v, typename W::value_type t) { v.push_back(t); } == false) static constexpr bool test() {
    return false;
  }

public:
  static constexpr bool value = test<U>();
};


template <template <class> class W, class U>
class has_push_front {

  template <template <class> class V, class T>
  requires(requires(V<T> v, T t) { v.push_front(t); } == true) static constexpr bool test() {
    return true;
  }


  template <template <class> class V, class T>
  requires(requires(V<T> v, T t) { v.push_front(t); } == false) static constexpr bool test() {
    return false;
  }

public:
  static constexpr bool value = test<W, U>();
};

}    // namespace has


template <class V>
bool has_push_back_v = has::has_push_back<V>::value;

template <template <class> class V, class T>
bool has_push_front_v = has::has_push_front<V, T>::value;


template <class T>
requires(min_idx_adjacency_list<T> || idx_adjacency_list<T>) class graph_adaptor : public unipartite_graph_base, public T {

  using base       = T;
  using graph_base = unipartite_graph_base;

  size_t num_edges_{0};

public:
  explicit graph_adaptor(size_t N = 0) : base(N) {
  }

  void open_for_push_back() {
    graph_base::is_open = true;
  }
  void close_for_push_back() {
    graph_base::is_open = true;
  }

  template <class... Attributes>
  void push_back(size_t i, Attributes... attrs) {
    if (i >= base::size()) {
      for (size_t j = base::size(); j <= i; ++j) {
        base::emplace_back();
      }
    }
    base::operator[](i).emplace_back(attrs...);
  }

  template <class... Attributes>
  void push_front(size_t i, Attributes... attrs) {
    if (i >= base::size()) {
      for (size_t j = base::size(); j <= i; ++j) {
        base::emplace_back();
      }
    }
    base::operator[](i).emplace_back(attrs...);
  }

  auto size() const {
    return base::size();
  }
  auto num_vertices() const {
    return base::size();
  }
  auto num_edgees() const {
    return num_edges_;
  }
};


template <typename... Ts>
auto graph_edge(std::tuple<Ts...> t) {
  return nth_cdr<1>(t);
}

/**
 * Fill a plain graph from edge list
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
 * Fill a non-plain graph from edge list
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
 * Make an edge list with properties copied from original data, e.g., vector<tuple<size_t, size_t, props...>>
 */
//template <template <class> class I = std::vector, class M, std::ranges::random_access_range E>
template <edge_list_graph EdgeList, class M, std::ranges::random_access_range E>
auto make_plain_edges(M& map, const E& edges) {
  EdgeList index_edges;

  for (auto&& e : edges) {
    std::apply([&](auto&& u, auto&& v, auto... props_) { index_edges.push_back(std::make_tuple(map[u], map[v])); }, e);
  }

  return index_edges;
}

/**
 * Make an edge list with properties copied from original data, e.g., vector<tuple<size_t, size_t, props...>>
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
 *  Make a plain graph from data, e.g., vector<vector<index>>
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
 *  Make an index graph from data, e.g., vector<vector<tuple<index, index>>>
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
 *  Make a property graph from data, e.g., vector<vector<tuple<index, properties...>>>
 */
template <adjacency_list_graph Graph, std::ranges::random_access_range V, std::ranges::forward_range E>
auto make_property_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {

  auto vertex_map     = make_index_map(vertices);
  auto property_edges = make_property_edges(vertex_map, edges);

  Graph G(size(vertices));

  push_back_fill(property_edges, G, directed, idx);

  return G;
}

template <adjacency_list_graph Graph, std::ranges::random_access_range V1, std::ranges::random_access_range V2,
          std::ranges::random_access_range E>
auto make_plain_bipartite_graph(const V1& left_vertices, const V2& right_vertices, const E& edges, size_t idx = 0) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(size(left_vertices));
  push_back_plain_fill(index_edges, G, true, idx);

  return G;
}

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
