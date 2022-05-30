/**
 * @file dijkstra_eg.cpp
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

#include "nwgraph/algorithms/dijkstra.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

//****************************************************************************
int main(int argc, char* argv[]) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " adj.mmio " << std::endl;
    return -1;
  }

  edge_list<directedness::directed, size_t> aos_a = read_mm<directedness::directed, size_t>(argv[1]);
  adjacency<0, size_t>                      A(aos_a);

  //  auto distance = dijkstra<size_t>(A, 0);

  //  for (auto d : distance)
  //    std::cout << d << std::endl;

  auto distance_v0 = dijkstra<size_t>(A, 0);

  for (auto d : distance_v0)
    std::cout << d << std::endl;

  return 0;
}
