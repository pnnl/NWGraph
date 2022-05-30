/**
 * @file vertex_range_eg.cpp
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
#include <queue>

#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

int main(int argc, char* argv[]) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " adj.mmio" << std::endl;
    return -1;
  }

  auto         aos_a = read_mm<directedness::directed>(argv[1]);
  adjacency<0> A(aos_a);
  using vertex_id_type = vertex_id_t<adjacency<0>>;
  vertex_id_type max = A.size();
  std::for_each(counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(max), [](auto i) {
    std::cout << i << " ";
  });
  std::cout << std::endl;

  for (auto& i : vertex_range<adjacency<0>>(A.size()))
    std::cout << i << " ";
  std::cout << std::endl;

  auto range = vertex_range<adjacency<0>>(A.size());
  std::for_each(range.begin(), range.end(), [](auto i) {
    std::cout << i << " ";
  });
  std::cout << std::endl;
  return 0;
}
