/**
 * @file kruskal_test.cpp
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

#include <algorithm>
#include <numeric>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/algorithms/kruskal.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;

TEST_CASE("Kruskal's MST algorithm", "[kruskal]") {

  SECTION("MST from file") {
    auto edges = read_mm<directedness::undirected, double>(DATA_DIR "msttest.mtx");

    auto mst = kruskal(edges);

    // Calculate total weight of MST
    double total_weight = 0.0;
    for (auto&& [u, v, w] : mst) {
      total_weight += w;
    }

    // Expected minimum weight from test file
    REQUIRE(total_weight == 39);
  }

  SECTION("Maximum spanning tree from file") {
    auto edges = read_mm<directedness::undirected, double>(DATA_DIR "msttest.mtx");

    auto compare = [](auto t1, auto t2) { return std::get<2>(t1) > std::get<2>(t2); };
    auto mst = kruskal(edges, compare);

    // Calculate total weight of maximum spanning tree
    double total_weight = 0.0;
    for (auto&& [u, v, w] : mst) {
      total_weight += w;
    }

    // Expected maximum weight from test file
    REQUIRE(total_weight == 59);
  }

  SECTION("MST edges have valid weights") {
    auto edges = read_mm<directedness::undirected, double>(DATA_DIR "msttest.mtx");

    auto mst = kruskal(edges);

    // All edges should have positive weights
    for (auto&& [u, v, w] : mst) {
      REQUIRE(w > 0);
    }
  }
}

TEST_CASE("Kruskal with custom comparator", "[kruskal]") {

  SECTION("Custom comparison function") {
    auto edges = read_mm<directedness::undirected, double>(DATA_DIR "msttest.mtx");

    // Use custom comparison for minimum spanning tree
    auto min_compare = [](auto t1, auto t2) {
      return std::get<2>(t1) < std::get<2>(t2);
    };

    auto mst = kruskal(edges, min_compare);

    double total_weight = 0.0;
    for (auto&& [u, v, w] : mst) {
      total_weight += w;
    }

    // Should produce minimum spanning tree
    REQUIRE(total_weight == 39);
  }
}
