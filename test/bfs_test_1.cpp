/**
 * @file bfs_test_1.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   Tony Liu
 *   liux238
 *
 */

#include <iostream>
#include <queue>
#include <vector>

#include "nwgraph/algorithms/bfs.hpp"
#include "nwgraph/containers/aos.hpp"

#include "common/abstract_test.hpp"

#define PRINT_OUT

using namespace nw::graph;
using namespace nw::util;

//****************************************************************************
template <typename EdgeListT>
bool validate(EdgeListT& aos, size_t seed, std::vector<vertex_id_t<EdgeListT>> const& distance,
              std::vector<vertex_id_t<EdgeListT>> const& predecessor) {
  bool pass(true);

  /// @note Should we parse the orginal mmio file instead?
  for (auto&& [u, v] : aos) {
    int    localdiff = distance[u] - distance[v];
    size_t diff      = abs(localdiff);
#ifdef PRINT_OUT
    std::cout << "Checking edge: " << u << " (" << distance[u] << ", " << predecessor[u] << ") -> " << v << " (" << distance[v] << ", "
              << predecessor[v] << "), diff = " << diff << std::endl;
#endif
    if (diff > 1) {
#ifdef PRINT_OUT
      std::cerr << "ERROR: too far: dist[" << u << "] = " << distance[u] << " too far from dist[" << v << "] = " << distance[v] << std::endl;
#endif
      pass = false;
      break;
    }
  }

  //std::cout << "vertex\tdepth\tparent\tpass-fail" << std::endl;
  for (size_t vid = 0; vid < distance.size(); ++vid) {
    bool passed = ((vid == seed) && (distance[vid] == 0)) || (distance[vid] == (1 + distance[predecessor[vid]]));
    pass &= passed;
    //std::cout << vid+1
    //          << ":\t" << distance[vid]
    //          << "\t" << predecessor[vid]+1
    //          << "\t" << (passed ? "PASS" : "FAIL") << std::endl;
  }

  return pass;
}

template <typename Graph>
auto bfs_m1(const Graph& graph, vertex_id_t<Graph> root) {
  using vertex_id_type = vertex_id_t<Graph>;

  std::deque<vertex_id_type>  q1, q2;
  std::vector<vertex_id_type> level(graph.size(), std::numeric_limits<vertex_id_type>::max());
  std::vector<vertex_id_type> pred(graph.size());
  size_t                      lvl = 0;

  q1.push_back(root);
  level[root] = lvl++;


  while (!q1.empty()) {

    std::for_each(q1.begin(), q1.end(), [&](vertex_id_type u) {
      std::for_each(graph[u].begin(), graph[u].end(), [&](auto&& x) {
        auto v = target(graph, x);
        if (level[v] == std::numeric_limits<vertex_id_type>::max()) {
          q2.push_back(v);
          level[v] = lvl;
          pred[v]  = u;
        }
      });
    });
    std::swap(q1, q2);
    q2.clear();
    ++lvl;
  }
  return std::make_pair(level, pred);
}

TEST_CASE("BFS traversal", "[bfs]") {

  test_util            util;
  auto                 aos_a = util.generate_directed_aos();
  directed_csr_graph_t A(aos_a);

  SECTION("default seed") {
    auto&& [distance, predecessor] = bfs_m1(A, 0);
    REQUIRE(validate(aos_a, 0, distance, predecessor));
  }

  SECTION("another seed") {
    auto&& [distance, predecessor] = bfs_m1(A, 1);
    REQUIRE(validate(aos_a, 1, distance, predecessor));
  }
}
