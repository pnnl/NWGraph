/**
 * @file graph_adaptor_test.cpp
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

#include <deque>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/graph_adaptor.hpp"

using namespace nw::graph;

TEST_CASE("Has push_back trait", "[graph_adaptor]") {

  SECTION("Vector has push_back") {
    REQUIRE(has_push_back_v<std::vector<int>> == true);
  }

  SECTION("Deque has push_back") {
    REQUIRE(has_push_back_v<std::deque<int>> == true);
  }

  SECTION("List has push_back") {
    REQUIRE(has_push_back_v<std::list<int>> == true);
  }
}

TEST_CASE("Make index map", "[graph_adaptor]") {

  SECTION("String vertices") {
    std::vector<std::string> vertices = {"X", "Y", "Z"};
    auto map = make_index_map(vertices);

    REQUIRE(map["X"] == 0);
    REQUIRE(map["Y"] == 1);
    REQUIRE(map["Z"] == 2);
  }

  SECTION("Integer vertices") {
    std::vector<int> vertices = {100, 200, 300};
    auto map = make_index_map(vertices);

    REQUIRE(map[100] == 0);
    REQUIRE(map[200] == 1);
    REQUIRE(map[300] == 2);
  }
}

TEST_CASE("Graph edge extraction", "[graph_adaptor]") {

  SECTION("graph_edge extracts edge properties") {
    auto full_edge = std::make_tuple(0ul, 1ul, 3.14, "label");
    auto edge_props = graph_edge(full_edge);

    REQUIRE(std::tuple_size_v<decltype(edge_props)> == 3);
    REQUIRE(std::get<0>(edge_props) == 1);
    REQUIRE(std::get<1>(edge_props) == Approx(3.14));
  }

  SECTION("graph_edge with just two elements") {
    auto simple_edge = std::make_tuple(5ul, 10ul);
    auto edge_props = graph_edge(simple_edge);

    REQUIRE(std::tuple_size_v<decltype(edge_props)> == 1);
    REQUIRE(std::get<0>(edge_props) == 10);
  }
}

TEST_CASE("Push back plain fill functions", "[graph_adaptor]") {

  SECTION("Push back plain fill directed") {
    std::vector<std::tuple<size_t, size_t>> edges = {
      {0, 1}, {0, 2}, {1, 2}
    };

    std::vector<std::vector<size_t>> adj(3);
    push_back_plain_fill(edges, adj, true, 0);

    REQUIRE(adj[0].size() == 2);
    REQUIRE(adj[1].size() == 1);
    REQUIRE(adj[2].size() == 0);
  }

  SECTION("Push back plain fill undirected") {
    std::vector<std::tuple<size_t, size_t>> edges = {
      {0, 1}, {1, 2}
    };

    std::vector<std::vector<size_t>> adj(3);
    push_back_plain_fill(edges, adj, false, 0);

    // Undirected: each edge adds in both directions
    REQUIRE(adj[0].size() == 1);  // 0 -> 1
    REQUIRE(adj[1].size() == 2);  // 1 -> 0, 1 -> 2
    REQUIRE(adj[2].size() == 1);  // 2 -> 1
  }
}

TEST_CASE("Data to graph edge list", "[graph_adaptor]") {

  SECTION("Create index edge list from bipartite data") {
    std::vector<std::string> left = {"L1", "L2"};
    std::vector<std::string> right = {"R1", "R2", "R3"};
    std::vector<std::tuple<std::string, std::string>> edges = {
      {"L1", "R1"},
      {"L1", "R2"},
      {"L2", "R2"},
      {"L2", "R3"}
    };

    auto index_edges = data_to_graph_edge_list(left, right, edges);

    REQUIRE(index_edges.size() == 4);
    // L1 -> R1 becomes (0, 0)
    REQUIRE(std::get<0>(index_edges[0]) == 0);
    REQUIRE(std::get<1>(index_edges[0]) == 0);
  }
}
