/**
 * @file dag_range_test.cpp
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

#include <set>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adaptors/dag_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

// Helper to create predecessor and successor lists from edge list
auto make_pred_succ_lists(const std::vector<std::pair<size_t, size_t>>& edges, size_t n) {
  std::vector<std::vector<size_t>> pred(n);
  std::vector<std::vector<size_t>> succ(n);

  for (auto [u, v] : edges) {
    succ[u].push_back(v);
    pred[v].push_back(u);
  }

  return std::make_pair(pred, succ);
}

TEST_CASE("DAG range basic", "[dag_range]") {

  SECTION("Simple linear DAG") {
    // DAG: 0 -> 1 -> 2 -> 3
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    auto [pred, succ] = make_pred_succ_lists({{0, 1}, {1, 2}, {2, 3}}, 4);

    dag_range<adjacency<0>> dag(G, pred, succ);

    std::vector<std::tuple<size_t, size_t, ready_to_process>> visited;
    for (auto&& [u, v, ready] : dag) {
      visited.push_back({u, v, ready});
    }

    // Should visit all edges in topological order
    REQUIRE(!visited.empty());
  }
}

TEST_CASE("DAG range with multiple roots", "[dag_range]") {

  SECTION("Diamond DAG") {
    // DAG:   0   1
    //         \ /
    //          2
    //          |
    //          3
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 2);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    auto [pred, succ] = make_pred_succ_lists({{0, 2}, {1, 2}, {2, 3}}, 4);

    dag_range<adjacency<0>> dag(G, pred, succ);

    std::set<size_t> visited_sources;
    for (auto&& [u, v, ready] : dag) {
      visited_sources.insert(u);
    }

    // Should visit edges from roots 0 and 1
    REQUIRE(visited_sources.count(0) > 0);
    REQUIRE(visited_sources.count(1) > 0);
  }
}

TEST_CASE("DAG range ready_to_process", "[dag_range]") {

  SECTION("ready_to_process enum values") {
    REQUIRE(ready_to_process::yes == 0);
    REQUIRE(ready_to_process::no == 1);
  }
}

TEST_CASE("DAG range empty check", "[dag_range]") {

  SECTION("Empty after traversal") {
    edge_list<directedness::directed> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    auto [pred, succ] = make_pred_succ_lists({{0, 1}}, 2);

    dag_range<adjacency<0>> dag(G, pred, succ);

    REQUIRE(!dag.empty());

    // Consume all
    for (auto&& elem : dag) {
      (void)elem;
    }

    REQUIRE(dag.empty());
  }
}
