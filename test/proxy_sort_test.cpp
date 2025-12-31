/**
 * @file proxy_sort_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   liux238
 *
 */

#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/util/proxysort.hpp"

using namespace nw::util;

TEST_CASE("Proxysort basic", "[proxysort]") {

  SECTION("Sort with comparator") {
    std::vector<float> x{3.0f, 1.0f, 0.14f, 0.15f, 9.0f};

    auto perm = proxysort<size_t>(x, std::less<float>());

    REQUIRE(perm.size() == 5);
    // After sorting ascending: 0.14, 0.15, 1.0, 3.0, 9.0
    // Indices would be: 2, 3, 1, 0, 4
    REQUIRE(x[perm[0]] <= x[perm[1]]);
    REQUIRE(x[perm[1]] <= x[perm[2]]);
    REQUIRE(x[perm[2]] <= x[perm[3]]);
    REQUIRE(x[perm[3]] <= x[perm[4]]);
  }

  SECTION("Sort descending") {
    std::vector<int> x{3, 1, 4, 1, 5, 9, 2, 6};

    auto perm = proxysort<size_t>(x, std::greater<int>());

    REQUIRE(perm.size() == 8);
    // After sorting descending, first should be largest
    REQUIRE(x[perm[0]] >= x[perm[1]]);
    REQUIRE(x[perm[1]] >= x[perm[2]]);
  }

  SECTION("In-place sort with provided permutation") {
    std::vector<double> x{5.0, 2.0, 8.0, 1.0};
    std::vector<size_t> perm(x.size());

    proxysort(x, perm, std::less<double>());

    REQUIRE(perm.size() == 4);
    REQUIRE(x[perm[0]] == Approx(1.0));
    REQUIRE(x[perm[1]] == Approx(2.0));
    REQUIRE(x[perm[2]] == Approx(5.0));
    REQUIRE(x[perm[3]] == Approx(8.0));
  }
}

TEST_CASE("Proxysort edge cases", "[proxysort]") {

  SECTION("Empty vector") {
    std::vector<int> x;

    auto perm = proxysort<size_t>(x, std::less<int>());

    REQUIRE(perm.size() == 0);
  }

  SECTION("Single element") {
    std::vector<int> x{42};

    auto perm = proxysort<size_t>(x, std::less<int>());

    REQUIRE(perm.size() == 1);
    REQUIRE(perm[0] == 0);
  }

  SECTION("Already sorted") {
    std::vector<int> x{1, 2, 3, 4, 5};

    auto perm = proxysort<size_t>(x, std::less<int>());

    REQUIRE(perm[0] == 0);
    REQUIRE(perm[1] == 1);
    REQUIRE(perm[2] == 2);
    REQUIRE(perm[3] == 3);
    REQUIRE(perm[4] == 4);
  }
}
