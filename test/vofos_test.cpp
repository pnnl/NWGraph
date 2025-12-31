/**
 * @file vofos_test.cpp
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

#include "nwgraph/vofos.hpp"

using namespace nw::graph;

TEST_CASE("adj_flist construction", "[vofos]") {

  SECTION("Default construction") {
    adj_flist<0> v(0);
    REQUIRE(v.size() == 0);
  }

  SECTION("Construction with size") {
    adj_flist<0> v(10);
    REQUIRE(v.size() == 10);
  }
}

TEST_CASE("adj_flist push_back", "[vofos]") {

  SECTION("Push back edges dynamically") {
    adj_flist<0> v(3);

    v.open_for_push_back();
    v.push_back(0, 1);
    v.push_back(0, 2);
    v.push_back(1, 2);
    v.close_for_push_back();

    // Vertex 0 should have 2 neighbors
    size_t count = 0;
    for (auto&& neighbor : v[0]) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("Push back expands size") {
    adj_flist<0> v(1);

    v.open_for_push_back();
    v.push_back(5, 0);  // Should expand to size 6
    v.close_for_push_back();

    REQUIRE(v.size() == 6);
  }
}

TEST_CASE("adj_flist iteration", "[vofos]") {

  SECTION("Iterate vertices") {
    adj_flist<0> v(3);

    v.open_for_push_back();
    v.push_back(0, 1);
    v.push_back(0, 2);
    v.push_back(1, 2);
    v.close_for_push_back();

    size_t vertex_count = 0;
    for (auto it = v.begin(); it != v.end(); ++it) {
      ++vertex_count;
    }

    REQUIRE(vertex_count == 3);
  }

  SECTION("Access neighbors") {
    adj_flist<0> v(3);

    v.open_for_push_back();
    v.push_back(0, 1);
    v.push_back(0, 2);
    v.close_for_push_back();

    // Vertex 0 should have neighbors 1 and 2
    size_t neighbor_count = 0;
    for (auto&& neighbor : v[0]) {
      ++neighbor_count;
    }
    REQUIRE(neighbor_count == 2);
  }
}

TEST_CASE("adj_flist num_vertices", "[vofos]") {

  SECTION("num_vertices returns array") {
    adj_flist<0> v(10);
    auto nv = v.num_vertices();
    REQUIRE(nv[0] == 10);
  }
}

TEST_CASE("vector_of_flist_of_structs base", "[vofos]") {

  SECTION("Basic construction") {
    vector_of_flist_of_structs<size_t> v(5);
    REQUIRE(v.size() == 5);
  }

  SECTION("Access inner forward_list") {
    vector_of_flist_of_structs<size_t, double> v(3);
    v[0].emplace_front(1, 2.5);
    v[0].emplace_front(2, 3.5);

    size_t count = 0;
    for (auto&& item : v[0]) {
      ++count;
    }
    REQUIRE(count == 2);
  }
}

TEST_CASE("adj_flist with weighted edges", "[vofos]") {

  SECTION("Weighted edges via push_back") {
    adj_flist<0, double> v(3);

    v.open_for_push_back();
    v.push_back(0, 1, 1.5);
    v.push_back(0, 2, 2.5);
    v.push_back(1, 2, 3.5);
    v.close_for_push_back();

    REQUIRE(v.size() == 3);

    // Check weights
    size_t count = 0;
    for (auto&& [neighbor, weight] : v[0]) {
      ++count;
    }
    REQUIRE(count == 2);
  }
}
