/**
 * @file neighbor_range_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Tony Liu
 *
 */


#include <iostream>

#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

int main(int argc, char* argv[]) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " adj.mmio " << std::endl;
    return -1;
  }

  auto         aos_a = read_mm<directedness::undirected>(argv[1]);
  adjacency<0> A(aos_a);

  for (auto&& [u, neighbors] : neighbor_range(A)) {
    std::cout << u << ": ";
    for (auto && [v] : neighbors)
        std::cout << v << " ";
    std::cout << std::endl;
  }
  
  
  auto neighborhoods = neighbor_range(A);
  std::for_each(std::execution::seq, neighborhoods.begin(), neighborhoods.end(), [&](auto&& x) {
    auto&& [u, neighbors] = x;
    std::cout << u << ": ";
    for (auto && [v] : neighbors)
        std::cout << v << " ";
    std::cout << std::endl;     
  });
    
  return 0;
}