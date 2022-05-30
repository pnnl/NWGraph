/**
 * @file tc_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *   liux238
 *
 */

#include <iostream>
#include <queue>

#include "nwgraph/algorithms/triangle_count.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/experimental/algorithms/triangle_count.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;


//****************************************************************************
TEST_CASE("triangle counting", "[tc]") {

  auto aos_a = read_mm<directedness::undirected>(DATA_DIR "karate.mtx");
  swap_to_triangular<0>(aos_a, succession::successor);
  lexical_sort_by<0>(aos_a);
  uniq(aos_a);
  adjacency<0> A(num_vertices(aos_a));
  push_back_fill(aos_a, A);


  SECTION("v0") {
    size_t triangles = triangle_count(A);
    std::cout << triangles << " triangles (v0)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v1") {
    size_t triangles = triangle_count_v1(A);
    std::cout << triangles << " triangles (v1)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v2") {
    size_t triangles = triangle_count_v2(A);
    std::cout << triangles << " triangles (v2)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v3") {
    size_t triangles = triangle_count_v3(A);
    std::cout << triangles << " triangles (v3)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v4") {
    size_t triangles = triangle_count(A, 1);
    std::cout << triangles << " triangles (v4)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v5") {
    size_t triangles = triangle_count_v5(A.begin(), A.end());
    std::cout << triangles << " triangles (v5)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v6") {
    size_t triangles = triangle_count_v6(A.begin(), A.end());
    std::cout << triangles << " triangles (v6)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v7") {
    size_t triangles = triangle_count_v7(A);
    std::cout << triangles << " triangles (v7)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v10") {
    size_t triangles = triangle_count_v10(A);
    std::cout << triangles << " triangles (v10)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v12") {
    size_t triangles = triangle_count_v12(A, 1);
    std::cout << triangles << " triangles (v12)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v13") {
    size_t triangles = triangle_count_v13(A, 1);
    std::cout << triangles << " triangles (v13)\n";
    REQUIRE(triangles == 45);
  }

  SECTION("v14") {
    size_t triangles = triangle_count_v14(A);
    std::cout << triangles << " triangles (v14)\n";
    REQUIRE(triangles == 45);
  }
  
  SECTION("v15") {
    size_t triangles = triangle_count_v15(A);
    std::cout << triangles << " triangles (v15)\n";
    REQUIRE(triangles == 45);
  }

}
