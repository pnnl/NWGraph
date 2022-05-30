/**
 * @file concepts_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   liux238
 *
 */


#include <deque>
#include <tuple>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/vovos.hpp"
#include "nwgraph/graphs/karate-graph.hpp"
#include "nwgraph/graphs/ospf-graph.hpp"
#include "nwgraph/graphs/imdb-graph.hpp"

template <nw::graph::edge_list_c edge_list_t>
auto foo(edge_list_t el) {}

template <nw::graph::edge_list_c edge_list_t>
auto bar(const edge_list_t& el) {}

template <nw::graph::edge_list_c edge_list_t>
auto baz(edge_list_t&& el) {}

template <nw::graph::adjacency_list_graph Graph>
auto bfs_vv(const Graph& graph, typename nw::graph::graph_traits<Graph>::vertex_id_type root) {
  using vertex_id_type = typename nw::graph::graph_traits<Graph>::vertex_id_type;

  std::deque<vertex_id_type>  q1, q2;
  std::vector<vertex_id_type> level(nw::graph::num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  std::vector<vertex_id_type> parents(nw::graph::num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  size_t                      lvl = 0;

  q1.push_back(root);
  level[root]   = lvl++;
  parents[root] = root;

  while (!q1.empty()) {
    std::for_each(q1.begin(), q1.end(), [&](vertex_id_type u) {
      std::for_each(graph[u].begin(), graph[u].end(), [&](auto&& x) {
        vertex_id_type v = nw::graph::target(graph, x);
        if (level[v] == std::numeric_limits<vertex_id_type>::max()) {
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


int main() {
  static_assert(nw::graph::min_idx_adjacency_list<decltype(karate_directed_adjacency_list)>);
  static_assert(nw::graph::idx_adjacency_list<decltype(ospf_index_adjacency_list)>);
  static_assert(nw::graph::idx_adjacency_list<decltype(actor_actor_adjacency_list)>);

  nw::graph::edge_list<nw::graph::directedness::directed> e{{0, 0}, {0, 4}, {4, 4}, {4, 0}};
  static_assert(nw::graph::edge_list_c<decltype(e)>);
  static_assert(nw::graph::edge_list_graph<decltype(e)>);
  
  std::vector<std::tuple<int, int>> f;
  static_assert(nw::graph::edge_list_c<decltype(f)>);
  
  auto g = nw::graph::make_zipped(std::vector<int>(), std::vector<int>());
  static_assert(nw::graph::edge_list_c<decltype(g)>);
  
  auto h = nw::graph::make_adjacency<0>(e);
  static_assert(nw::graph::adjacency_list_graph<decltype(h)>);
  static_assert(nw::graph::edge_list_c<decltype(h[0])>);
  
  auto i = nw::graph::make_adjacency<1>(e);
  static_assert(nw::graph::adjacency_list_graph<decltype(h)>);
  static_assert(nw::graph::edge_list_c<decltype(i[0])>);
  
  bfs_vv(h, 0);
  
  bfs_vv(karate_directed_adjacency_list, 0);
  bfs_vv(karate_undirected_adjacency_list, 0);
  
  return 0;
}
