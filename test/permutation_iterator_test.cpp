/**
 * @file permutation_iterator_test.cpp
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

#include "nwgraph/util/permutation_iterator.hpp"

using namespace nw::graph;

TEST_CASE("Permutation iterator basic", "[permutation_iterator]") {

  SECTION("Identity permutation") {
    std::vector<int> data = {10, 20, 30, 40, 50};

    // Identity permutation: i -> i
    auto identity = [](auto i) { return i; };

    permutation_iterator<std::vector<int>::iterator, decltype(identity)> it(data.begin(), 0, identity);

    REQUIRE(*it == 10);
    ++it;
    REQUIRE(*it == 20);
    ++it;
    REQUIRE(*it == 30);
  }

  SECTION("Reverse permutation") {
    std::vector<int> data = {10, 20, 30, 40, 50};

    // Reverse permutation: i -> (size - 1 - i)
    auto reverse = [&data](auto i) { return data.size() - 1 - i; };

    permutation_iterator<std::vector<int>::iterator, decltype(reverse)> it(data.begin(), 0, reverse);

    REQUIRE(*it == 50);  // index 0 -> data[4]
    ++it;
    REQUIRE(*it == 40);  // index 1 -> data[3]
    ++it;
    REQUIRE(*it == 30);  // index 2 -> data[2]
  }

  SECTION("Custom permutation") {
    std::vector<int> data = {100, 200, 300, 400};
    std::vector<size_t> perm = {2, 0, 3, 1};  // Permutation array

    auto lookup = [&perm](auto i) { return perm[i]; };

    permutation_iterator<std::vector<int>::iterator, decltype(lookup)> it(data.begin(), 0, lookup);

    REQUIRE(*it == 300);  // index 0 -> perm[0]=2 -> data[2]
    ++it;
    REQUIRE(*it == 100);  // index 1 -> perm[1]=0 -> data[0]
    ++it;
    REQUIRE(*it == 400);  // index 2 -> perm[2]=3 -> data[3]
    ++it;
    REQUIRE(*it == 200);  // index 3 -> perm[3]=1 -> data[1]
  }
}

TEST_CASE("Permutation iterator comparison", "[permutation_iterator]") {

  SECTION("Inequality comparison") {
    std::vector<int> data = {1, 2, 3};
    auto identity = [](auto i) { return i; };

    permutation_iterator<std::vector<int>::iterator, decltype(identity)> it1(data.begin(), 0, identity);
    permutation_iterator<std::vector<int>::iterator, decltype(identity)> it2(data.begin(), 0, identity);
    permutation_iterator<std::vector<int>::iterator, decltype(identity)> it3(data.begin(), 1, identity);

    REQUIRE(!(it1 != it2));  // Same position
    REQUIRE(it1 != it3);     // Different positions
  }
}

TEST_CASE("Permutation iterator with doubles", "[permutation_iterator]") {

  SECTION("Double data") {
    std::vector<double> data = {1.1, 2.2, 3.3, 4.4};
    std::vector<size_t> perm = {3, 1, 2, 0};

    auto lookup = [&perm](auto i) { return perm[i]; };

    permutation_iterator<std::vector<double>::iterator, decltype(lookup)> it(data.begin(), 0, lookup);

    REQUIRE(*it == Approx(4.4));
    ++it;
    REQUIRE(*it == Approx(2.2));
  }
}
