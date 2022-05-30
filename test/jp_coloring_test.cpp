/**
 * @file jp_coloring_test.cpp
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

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <list>

#include "nwgraph/algorithms/jones_plassmann_coloring.hpp"
#include "nwgraph/containers/aos.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/util.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

typedef adjacency<0> csr_graph;

TEST_CASE("Jones-Plassmann Coloring", "[jp]") {

  /* Read the edgelist */
  auto aos_a = read_mm<directedness::undirected>(DATA_DIR "coloringData.mmio");

  swap_to_triangular<0>(aos_a, succession::predecessor);
  sort_by<1>(aos_a);
  stable_sort_by<0>(aos_a);

  /* Construct the graph */
  adjacency<0> A(aos_a);

  size_t              N = A.size();
  std::vector<size_t> colors(N, std::numeric_limits<std::uint32_t>::max());

  std::vector<size_t> result = {0, 1, 1, 0, 0, 0, 0, 1};
  std::vector<size_t> result2 = {1, 0, 0, 1, 1, 1, 1, 0};
  jones_plassmann_coloring(A, colors);
  REQUIRE((colors == result || colors == result2));
}
