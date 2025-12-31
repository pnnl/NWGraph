/**
 * @file build_test.cpp
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
#include <map>

#include "common/test_header.hpp"

#include "nwgraph/build.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

TEST_CASE("Sort by functions", "[build]") {

  SECTION("sort_by<0> sorts by first element") {
    edge_list<directedness::directed> el(4);
    el.open_for_push_back();
    el.push_back(3, 0);
    el.push_back(1, 2);
    el.push_back(2, 1);
    el.push_back(0, 3);
    el.close_for_push_back();

    sort_by<0>(el);

    REQUIRE(std::get<0>(el[0]) == 0);
    REQUIRE(std::get<0>(el[1]) == 1);
    REQUIRE(std::get<0>(el[2]) == 2);
    REQUIRE(std::get<0>(el[3]) == 3);
  }

  SECTION("sort_by<1> sorts by second element") {
    edge_list<directedness::directed> el(4);
    el.open_for_push_back();
    el.push_back(0, 3);
    el.push_back(1, 1);
    el.push_back(2, 2);
    el.push_back(3, 0);
    el.close_for_push_back();

    sort_by<1>(el);

    REQUIRE(std::get<1>(el[0]) == 0);
    REQUIRE(std::get<1>(el[1]) == 1);
    REQUIRE(std::get<1>(el[2]) == 2);
    REQUIRE(std::get<1>(el[3]) == 3);
  }
}

TEST_CASE("Lexical sort", "[build]") {

  SECTION("lexical_sort_by<0>") {
    edge_list<directedness::directed> el(4);
    el.open_for_push_back();
    el.push_back(1, 2);
    el.push_back(1, 1);
    el.push_back(0, 2);
    el.push_back(0, 1);
    el.close_for_push_back();

    lexical_sort_by<0>(el);

    // Should be sorted by (first, second)
    REQUIRE(std::get<0>(el[0]) == 0);
    REQUIRE(std::get<1>(el[0]) == 1);
    REQUIRE(std::get<0>(el[1]) == 0);
    REQUIRE(std::get<1>(el[1]) == 2);
    REQUIRE(std::get<0>(el[2]) == 1);
    REQUIRE(std::get<1>(el[2]) == 1);
  }
}

TEST_CASE("Swap to triangular", "[build]") {

  SECTION("swap_to_triangular predecessor") {
    edge_list<directedness::directed> el(3);
    el.open_for_push_back();
    el.push_back(0, 2);  // 0 < 2, will swap
    el.push_back(1, 0);  // 1 > 0, no swap
    el.push_back(2, 1);  // 2 > 1, no swap
    el.close_for_push_back();

    swap_to_triangular<0>(el, succession::predecessor);

    // After swap, first should be >= second for predecessor
    for (auto&& e : el) {
      REQUIRE(std::get<0>(e) >= std::get<1>(e));
    }
  }

  SECTION("swap_to_triangular successor") {
    edge_list<directedness::directed> el(3);
    el.open_for_push_back();
    el.push_back(2, 0);  // 2 > 0, will swap
    el.push_back(0, 1);  // 0 < 1, no swap
    el.push_back(1, 2);  // 1 < 2, no swap
    el.close_for_push_back();

    swap_to_triangular<0>(el, succession::successor);

    // After swap, first should be <= second for successor
    for (auto&& e : el) {
      REQUIRE(std::get<0>(e) <= std::get<1>(e));
    }
  }
}

TEST_CASE("Uniq function", "[build]") {

  SECTION("Remove duplicates") {
    edge_list<directedness::directed> el(5);
    el.open_for_push_back();
    el.push_back(0, 1);
    el.push_back(0, 1);  // duplicate
    el.push_back(0, 2);
    el.push_back(1, 2);
    el.push_back(1, 2);  // duplicate
    el.close_for_push_back();

    lexical_sort_by<0>(el);
    uniq(el);

    REQUIRE(el.size() == 3);
  }
}

TEST_CASE("Remove self loops", "[build]") {

  SECTION("Remove self loops from edge list") {
    edge_list<directedness::directed> el(4);
    el.open_for_push_back();
    el.push_back(0, 1);
    el.push_back(1, 1);  // self loop
    el.push_back(2, 3);
    el.push_back(3, 3);  // self loop
    el.close_for_push_back();

    remove_self_loops(el);

    REQUIRE(el.size() == 2);
    for (auto&& e : el) {
      REQUIRE(std::get<0>(e) != std::get<1>(e));
    }
  }
}

TEST_CASE("Make index map", "[build]") {

  SECTION("Create index map from strings") {
    std::vector<std::string> vertices = {"A", "B", "C", "D"};
    auto map = make_index_map(vertices);

    REQUIRE(map["A"] == 0);
    REQUIRE(map["B"] == 1);
    REQUIRE(map["C"] == 2);
    REQUIRE(map["D"] == 3);
  }

  SECTION("Create index map from integers") {
    std::vector<int> vertices = {10, 20, 30};
    auto map = make_index_map(vertices);

    REQUIRE(map[10] == 0);
    REQUIRE(map[20] == 1);
    REQUIRE(map[30] == 2);
  }
}

TEST_CASE("Push back fill", "[build]") {

  SECTION("Fill adjacency from edge list") {
    edge_list<directedness::directed> el(4);
    el.open_for_push_back();
    el.push_back(0, 1);
    el.push_back(0, 2);
    el.push_back(1, 2);
    el.push_back(2, 3);
    el.close_for_push_back();

    std::vector<std::vector<size_t>> adj(4);
    push_back_fill(el, adj, true, 0);

    REQUIRE(adj[0].size() == 2);  // 0 -> 1, 2
    REQUIRE(adj[1].size() == 1);  // 1 -> 2
    REQUIRE(adj[2].size() == 1);  // 2 -> 3
    REQUIRE(adj[3].size() == 0);  // 3 -> nothing
  }
}
