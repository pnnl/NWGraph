/**
 * @file mmio_test.cpp
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


#include "nwgraph/containers/compressed.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

// TODO: Need to add proper catch asserts to verify results

TEST_CASE("constructing graphs using mmio", "[mmio]") {

  SECTION("reading an undirected graph") {
    edge_list<directedness::undirected> A = read_mm<directedness::undirected>(DATA_FILE);
    A.stream();
  }
  SECTION("reading a directed graph") {
    edge_list<directedness::directed> B = read_mm<directedness::directed>(DATA_FILE);
    B.stream();
  }
  SECTION("undirected with attributes") {
    edge_list<directedness::undirected, double> C = read_mm<directedness::undirected, double>(DATA_FILE);
    C.stream();
  }
  SECTION("directed with attributes") {
    edge_list<directedness::directed, double> D = read_mm<directedness::directed, double>(DATA_FILE);
    D.stream();
  }
}

TEST_CASE("constructing graphs using mmio-testing with auto", "[mmio]") {

  SECTION("reading an undirected graph") {
    auto A = read_mm<directedness::undirected>(DATA_FILE);
    A.stream();
  }
  SECTION("reading a directed graph") {
    auto B = read_mm<directedness::directed>(DATA_FILE);
    B.stream();
  }
  SECTION("undirected with attributes") {
    auto C = read_mm<directedness::undirected, double>(DATA_FILE);
    C.stream();
  }
  SECTION("directed with attributes") {
    auto D = read_mm<directedness::directed, double>(DATA_FILE);
    D.stream();
  }
}

#if 0
TEST_CASE("constructing graphs using par_mmio", "[parmmio]") {

  SECTION("reading an undirected graph") {
    edge_list<directedness::undirected> A = par_read_mm<directedness::undirected>(DATA_FILE, true, 2);
    A.stream();
  }
  SECTION("reading a directed graph") {
    edge_list<directedness::directed> B = par_read_mm<directedness::directed>(DATA_FILE, true, 2);
    B.stream();
  }
  SECTION("undirected with attributes") {
    edge_list<directedness::undirected, double> C = par_read_mm<directedness::undirected, double>(DATA_FILE, true, 2);
    C.stream();
  }
  SECTION("directed with attributes") {
    edge_list<directedness::directed, double> D = par_read_mm<directedness::directed, double>(DATA_FILE, true, 2);
    D.stream();
  }
}
#endif

TEST_CASE("writing graphsusing mmio", "[mmio]") {
  edge_list<directedness::directed, double> A(5);
  A.push_back(0, 1, 1);
  A.push_back(1, 2, 2);
  A.push_back(2, 3, 3);
  A.push_back(3, 4, 4);
  write_mm<2, double>("data/mmout_test.mtx", A);

  edge_list<directedness::undirected, double> B(5);
  B.push_back(0, 1, 1);
  B.push_back(1, 2, 2);
  B.push_back(2, 3, 3);
  B.push_back(3, 4, 4);
  write_mm<2, double>("data/mmout_test2.mtx", B);
  write_mm<2, double>("data/mmout_test3.mtx", B, "symmetric");

  edge_list<directedness::undirected, int, double> C(5);
  C.push_back(0, 1, 1, 5);
  C.push_back(1, 2, 2, 6);
  C.push_back(2, 3, 3, 7);
  C.push_back(3, 4, 4, 8);
  write_mm<2, int>("data/mmout_test4.mtx", C);
  write_mm<3, double>("data/mmout_test5.mtx", C);
  write_mm("data/mmout_test5.mtx", C);

#if 0
  edge_list<directedness::undirected> D(5);
  D.push_back(0, 1);
  D.push_back(1, 2);
  D.push_back(2, 3);
  D.push_back(3, 4);
  write_mm<directedness::undirected>("data/mmout_test6.mtx", D);
#endif

  adjacency<0, double>      A_A(A);
  adjacency<0, double>      A_B(B);
  adjacency<0, int, double> A_C(C);
  write_mm<1, double>("data/mmout_test7.mtx", A_A);
  write_mm<1, double>("data/mmout_test8.mtx", A_B);
}
