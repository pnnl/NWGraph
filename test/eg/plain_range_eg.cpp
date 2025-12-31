/**
 * @file plain_range_eg.cpp
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


#include <iostream>
#include <queue>

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

int main(int argc, char* argv[]) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " adj.mmio [ num_points ] [ seed ]" << std::endl;
    return -1;
  }

  auto         aos_a = read_mm<directedness::directed>(argv[1]);
  adjacency<0> A(aos_a);

  for (auto&& [y] : plain_range(A)) {
    std::cout << y << std::endl;
  }

  for (auto&& [x, y] : edge_range(A)) {
    std::cout << x << " " << y << std::endl;
  }

  return 0;
}
