/**
 * @file bk_test.cpp
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


#include <algorithm>
#include <vector>

#include "nwgraph/adaptors/back_edge_range.hpp"
#include "nwgraph/algorithms/boykov_kolmogorov.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("boykov-kolmogorov max flow", "[bkmf]") {

  auto aos_a = read_mm<directedness::directed, double>(DATA_DIR "bktest1.mtx");

  size_t n_vtx = aos_a.size();

  edge_list<directedness::directed, double, double*> A_list(n_vtx);
  for (auto y : aos_a) {
    A_list.push_back(std::get<0>(y), std::get<1>(y), std::get<2>(y), NULL);
  }

  adjacency<0, double, double*> A(A_list);

  std::vector<double> capacities(n_vtx);

  for (auto it = capacities.begin(); it != capacities.end(); ++it) {
    *it = 1;
  }
  capacities[1] = -1;
  capacities[3] = -1;
  capacities[7] = -1;
  back_edge_range back(A);
  auto            result = bk_maxflow(back, capacities);
  REQUIRE(std::get<0>(result) == 0);
}
