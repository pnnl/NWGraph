/**
 * @file zip_test.cpp
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

#include "nwgraph/containers/zip.hpp"

using namespace nw::graph;

TEST_CASE("Zipped basic operations", "[zip]") {

  SECTION("Zip two vectors") {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<double> b = {1.1, 2.2, 3.3, 4.4, 5.5};

    auto z = make_zipped(a, b);

    REQUIRE(z.size() == 5);

    // Access elements
    auto [x, y] = z[0];
    REQUIRE(x == 1);
    REQUIRE(y == Approx(1.1));
  }

  SECTION("Zip iteration") {
    std::vector<int> a = {10, 20, 30};
    std::vector<int> b = {1, 2, 3};

    auto z = make_zipped(a, b);

    std::vector<int> sums;
    for (auto&& [x, y] : z) {
      sums.push_back(x + y);
    }

    REQUIRE(sums.size() == 3);
    REQUIRE(sums[0] == 11);
    REQUIRE(sums[1] == 22);
    REQUIRE(sums[2] == 33);
  }

  SECTION("Zip three vectors") {
    std::vector<int> a = {1, 2};
    std::vector<char> b = {'a', 'b'};
    std::vector<double> c = {0.1, 0.2};

    auto z = make_zipped(a, b, c);

    REQUIRE(z.size() == 2);

    auto [x, y, w] = z[0];
    REQUIRE(x == 1);
    REQUIRE(y == 'a');
    REQUIRE(w == Approx(0.1));
  }

  SECTION("Zip modification") {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {10, 20, 30};

    auto z = make_zipped(a, b);

    // Modify through zip
    for (auto&& [x, y] : z) {
      x *= 2;
      y += 5;
    }

    REQUIRE(a[0] == 2);
    REQUIRE(a[1] == 4);
    REQUIRE(a[2] == 6);
    REQUIRE(b[0] == 15);
    REQUIRE(b[1] == 25);
    REQUIRE(b[2] == 35);
  }
}

TEST_CASE("Zipped iterator operations", "[zip]") {

  SECTION("Iterator comparison") {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};

    auto z = make_zipped(a, b);

    auto it1 = z.begin();
    auto it2 = z.begin();
    auto end = z.end();

    REQUIRE(it1 == it2);
    REQUIRE(it1 != end);
  }

  SECTION("Iterator arithmetic") {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {10, 20, 30, 40, 50};

    auto z = make_zipped(a, b);

    auto it = z.begin();
    auto it2 = it + 2;

    auto [x, y] = *it2;
    REQUIRE(x == 3);
    REQUIRE(y == 30);

    REQUIRE(it2 - it == 2);
  }

  SECTION("Iterator random access") {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};

    auto z = make_zipped(a, b);

    auto it = z.begin();
    auto [x, y] = it[1];

    REQUIRE(x == 2);
    REQUIRE(y == 5);
  }
}

TEST_CASE("Zipped empty and resize", "[zip]") {

  SECTION("Empty zip") {
    std::vector<int> a;
    std::vector<int> b;

    auto z = make_zipped(a, b);

    REQUIRE(z.size() == 0);
  }

  SECTION("Clear") {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};

    auto z = make_zipped(a, b);
    z.clear();

    REQUIRE(z.size() == 0);
    REQUIRE(a.size() == 0);
    REQUIRE(b.size() == 0);
  }

  SECTION("Resize") {
    std::vector<int> a = {1, 2};
    std::vector<int> b = {3, 4};

    auto z = make_zipped(a, b);
    z.resize(5);

    REQUIRE(z.size() == 5);
    REQUIRE(a.size() == 5);
    REQUIRE(b.size() == 5);
  }
}

TEST_CASE("Zipped const access", "[zip]") {

  SECTION("Const iteration") {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};

    const auto z = make_zipped(a, b);

    int sum = 0;
    for (auto&& [x, y] : z) {
      sum += x + y;
    }

    REQUIRE(sum == (1+4) + (2+5) + (3+6));
  }

  SECTION("Const element access") {
    std::vector<int> a = {10, 20};
    std::vector<int> b = {1, 2};

    const auto z = make_zipped(a, b);

    auto [x, y] = z[0];
    REQUIRE(x == 10);
    REQUIRE(y == 1);
  }
}
