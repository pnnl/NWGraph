
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

template <class>
class graph_adaptor;


// clang-format off
template <class T>
requires (min_idx_adjacency_list<T> || idx_adjacency_list<T>)
class graph_adaptor<T> : public unipartite_graph_base, public T {
  // clang-format on

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

  auto num_vertices() const {
    return base::size();
  }
  auto num_edgees() const {
    return num_edges_;
  }

private:
  auto size() const {
    return base::size();
  }
};


// clang-format off
template <class T>
requires (!min_idx_adjacency_list<T> && !idx_adjacency_list<T>)
class graph_adaptor<T> : public unipartite_graph_base, public T {
  // clang-format on

  using base       = T;
  using graph_base = unipartite_graph_base;

  size_t num_vertices_{0};

public:
  using vertex_id_type = vertex_id_t<T>;

  explicit graph_adaptor(size_t N = 0) : base(N) {
  }

  void open_for_push_back() {
    graph_base::is_open = true;
  }
  void close_for_push_back() {
    graph_base::is_open = false;
  }

  template <class... Attributes>
  void push_back(vertex_id_type i, vertex_id_type j, Attributes... attrs) {
    base::emplace_back(attrs...);
  }

  template <class... Attributes>
  void push_front(vertex_id_type i, vertex_id_type j, Attributes... attrs) {
    base::emplace_front(attrs...);
  }

  auto num_vertices() const {
    return num_vertices_;
  }
  auto num_edges() const {
    return base::size();
  }

private:
  auto size() const {
    return base::size();
  }
};


template <class V>
auto tag_invoke(const num_edges_tag, const graph_adaptor<V>& graph) {
  return graph.num_edges();
}

template <class V>
auto tag_invoke(const num_vertices_tag, const graph_adaptor<V>& graph) {
  return graph.num_vertices();
}

template <class V>
auto& tag_invoke(const source_tag, graph_adaptor<V>& v, typename V::value_type e) {
  return source(V(), e);
}

template <class V>
auto& tag_invoke(const target_tag, graph_adaptor<V>& v, typename V::value_type e) {
  return target(V(), e);
}

template <typename... Ts>
auto graph_edge(std::tuple<Ts...> t) {
  return nth_cdr<1>(t);
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
