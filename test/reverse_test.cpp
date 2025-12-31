/**
 * @file reverse_test.cpp
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

#include "nwgraph/adaptors/reverse.hpp"

using namespace nw::graph;

// Path edge structure similar to what Dijkstra produces
struct path_edge {
  size_t predecessor;
  double distance;
};

TEST_CASE("Reverse path basic", "[reverse]") {

  SECTION("Simple linear path") {
    // Path: 0 -> 1 -> 2 -> 3 (stored as predecessor chain)
    // predecessor[3] = 2, predecessor[2] = 1, predecessor[1] = 0
    std::vector<path_edge> path(4);
    path[0] = {0, 0.0};       // start - predecessor is self
    path[1] = {0, 1.0};       // came from 0
    path[2] = {1, 2.0};       // came from 1
    path[3] = {2, 3.0};       // came from 2

    // Reverse from 3 back to 0
    reverse_path<path_edge, size_t> rev(path, 3, 0);

    std::vector<size_t> visited;
    for (auto [v] : rev) {
      visited.push_back(v);
    }

    // Should visit 3, 2, 1 (stops when it finds 0)
    REQUIRE(visited.size() == 3);
    REQUIRE(visited[0] == 3);
    REQUIRE(visited[1] == 2);
    REQUIRE(visited[2] == 1);
  }

  SECTION("Short path") {
    std::vector<path_edge> path(3);
    path[0] = {0, 0.0};
    path[1] = {0, 1.0};
    path[2] = {1, 2.0};

    reverse_path<path_edge, size_t> rev(path, 2, 0);

    std::vector<size_t> visited;
    for (auto [v] : rev) {
      visited.push_back(v);
    }

    REQUIRE(visited.size() == 2);
    REQUIRE(visited[0] == 2);
    REQUIRE(visited[1] == 1);
  }

  SECTION("Single edge path") {
    std::vector<path_edge> path(2);
    path[0] = {0, 0.0};
    path[1] = {0, 1.0};

    reverse_path<path_edge, size_t> rev(path, 1, 0);

    std::vector<size_t> visited;
    for (auto [v] : rev) {
      visited.push_back(v);
    }

    REQUIRE(visited.size() == 1);
    REQUIRE(visited[0] == 1);
  }
}

TEST_CASE("Reverse path found method", "[reverse]") {

  SECTION("Check found detection") {
    std::vector<path_edge> path(4);
    path[0] = {0, 0.0};
    path[1] = {0, 1.0};
    path[2] = {1, 2.0};
    path[3] = {2, 3.0};

    reverse_path<path_edge, size_t> rev(path, 3, 0);

    REQUIRE(rev.found(0) == true);
    REQUIRE(rev.found(1) == false);
    REQUIRE(rev.found(2) == false);
    REQUIRE(rev.found(3) == false);
  }
}

TEST_CASE("Reverse path with different vertex types", "[reverse]") {

  SECTION("With unsigned int") {
    std::vector<path_edge> path(3);
    path[0] = {0, 0.0};
    path[1] = {0, 1.0};
    path[2] = {1, 2.0};

    reverse_path<path_edge, unsigned int> rev(path, 2u, 0u);

    std::vector<unsigned int> visited;
    for (auto [v] : rev) {
      visited.push_back(v);
    }

    REQUIRE(visited.size() == 2);
  }
}
