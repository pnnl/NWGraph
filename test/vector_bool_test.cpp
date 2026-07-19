/**
 * @file vector_bool_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2026 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2026 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */


#include <algorithm>
#include <tuple>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/containers/soa.hpp"
#include "nwgraph/edge_list.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;

TEST_CASE("bool attribute columns", "[vector_bool]") {

  SECTION("sorting a struct_of_arrays permutes the vector<bool> column") {
    struct_of_arrays<unsigned, bool> soa;
    soa.push_back(3, true);
    soa.push_back(1, false);
    soa.push_back(2, true);
    soa.push_back(0, false);

    std::sort(soa.begin(), soa.end(), [](auto&& a, auto&& b) { return std::get<0>(a) < std::get<0>(b); });

    std::vector<std::tuple<unsigned, bool>> expected{{0, false}, {1, false}, {2, true}, {3, true}};
    size_t                                  i = 0;
    for (auto&& e : soa) {
      REQUIRE(unsigned(std::get<0>(e)) == std::get<0>(expected[i]));
      REQUIRE(bool(std::get<1>(e)) == std::get<1>(expected[i]));
      ++i;
    }
    REQUIRE(i == expected.size());
  }

  SECTION("writes through the reference tuple reach the underlying bits") {
    struct_of_arrays<unsigned, bool> soa;
    soa.push_back(0, false);

    auto it          = soa.begin();
    std::get<1>(*it) = true;
    REQUIRE(bool(std::get<1>(*soa.begin())) == true);
  }

  SECTION("adjacency can be built from an edge list with a bool attribute") {
    edge_list<directedness::directed, bool> A_list{
        {0, 1, true},
        {1, 2, true},
        {2, 3, false},
        {3, 4, true},
    };

    adjacency<0, bool> A(A_list);
    REQUIRE(num_vertices(A) == 5);

    std::vector<std::tuple<unsigned, unsigned, bool>> flat;
    for (size_t u = 0; u < 5; ++u) {
      for (auto&& e : A[u]) {
        flat.emplace_back(unsigned(u), unsigned(std::get<0>(e)), bool(std::get<1>(e)));
      }
    }

    std::vector<std::tuple<unsigned, unsigned, bool>> expected{
        {0, 1, true},
        {1, 2, true},
        {2, 3, false},
        {3, 4, true},
    };
    REQUIRE(flat == expected);
  }
}
