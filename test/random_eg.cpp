/**
 * @file random_eg.cpp
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

#include "nwgraph/adaptors/random_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

int main(int argc, char* argv[]) {

  size_t num_points = 8;
  size_t start      = 0;
  size_t seed       = 1980;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <adj.mmio> [ num_points ] [ start ]  [ seed ]" << std::endl;
    return -1;
  }

  auto aos_a = read_mm<directedness::directed>(argv[1]);
  if (argc >= 3) num_points = std::stol(argv[2]);
  if (argc >= 4) start = std::stol(argv[3]);
  if (argc >= 5) seed = std::stol(argv[4]);

  adjacency<0> A(aos_a);

  size_t              N = A.end() - A.begin();
  std::vector<size_t> hist(N);

  for (auto y : random_range(A, num_points, start, seed)) {
    ++hist[y];
  }
  for (auto y : hist) {
    std::cout << y << std::endl;
  }

  return 0;
}
