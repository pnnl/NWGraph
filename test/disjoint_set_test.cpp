/**
 * @file disjoint_set_test.cpp
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
#include <set>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/util/disjoint_set.hpp"

using namespace nw::graph;

// Test the free functions for disjoint set operations
TEST_CASE("Disjoint set free functions", "[disjoint-set]") {

  SECTION("Basic find and union") {
    // Create subsets: each element starts as its own parent with rank 0
    std::vector<std::pair<vertex_id_type, size_t>> subsets(10);
    for (size_t i = 0; i < 10; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    // Initially each element is its own root
    for (vertex_id_type i = 0; i < 10; ++i) {
      REQUIRE(disjoint_find(subsets, i) == i);
    }

    // Union 0 and 1
    disjoint_union(subsets, 0, 1);
    REQUIRE(disjoint_find(subsets, 0) == disjoint_find(subsets, 1));

    // Union 2 and 3
    disjoint_union(subsets, 2, 3);
    REQUIRE(disjoint_find(subsets, 2) == disjoint_find(subsets, 3));

    // 0-1 and 2-3 should still be different sets
    REQUIRE(disjoint_find(subsets, 0) != disjoint_find(subsets, 2));
  }

  SECTION("Chain of unions") {
    std::vector<std::pair<vertex_id_type, size_t>> subsets(5);
    for (size_t i = 0; i < 5; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    // Union all into one set
    disjoint_union(subsets, 0, 1);
    disjoint_union(subsets, 1, 2);
    disjoint_union(subsets, 2, 3);
    disjoint_union(subsets, 3, 4);

    // All should be in the same set
    vertex_id_type root = disjoint_find(subsets, 0);
    for (vertex_id_type i = 1; i < 5; ++i) {
      REQUIRE(disjoint_find(subsets, i) == root);
    }
  }

  SECTION("Union-find combined operation") {
    std::vector<std::pair<vertex_id_type, size_t>> subsets(4);
    for (size_t i = 0; i < 4; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    // First union should return true (elements not in same set)
    REQUIRE(disjoint_union_find(subsets, 0, 1) == true);

    // Second union of same elements should return false (already in same set)
    REQUIRE(disjoint_union_find(subsets, 0, 1) == false);

    // Union with different set should return true
    REQUIRE(disjoint_union_find(subsets, 2, 3) == true);

    // Merging two sets should return true
    REQUIRE(disjoint_union_find(subsets, 0, 2) == true);

    // All now in same set, any union should return false
    REQUIRE(disjoint_union_find(subsets, 1, 3) == false);
  }
}

TEST_CASE("Disjoint set for graph algorithms", "[disjoint-set]") {

  SECTION("Connected components simulation") {
    // Simulate finding connected components
    // Graph: 0-1-2  3-4  5 (three components)
    std::vector<std::pair<vertex_id_type, size_t>> subsets(6);
    for (size_t i = 0; i < 6; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    // Add edges for first component
    disjoint_union(subsets, 0, 1);
    disjoint_union(subsets, 1, 2);

    // Add edge for second component
    disjoint_union(subsets, 3, 4);

    // vertex 5 stays isolated

    // Count distinct components
    std::set<vertex_id_type> components;
    for (vertex_id_type i = 0; i < 6; ++i) {
      components.insert(disjoint_find(subsets, i));
    }

    REQUIRE(components.size() == 3);
  }

  SECTION("Cycle detection for Kruskal's algorithm") {
    // In Kruskal's algorithm, adding edge creates cycle if endpoints
    // are already in same set
    std::vector<std::pair<vertex_id_type, size_t>> subsets(4);
    for (size_t i = 0; i < 4; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    // Build tree: 0-1, 1-2, 2-3
    REQUIRE(disjoint_union_find(subsets, 0, 1) == true);  // Can add 0-1
    REQUIRE(disjoint_union_find(subsets, 1, 2) == true);  // Can add 1-2
    REQUIRE(disjoint_union_find(subsets, 2, 3) == true);  // Can add 2-3

    // Adding 0-3 would create cycle - returns false
    REQUIRE(disjoint_union_find(subsets, 0, 3) == false);
  }
}

TEST_CASE("Disjoint set edge cases", "[disjoint-set]") {

  SECTION("Single element") {
    std::vector<std::pair<vertex_id_type, size_t>> subsets(1);
    subsets[0] = {0, 0};

    REQUIRE(disjoint_find(subsets, 0) == 0);
  }

  SECTION("Two elements") {
    std::vector<std::pair<vertex_id_type, size_t>> subsets(2);
    subsets[0] = {0, 0};
    subsets[1] = {1, 0};

    REQUIRE(disjoint_find(subsets, 0) == 0);
    REQUIRE(disjoint_find(subsets, 1) == 1);

    disjoint_union(subsets, 0, 1);
    REQUIRE(disjoint_find(subsets, 0) == disjoint_find(subsets, 1));
  }

  SECTION("Larger number of elements") {
    const size_t N = 1000;
    std::vector<std::pair<vertex_id_type, size_t>> subsets(N);
    for (size_t i = 0; i < N; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    // Union all into one set
    for (size_t i = 0; i < N - 1; ++i) {
      disjoint_union(subsets, static_cast<vertex_id_type>(i),
                     static_cast<vertex_id_type>(i + 1));
    }

    // All should have same root
    vertex_id_type root = disjoint_find(subsets, 0);
    for (size_t i = 0; i < N; ++i) {
      REQUIRE(disjoint_find(subsets, static_cast<vertex_id_type>(i)) == root);
    }
  }
}

TEST_CASE("Disjoint set merge patterns", "[disjoint-set]") {

  SECTION("Star pattern") {
    // All elements connect to center
    std::vector<std::pair<vertex_id_type, size_t>> subsets(10);
    for (size_t i = 0; i < 10; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    for (vertex_id_type i = 1; i < 10; ++i) {
      disjoint_union(subsets, 0, i);
    }

    vertex_id_type root = disjoint_find(subsets, 0);
    for (vertex_id_type i = 0; i < 10; ++i) {
      REQUIRE(disjoint_find(subsets, i) == root);
    }
  }

  SECTION("Binary tree pattern") {
    // Union in binary tree order
    std::vector<std::pair<vertex_id_type, size_t>> subsets(8);
    for (size_t i = 0; i < 8; ++i) {
      subsets[i] = {static_cast<vertex_id_type>(i), 0};
    }

    // Level 1
    disjoint_union(subsets, 0, 1);
    disjoint_union(subsets, 2, 3);
    disjoint_union(subsets, 4, 5);
    disjoint_union(subsets, 6, 7);

    // Level 2
    disjoint_union(subsets, 0, 2);
    disjoint_union(subsets, 4, 6);

    // Level 3
    disjoint_union(subsets, 0, 4);

    // All should be in same set
    vertex_id_type root = disjoint_find(subsets, 0);
    for (vertex_id_type i = 0; i < 8; ++i) {
      REQUIRE(disjoint_find(subsets, i) == root);
    }
  }
}
