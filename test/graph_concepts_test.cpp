/**
 * @file graph_concepts_test.cpp
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

#include "common/test_header.hpp"

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/graph_traits.hpp"
#include "nwgraph/volos.hpp"

using namespace nw::graph;

// Test that graph types satisfy the appropriate concepts
TEST_CASE("Graph concepts compile-time checks", "[concepts]") {

  SECTION("adjacency satisfies graph concepts") {
    // Create a simple adjacency graph
    edge_list<directedness::directed> edges(5);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.push_back(3, 4);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Verify basic properties
    REQUIRE(num_vertices(G) == 5);

    // Static assertions for concepts (compile-time checks)
    static_assert(std::ranges::random_access_range<adjacency<0>>);
    static_assert(std::ranges::sized_range<adjacency<0>>);
  }

  SECTION("adj_list satisfies graph concepts") {
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adj_list<0> G(edges);

    REQUIRE(num_vertices(G) == 4);
  }
}

TEST_CASE("Graph traits", "[traits]") {

  SECTION("vertex_id_type trait") {
    using G = adjacency<0>;
    using vid_t = vertex_id_t<G>;

    // Default vertex id should be unsigned
    static_assert(std::is_unsigned_v<vid_t>);
  }

  SECTION("directedness trait") {
    edge_list<directedness::directed> directed_edges(2);
    edge_list<directedness::undirected> undirected_edges(2);

    REQUIRE(directed_edges.size() == 0);
    REQUIRE(undirected_edges.size() == 0);
  }
}

TEST_CASE("Graph operations", "[graph-ops]") {

  SECTION("num_vertices for adjacency") {
    edge_list<directedness::directed> edges(10);
    adjacency<0> adj_graph(edges);
    REQUIRE(num_vertices(adj_graph) == 10);
  }

  SECTION("num_vertices for adj_list") {
    edge_list<directedness::directed> edges(10);
    adj_list<0> list_graph(edges);
    REQUIRE(num_vertices(list_graph) == 10);
  }

  SECTION("degree computation") {
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 2);
    edges.push_back(0, 3);
    edges.push_back(1, 2);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Vertex 0 has out-degree 3
    REQUIRE(G[0].size() == 3);
    // Vertex 1 has out-degree 1
    REQUIRE(G[1].size() == 1);
    // Vertex 2 has out-degree 0
    REQUIRE(G[2].size() == 0);
    // Vertex 3 has out-degree 0
    REQUIRE(G[3].size() == 0);
  }
}

TEST_CASE("Graph with vertices but no edges", "[no-edges]") {

  SECTION("All vertices should have degree 0") {
    edge_list<directedness::directed> edges(5);
    adjacency<0> G(edges);

    REQUIRE(num_vertices(G) == 5);

    // All vertices should have degree 0
    for (size_t i = 0; i < num_vertices(G); ++i) {
      REQUIRE(G[i].size() == 0);
    }
  }
}

TEST_CASE("Self-loops and multi-edges", "[special-edges]") {

  SECTION("Self-loop handling") {
    edge_list<directedness::directed> edges(3);
    edges.open_for_push_back();
    edges.push_back(0, 0);  // self-loop
    edges.push_back(0, 1);
    edges.push_back(1, 1);  // self-loop
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Vertex 0 should have 2 outgoing edges (including self-loop)
    REQUIRE(G[0].size() == 2);
    // Vertex 1 should have 1 outgoing edge (self-loop)
    REQUIRE(G[1].size() == 1);
  }

  SECTION("Multi-edge handling") {
    edge_list<directedness::directed> edges(2);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(0, 1);  // duplicate edge
    edges.push_back(0, 1);  // another duplicate
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // Without deduplication, should have 3 edges
    REQUIRE(G[0].size() == 3);
  }
}
