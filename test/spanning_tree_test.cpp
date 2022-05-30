/**
 * @file spanning_tree_test.cpp
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

#include "nwgraph/algorithms/kruskal.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("max/min weight spanning tree", "[mst]") {
  auto                                        A_list = read_mm<directedness::undirected, double>(DATA_DIR "msttest.mtx");
  //edge_list<directedness::undirected, double> A_list(aos_a.size());
  //for (auto y : aos_a) {
  //  A_list.push_back(std::get<0>(y), std::get<1>(y), std::get<2>(y));
  //}

  SECTION("min weight") {
    edge_list<directedness::undirected, double> T_list = kruskal(A_list);

    double totalweight = 0.0;
    for (auto y : T_list) {
      totalweight += std::get<2>(y);
    }

    REQUIRE(totalweight == 39);
  }

  SECTION("max weight") {
    auto                                        compare = [](auto t1, auto t2) { return std::get<2>(t1) > std::get<2>(t2); };
    edge_list<directedness::undirected, double> T_list  = kruskal(A_list, compare);

    double totalweight = 0.0;
    for (auto y : T_list) {
      totalweight += std::get<2>(y);
    }

    REQUIRE(totalweight == 59);
  }
}
