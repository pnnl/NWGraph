/**
 * @file back_edge_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   Tony Liu
 *   liux238
 *
 */

#include "nwgraph/adaptors/back_edge_range.hpp"
#include "nwgraph/adaptors/filtered_bfs_range.hpp"
#include "nwgraph/adaptors/reverse.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("back edge", "[back edge]") {

  edge_list<nw::graph::directedness::directed, double, double> E_list(6);
  E_list.push_back(0, 1, 1.0, 0);
  E_list.push_back(1, 2, 2.0, 0);
  E_list.push_back(2, 3, 3.0, 0);
  E_list.push_back(3, 2, 4.0, 0);
  E_list.push_back(3, 4, 5.0, 0);
  E_list.push_back(4, 3, 6.0, 0);
  E_list.push_back(5, 4, 2.0, 0);

  adjacency<0, double, double> A(E_list);

  back_edge_range back(A);

  auto G = back.begin();
  for (auto outer = back.begin(); outer != back.end(); ++outer) {
    for (auto inner = G[outer - G].begin(); inner != G[outer - G].end(); ++inner) {
      REQUIRE(static_cast<size_t>(outer - back.begin()) == std::get<0>(back.get_back_edge(outer - back.begin(), inner)));
    }
  }
}
