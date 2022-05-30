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

#include "nwgraph/build.hpp"
#include "common/test_header.hpp"
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

#if 0

  array_of_structs<size_t> A;
  array_of_structs<size_t, size_t> B;
  array_of_structs<size_t, size_t, double> C;
  array_of_structs<double, size_t, size_t, double> D;

  A.push_back(8675309);
  B.push_back(867, 5309);
  C.push_back(867, 5309, 3.14159);
  D.push_back(95141.3, 867, 5309, 3.14159);


  struct_of_arrays<size_t> E;
  struct_of_arrays<size_t, size_t> F;
  struct_of_arrays<size_t, size_t, double> G;
  struct_of_arrays<double, size_t, size_t, double> H;

  E.push_back(8675309);
  F.push_back(867, 5309);
  G.push_back(867, 5309, 3.14159);
  H.push_back(95141.3, 867, 5309, 3.14159);

  H.clear();
  D.clear();



  sparse_aos<directedness::undirected> A(N, N);
  A.push_back(1, 2);

  sparse_aos<directedness::undirected, double> B(N, N);
  B.push_back(1, 2, 3.14);

  sparse_aos<directedness::undirected, double, size_t> C(N, N);
  C.push_back(1, 2, 3.14, 159);
  C.push_back(3, 3, 3.141, 59);
  C.push_back(2, 1, 3.1415, 9);

  std::cout << std::get<0>(C[0]) << " "<< std::get<1>(C[0]) << " " << std::get<2>(C[0]) << " " << std::get<3>(C[0]) << std::endl;
  std::cout << std::get<0>(C[1]) << " "<< std::get<1>(C[1]) << " " << std::get<2>(C[1]) << " " << std::get<3>(C[1]) << std::endl;
  std::cout << std::get<0>(C[2]) << " "<< std::get<1>(C[2]) << " " << std::get<2>(C[2]) << " " << std::get<3>(C[2]) << std::endl;

  C.sort_by<0>();

  std::cout << std::endl;
  std::cout << std::get<0>(C[0]) << " "<< std::get<1>(C[0]) << " " << std::get<2>(C[0]) << " " << std::get<3>(C[0]) << std::endl;
  std::cout << std::get<0>(C[1]) << " "<< std::get<1>(C[1]) << " " << std::get<2>(C[1]) << " " << std::get<3>(C[1]) << std::endl;
  std::cout << std::get<0>(C[2]) << " "<< std::get<1>(C[2]) << " " << std::get<2>(C[2]) << " " << std::get<3>(C[2]) << std::endl;

  C.sort_by<1>();

  std::cout << std::endl;
  std::cout << std::get<0>(C[0]) << " "<< std::get<1>(C[0]) << " " << std::get<2>(C[0]) << " " << std::get<3>(C[0]) << std::endl;
  std::cout << std::get<0>(C[1]) << " "<< std::get<1>(C[1]) << " " << std::get<2>(C[1]) << " " << std::get<3>(C[1]) << std::endl;
  std::cout << std::get<0>(C[2]) << " "<< std::get<1>(C[2]) << " " << std::get<2>(C[2]) << " " << std::get<3>(C[2]) << std::endl;
#endif
