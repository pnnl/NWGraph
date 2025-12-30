/**
 * @file edge_list_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   liux238
 *
 */


#define EDGELIST_AOS

#include "common/test_header.hpp"
#include "nwgraph/build.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;
using namespace nw::util;

static constexpr size_t N = 1024;

TEST_CASE("push_back_zz", "[edge_list]") {
  edge_list<nw::graph::directedness::directed, double> A(N);
  for (auto&& j : A) {
    ;
  }
}

TEST_CASE("push_back", "[edge_list]") {
  edge_list<nw::graph::directedness::directed, double> A(N);
  A.push_back(8, 6, 7);
  A.push_back(5, 3, 0);

  REQUIRE(A.size() == 2);

  REQUIRE(std::get<0>(A[(size_t)0]) == 8);
  REQUIRE(std::get<1>(A[0]) == 6);
  REQUIRE(std::get<2>(A[0]) == 7);

  REQUIRE(std::get<0>(A[1]) == 5);
  REQUIRE(std::get<1>(A[1]) == 3);
  REQUIRE(std::get<2>(A[1]) == 0);
}

TEST_CASE("push_back - three tuples", "[edge_list]") {
  edge_list<nw::graph::directedness::directed, double, float> A(N);
  A.push_back(8, 6, 7, 5);
  A.push_back(3, 0, 9, 9);

  REQUIRE(A.size() == 2);

  REQUIRE(std::get<0>(A[0]) == 8);
  REQUIRE(std::get<1>(A[0]) == 6);
  REQUIRE(std::get<2>(A[0]) == 7);
  REQUIRE(std::get<3>(A[0]) == 5);

  REQUIRE(std::get<0>(A[1]) == 3);
  REQUIRE(std::get<1>(A[1]) == 0);
  REQUIRE(std::get<2>(A[1]) == 9);
  REQUIRE(std::get<3>(A[1]) == 9);
}

TEST_CASE("sort", "[edge_list]") {
  edge_list<nw::graph::directedness::directed, double, size_t> A(N);

  A.push_back(1, 2, 3.14, 159);
  A.push_back(3, 3, 3.141, 59);
  A.push_back(2, 1, 3.1415, 9);

  REQUIRE(A.size() == 3);

  SECTION("sort 0") {
    sort_by<0>(A);
    REQUIRE(std::get<0>(A[0]) == 1);
    REQUIRE(std::get<0>(A[1]) == 2);
    REQUIRE(std::get<0>(A[2]) == 3);
  }

  SECTION("sort 1") {
    sort_by<1>(A);
    REQUIRE(std::get<1>(A[0]) == 1);
    REQUIRE(std::get<1>(A[1]) == 2);
    REQUIRE(std::get<1>(A[2]) == 3);
  }
}

TEST_CASE("edge_list directedness", "[edge_list]") {
  SECTION("directed edge list") {
    edge_list<nw::graph::directedness::directed> A(10);
    A.push_back(0, 1);
    A.push_back(1, 2);

    REQUIRE(A.size() == 2);
    REQUIRE(num_vertices(A) == 10);
  }

  SECTION("undirected edge list") {
    edge_list<nw::graph::directedness::undirected> A(10);
    A.push_back(0, 1);
    A.push_back(1, 2);

    REQUIRE(A.size() == 2);
  }
}

TEST_CASE("edge_list iteration", "[edge_list]") {
  edge_list<nw::graph::directedness::directed, double> A(N);
  A.push_back(0, 1, 1.0);
  A.push_back(1, 2, 2.0);
  A.push_back(2, 3, 3.0);

  SECTION("range-based for loop") {
    size_t count = 0;
    double total_weight = 0.0;
    for (auto&& [u, v, w] : A) {
      count++;
      total_weight += w;
    }
    REQUIRE(count == 3);
    REQUIRE(total_weight == Approx(6.0));
  }

  SECTION("iterator access") {
    auto it = A.begin();
    REQUIRE(std::get<0>(*it) == 0);
    REQUIRE(std::get<1>(*it) == 1);
    ++it;
    REQUIRE(std::get<0>(*it) == 1);
    REQUIRE(std::get<1>(*it) == 2);
  }
}

TEST_CASE("edge_list empty", "[edge_list]") {
  edge_list<nw::graph::directedness::directed> A(0);

  REQUIRE(A.size() == 0);
  REQUIRE(A.begin() == A.end());
}

TEST_CASE("edge_list open_close_push_back", "[edge_list]") {
  edge_list<nw::graph::directedness::directed> A(5);

  A.open_for_push_back();
  A.push_back(0, 1);
  A.push_back(1, 2);
  A.push_back(2, 3);
  A.close_for_push_back();

  REQUIRE(A.size() == 3);

  // Verify edges are stored correctly
  size_t idx = 0;
  for (auto&& [u, v] : A) {
    REQUIRE(u == idx);
    REQUIRE(v == idx + 1);
    idx++;
  }
}

TEST_CASE("edge_list lexical_sort", "[edge_list]") {
  edge_list<nw::graph::directedness::directed> A(5);
  A.push_back(2, 1);
  A.push_back(0, 2);
  A.push_back(1, 0);
  A.push_back(0, 1);

  lexical_sort_by<0>(A);

  // After sorting by column 0, edges should be ordered by source
  REQUIRE(std::get<0>(A[0]) == 0);
  REQUIRE(std::get<0>(A[1]) == 0);
  REQUIRE(std::get<0>(A[2]) == 1);
  REQUIRE(std::get<0>(A[3]) == 2);
}

TEST_CASE("edge_list uniq", "[edge_list]") {
  edge_list<nw::graph::directedness::directed> A(5);
  A.push_back(0, 1);
  A.push_back(0, 1);  // duplicate
  A.push_back(1, 2);
  A.push_back(1, 2);  // duplicate
  A.push_back(2, 3);

  lexical_sort_by<0>(A);
  uniq(A);

  REQUIRE(A.size() == 3);
}

TEST_CASE("edge_list num_vertices", "[edge_list]") {
  SECTION("returns construction size") {
    edge_list<nw::graph::directedness::directed> A(100);
    REQUIRE(num_vertices(A) == 100);
  }

  SECTION("zero vertices") {
    edge_list<nw::graph::directedness::directed> A(0);
    REQUIRE(num_vertices(A) == 0);
  }
}
