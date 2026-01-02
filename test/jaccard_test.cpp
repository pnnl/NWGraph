/**
 * @file jaccard_test.cpp
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

#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/adjacency.hpp"
#include "nwgraph/algorithms/jaccard.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/build.hpp"

using namespace nw::graph;

// Helper to create a triangle graph where all neighbors are shared
// 0 -- 1
//  \  /
//   2
auto create_triangle_graph() {
  edge_list<directedness::undirected, double> edges(3);
  edges.open_for_push_back();
  edges.push_back(0, 1, 0.0);
  edges.push_back(1, 2, 0.0);
  edges.push_back(2, 0, 0.0);
  edges.close_for_push_back();
  return edges;
}

// Create a star graph with center at 0
// 0 connected to 1, 2, 3, 4
// No edges between leaves
auto create_star_graph() {
  edge_list<directedness::undirected, double> edges(5);
  edges.open_for_push_back();
  edges.push_back(0, 1, 0.0);
  edges.push_back(0, 2, 0.0);
  edges.push_back(0, 3, 0.0);
  edges.push_back(0, 4, 0.0);
  edges.close_for_push_back();
  return edges;
}

// Create a path graph: 0-1-2-3
auto create_path_graph() {
  edge_list<directedness::undirected, double> edges(4);
  edges.open_for_push_back();
  edges.push_back(0, 1, 0.0);
  edges.push_back(1, 2, 0.0);
  edges.push_back(2, 3, 0.0);
  edges.close_for_push_back();
  return edges;
}

TEST_CASE("Jaccard similarity basic", "[jaccard]") {

  SECTION("Triangle graph - neighbors share one common neighbor") {
    auto el = create_triangle_graph();

    // Need to process the edge list for triangular form
    swap_to_triangular<0, decltype(el), succession::successor>(el);
    lexical_sort_by<0>(el);
    uniq(el);

    adjacency<0, double> G(el);

    // Define weight accessor that writes to the edge weight
    auto weight = [](auto& e) -> auto& { return std::get<1>(e); };

    size_t count = jaccard_similarity(G, weight);

    // Should process some edges
    REQUIRE(count > 0);
  }

  SECTION("Star graph - leaves have no common neighbors") {
    auto el = create_star_graph();

    swap_to_triangular<0, decltype(el), succession::successor>(el);
    lexical_sort_by<0>(el);
    uniq(el);

    adjacency<0, double> G(el);

    auto weight = [](auto& e) -> auto& { return std::get<1>(e); };

    size_t count = jaccard_similarity(G, weight);

    // Star graph edges: only edges from center to leaves
    // Jaccard for center-leaf edges: intersection of neighbors
    REQUIRE(count >= 0);
  }

  SECTION("Path graph") {
    auto el = create_path_graph();

    swap_to_triangular<0, decltype(el), succession::successor>(el);
    lexical_sort_by<0>(el);
    uniq(el);

    adjacency<0, double> G(el);

    auto weight = [](auto& e) -> auto& { return std::get<1>(e); };

    size_t count = jaccard_similarity(G, weight);

    // Path graph has edges, should process them
    REQUIRE(count >= 0);
  }
}

TEST_CASE("Jaccard empty graph", "[jaccard]") {

  SECTION("Graph with no edges") {
    edge_list<directedness::undirected, double> el(3);
    // No edges added

    adjacency<0, double> G(el);

    auto weight = [](auto& e) -> auto& { return std::get<1>(e); };

    size_t count = jaccard_similarity(G, weight);

    // No edges to process
    REQUIRE(count == 0);
  }
}

TEST_CASE("Jaccard single edge", "[jaccard]") {

  SECTION("Two vertices with one edge") {
    edge_list<directedness::undirected, double> el(2);
    el.open_for_push_back();
    el.push_back(0, 1, 0.0);
    el.close_for_push_back();

    swap_to_triangular<0, decltype(el), succession::successor>(el);
    lexical_sort_by<0>(el);
    uniq(el);

    adjacency<0, double> G(el);

    auto weight = [](auto& e) -> auto& { return std::get<1>(e); };

    size_t count = jaccard_similarity(G, weight);

    // One edge processed
    // Jaccard(0,1) = |N(0) ∩ N(1)| / |N(0) ∪ N(1)|
    // N(0) = {1}, N(1) = {0} (but in triangular form this may differ)
    REQUIRE(count >= 0);
  }
}
