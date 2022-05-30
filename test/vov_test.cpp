/**
 * @file vov_test.cpp
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

#include <complex>

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/vovos.hpp"
#include "nwgraph/util/print_types.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("vector of vector of structures", "[vector_of_vector_of_structs]") {
  SECTION("construct") { vector_of_vector_of_structs A(5); }
}

TEST_CASE("vector of vectors", "[vov]") {
  SECTION("construct") {
    vov<0> A(5);
    A.push_back(3, 4);
    vov<0, double> B(5);
    B.push_back(3, 1, 4.159);
    vov<0, double, std::complex<float>> C(5);
    C.push_back(3, 1, 4.159, {86.7, 5.309});
    vov<1, double, std::complex<float>> D(5);
    D.push_back(3, 1, 4.159, {86.7, 5.309});
  }
  SECTION("edge_list") {
    edge_list<nw::graph::directedness::directed, double> A{{0, 0, 8.0}, {0, 1, 6.7}, {1, 2, 5.3}, {3, 0, 0.9}};
    vov<0, double>                                       B(A);
    std::cout << "edgelist ->vov" << std::endl;
    //    for (auto&& [i, j, v] : make_edge_range<0>(B)) {
    //std::cout << i << " " << j << " " << v << std::endl;
    //}

    std::for_each(B.begin(), B.end(), [&](auto&& j) {
      std::for_each(j.begin(), j.end(), [&](auto&& k) { std::cout << std::get<0>(k) << " " << std::get<1>(k) << std::endl; });
    });
    std::cout << std::endl;
  }
  SECTION("iterate") {
    vov<0, double> A(5);
    A.push_back(0, 0, 3);
    A.push_back(1, 4, 1.5);
    A.push_back(2, 2, 6.9);
    A.push_back(2, 3, 2.9);
    A.push_back(3, 2, 7.8);
    A.push_back(4, 0, 3.9);
    for (auto&& [y] : plain_range(A)) {
      std::cout << y << std::endl;
    }

    for (auto&& [x, y] : make_edge_range(A)) {
      std::cout << x << " " << y << std::endl;
    }
    for (auto&& [x, y, z] : make_edge_range<0>(A)) {
      std::cout << x << " " << y << " " << z << std::endl;
    }
  }
}
