/**
 * @file priority_queue_test.cpp
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

#include <functional>
#include <tuple>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/util/make_priority_queue.hpp"

using namespace nw::graph;

TEST_CASE("Make priority queue basic", "[priority_queue]") {

  SECTION("Min heap with integers") {
    auto pq = make_priority_queue<int>(std::greater<int>{});

    pq.push(5);
    pq.push(2);
    pq.push(8);
    pq.push(1);

    REQUIRE(pq.top() == 1);
    pq.pop();
    REQUIRE(pq.top() == 2);
    pq.pop();
    REQUIRE(pq.top() == 5);
    pq.pop();
    REQUIRE(pq.top() == 8);
  }

  SECTION("Max heap with integers") {
    auto pq = make_priority_queue<int>(std::less<int>{});

    pq.push(5);
    pq.push(2);
    pq.push(8);
    pq.push(1);

    REQUIRE(pq.top() == 8);
    pq.pop();
    REQUIRE(pq.top() == 5);
    pq.pop();
    REQUIRE(pq.top() == 2);
    pq.pop();
    REQUIRE(pq.top() == 1);
  }
}

TEST_CASE("Make priority queue with tuples", "[priority_queue]") {

  SECTION("Priority queue for Dijkstra-like usage") {
    // Tuple of (distance, vertex_id)
    using elem_t = std::tuple<double, size_t>;

    auto compare = [](const elem_t& a, const elem_t& b) {
      return std::get<0>(a) > std::get<0>(b);  // Min heap by distance
    };

    auto pq = make_priority_queue<elem_t>(compare);

    pq.push({3.5, 0});
    pq.push({1.2, 1});
    pq.push({2.7, 2});

    auto [dist1, v1] = pq.top();
    REQUIRE(v1 == 1);
    REQUIRE(dist1 == Approx(1.2));
    pq.pop();

    auto [dist2, v2] = pq.top();
    REQUIRE(v2 == 2);
    REQUIRE(dist2 == Approx(2.7));
    pq.pop();

    auto [dist3, v3] = pq.top();
    REQUIRE(v3 == 0);
    REQUIRE(dist3 == Approx(3.5));
  }
}

TEST_CASE("Make priority queue with custom container", "[priority_queue]") {

  SECTION("Explicit vector container") {
    auto pq = make_priority_queue<double, std::vector<double>>(std::greater<double>{});

    pq.push(1.5);
    pq.push(0.5);
    pq.push(2.5);

    REQUIRE(pq.top() == Approx(0.5));
  }
}

TEST_CASE("Make priority queue empty checks", "[priority_queue]") {

  SECTION("Empty initially") {
    auto pq = make_priority_queue<int>(std::less<int>{});
    REQUIRE(pq.empty());
  }

  SECTION("Not empty after push") {
    auto pq = make_priority_queue<int>(std::less<int>{});
    pq.push(42);
    REQUIRE(!pq.empty());
    REQUIRE(pq.size() == 1);
  }

  SECTION("Empty after pop all") {
    auto pq = make_priority_queue<int>(std::less<int>{});
    pq.push(1);
    pq.push(2);
    pq.pop();
    pq.pop();
    REQUIRE(pq.empty());
  }
}
