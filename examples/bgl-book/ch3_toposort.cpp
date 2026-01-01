/**
 * @file ch3_toposort.cpp
 *
 * @brief File Dependencies - Topological Sort (BGL Book Chapter 3)
 *
 * This example demonstrates topological sorting using DFS on a makefile
 * dependency graph.
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/adaptors/dfs_range.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;

int main() {
  std::cout << "=== Topological Sort for Makefile Dependencies ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 3" << std::endl << std::endl;

  edge_list<directedness::directed> E = read_mm<directedness::directed>("makefile-dependencies.mmio");
  adjacency<0>                      A(E);

  std::vector<std::string> name;
  std::ifstream            name_in("makefile-target-names.dat");
  while (!name_in.eof()) {
    std::string buffer;
    name_in >> buffer;
    if (!buffer.empty()) {
      name.push_back(buffer);
    }
  }

  std::cout << "DFS traversal order:" << std::endl;
  std::vector<size_t> order;
  for (auto&& j : dfs_range(A, 0)) {
    if (j < name.size()) {
      std::cout << "  " << name[j] << std::endl;
    }
    order.push_back(j);
  }

  return 0;
}
