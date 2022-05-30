/**
 * @file new_dfs_test.cpp
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

#include <algorithm>
#include <vector>

#include "nwgraph/adaptors/new_dfs_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("New Depth First Search", "[DFS]") {
  size_t n_vtx = 15;

  edge_list<directedness::undirected> A_list(n_vtx);
  A_list.push_back(0, 1);
  A_list.push_back(1, 2);
  A_list.push_back(2, 3);
  A_list.push_back(3, 4);
  A_list.push_back(4, 5);
  A_list.push_back(0, 6);
  A_list.push_back(6, 7);
  A_list.push_back(7, 8);
  A_list.push_back(8, 9);
  A_list.push_back(0, 10);
  A_list.push_back(10, 11);
  A_list.push_back(11, 12);
  A_list.push_back(12, 13);
  A_list.push_back(13, 14);
  A_list.push_back(14, 11);

  adjacency<0> A(A_list);

  dfs_edge_range ranges(A, 7);

  auto ite = ranges.begin();
  for (; ite != ranges.end(); ++ite) {
    auto u = std::get<0>(*ite);
    auto v = std::get<1>(*ite);
    auto w = std::get<2>(*ite);
    if (u)
      std::cout << "traverse " << v << " to " << w << std::endl;
    else
      std::cout << "view " << v << " to " << w << std::endl;
  }
}
