/**
 * @file util_test.cpp
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

#include "nwgraph/util/arrow_proxy.hpp"
#include "nwgraph/util/util.hpp"

using namespace nw::graph;

TEST_CASE("Arrow proxy basic", "[arrow_proxy]") {

  SECTION("Arrow proxy with int") {
    arrow_proxy<int> proxy{42};
    REQUIRE(*proxy.operator->() == 42);
  }

  SECTION("Arrow proxy with pair") {
    std::pair<int, double> p{10, 3.14};
    arrow_proxy<std::pair<int, double>> proxy{p};

    REQUIRE(proxy->first == 10);
    REQUIRE(proxy->second == Approx(3.14));
  }

  SECTION("Arrow proxy with tuple") {
    std::tuple<int, int, int> t{1, 2, 3};
    arrow_proxy<std::tuple<int, int, int>> proxy{t};

    REQUIRE(std::get<0>(*proxy.operator->()) == 1);
    REQUIRE(std::get<1>(*proxy.operator->()) == 2);
    REQUIRE(std::get<2>(*proxy.operator->()) == 3);
  }
}

TEST_CASE("Utility functions", "[util]") {

  SECTION("pow2 function") {
    REQUIRE(pow2(0) == 1);
    REQUIRE(pow2(1) == 2);
    REQUIRE(pow2(2) == 4);
    REQUIRE(pow2(3) == 8);
    REQUIRE(pow2(10) == 1024);
  }

  SECTION("ceil_log2 function") {
    REQUIRE(ceil_log2(1) == 0);
    REQUIRE(ceil_log2(2) == 1);
    REQUIRE(ceil_log2(3) == 2);
    REQUIRE(ceil_log2(4) == 2);
    REQUIRE(ceil_log2(5) == 3);
    REQUIRE(ceil_log2(8) == 3);
    REQUIRE(ceil_log2(9) == 4);
  }
}

TEST_CASE("Tuple utilities", "[util]") {

  SECTION("nth_cdr extracts tail of tuple") {
    auto t = std::make_tuple(1, 2.0, 'a', "hello");

    auto tail1 = nth_cdr<1>(t);
    REQUIRE(std::tuple_size_v<decltype(tail1)> == 3);
    REQUIRE(std::get<0>(tail1) == Approx(2.0));

    auto tail2 = nth_cdr<2>(t);
    REQUIRE(std::tuple_size_v<decltype(tail2)> == 2);
    REQUIRE(std::get<0>(tail2) == 'a');
  }

  SECTION("props extracts properties from edge tuple") {
    // Edge tuple: (source, target, weight, label)
    auto edge = std::make_tuple(0ul, 1ul, 3.14, 'x');

    auto properties = props(edge);
    REQUIRE(std::tuple_size_v<decltype(properties)> == 2);
    REQUIRE(std::get<0>(properties) == Approx(3.14));
    REQUIRE(std::get<1>(properties) == 'x');
  }

  SECTION("props on plain edge returns empty tuple") {
    auto edge = std::make_tuple(0ul, 1ul);
    auto properties = props(edge);
    REQUIRE(std::tuple_size_v<decltype(properties)> == 0);
  }
}

TEST_CASE("Counter utility", "[util]") {

  SECTION("Counter increments on assignment") {
    counter c;
    c = 1;
    c = 2;
    c = 3;

    // Counter should have counted 3 assignments
    REQUIRE(static_cast<size_t>(c) == 3);
  }

  SECTION("Counter default starts at zero") {
    counter c;
    REQUIRE(static_cast<size_t>(c) == 0);
  }
}
