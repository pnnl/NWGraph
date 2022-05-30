/**
 * @file volos_test.cpp
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

#include <complex>

#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/volos.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("array of list of structures", "[array_of_list_of_structs]") {
  SECTION("construct") { vector_of_list_of_structs A(5); }
}

TEST_CASE("adjacency list", "[adj_list]") {
  SECTION("construct") {
    adj_list<0> A(5);
    A.push_back(3, 4);
    adj_list<0, double> B(5);
    B.push_back(3, 1, 4.159);
    adj_list<0, double, std::complex<float>> C(5);
    C.push_back(3, 1, 4.159, {86.7, 5.309});
  }

  SECTION("traverse") {
    adj_list<0, int> A(6);
    A.push_back(0, 1, 1);
    A.push_back(1, 2, 2);
    A.push_back(2, 3, 3);
    A.push_back(3, 4, 4);
    A.push_back(4, 5, 5);
    for (auto outer = A.begin(); outer != A.end(); ++outer) {
      for (auto inner = (*outer).begin(); inner != (*outer).end(); ++inner) {
        auto v = std::get<0>(*inner);
      }
    }

    bfs_edge_range range(A, 0);
    for (auto ite = range.begin(); ite != range.end(); ++ite) {
      auto v = std::get<0>(*ite);
    }
  }
}
