/**
 * @file intersection_size_test.cpp
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

#include <tuple>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/util/intersection_size.hpp"

using namespace nw::graph;

// Helper to create a sorted vector of tuples (like neighbor lists)
auto make_neighbor_list(std::initializer_list<size_t> neighbors) {
  std::vector<std::tuple<size_t>> result;
  for (auto n : neighbors) {
    result.push_back(std::make_tuple(n));
  }
  return result;
}

TEST_CASE("Intersection size basic", "[intersection_size]") {

  SECTION("Complete overlap") {
    auto a = make_neighbor_list({1, 2, 3, 4, 5});
    auto b = make_neighbor_list({1, 2, 3, 4, 5});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 5);
  }

  SECTION("No overlap") {
    auto a = make_neighbor_list({1, 2, 3});
    auto b = make_neighbor_list({4, 5, 6});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 0);
  }

  SECTION("Partial overlap") {
    auto a = make_neighbor_list({1, 2, 3, 4});
    auto b = make_neighbor_list({3, 4, 5, 6});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 2);  // 3 and 4
  }

  SECTION("Single element overlap") {
    auto a = make_neighbor_list({1, 5, 10});
    auto b = make_neighbor_list({2, 5, 11});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 1);  // only 5
  }
}

TEST_CASE("Intersection size edge cases", "[intersection_size]") {

  SECTION("Empty first list") {
    auto a = make_neighbor_list({});
    auto b = make_neighbor_list({1, 2, 3});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 0);
  }

  SECTION("Empty second list") {
    auto a = make_neighbor_list({1, 2, 3});
    auto b = make_neighbor_list({});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 0);
  }

  SECTION("Both empty") {
    auto a = make_neighbor_list({});
    auto b = make_neighbor_list({});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 0);
  }

  SECTION("Single element lists - match") {
    auto a = make_neighbor_list({5});
    auto b = make_neighbor_list({5});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 1);
  }

  SECTION("Single element lists - no match") {
    auto a = make_neighbor_list({5});
    auto b = make_neighbor_list({10});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 0);
  }
}

TEST_CASE("Intersection size with iterators", "[intersection_size]") {

  SECTION("Iterator interface - full ranges") {
    auto a = make_neighbor_list({1, 3, 5, 7, 9});
    auto b = make_neighbor_list({2, 3, 5, 8, 9});

    size_t count = intersection_size(a.begin(), a.end(), b.begin(), b.end());
    REQUIRE(count == 3);  // 3, 5, 9
  }

  SECTION("Iterator interface - partial range first") {
    auto a = make_neighbor_list({1, 3, 5, 7, 9});
    auto b = make_neighbor_list({3, 5, 7});

    // Only use first 3 elements of a (1, 3, 5)
    size_t count = intersection_size(a.begin(), a.begin() + 3, b);
    REQUIRE(count == 2);  // 3, 5
  }
}

TEST_CASE("Intersection size asymmetric", "[intersection_size]") {

  SECTION("First list much larger") {
    auto a = make_neighbor_list({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    auto b = make_neighbor_list({5, 10});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 2);
  }

  SECTION("Second list much larger") {
    auto a = make_neighbor_list({5, 10});
    auto b = make_neighbor_list({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    size_t count = intersection_size(a, b);
    REQUIRE(count == 2);
  }
}
