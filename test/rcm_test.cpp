/**
 * @file rcm_test.cpp
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

#include <execution>
#include <iostream>
#include <unordered_set>

#include "nwgraph/algorithms/triangle_count.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"
#include "rcm.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("Reverse Cuthill-Mckee Ordering", "[rcm]") {
  edge_list<directedness::directed> el(10);
  el.push_back(0, 1);
  el.push_back(0, 6);
  el.push_back(0, 8);
  el.push_back(1, 0);
  el.push_back(1, 4);
  el.push_back(1, 6);
  el.push_back(1, 9);
  el.push_back(2, 4);
  el.push_back(2, 6);
  el.push_back(3, 4);
  el.push_back(3, 5);
  el.push_back(3, 8);
  el.push_back(4, 1);
  el.push_back(4, 2);
  el.push_back(4, 3);
  el.push_back(4, 5);
  el.push_back(4, 9);
  el.push_back(5, 3);
  el.push_back(5, 4);
  el.push_back(6, 0);
  el.push_back(6, 1);
  el.push_back(6, 2);
  el.push_back(7, 8);
  el.push_back(7, 9);
  el.push_back(8, 0);
  el.push_back(8, 3);
  el.push_back(8, 7);
  el.push_back(9, 1);
  el.push_back(9, 4);
  el.push_back(9, 7);

  el.lexical_sort_by<0>();
  auto perm = rcm(el);
  REQUIRE(perm[0] == 8);

  el.relabel(perm);
}
