/**
 * @file triangular_eg.cpp
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

#include "nwgraph/edge_list.hpp"
#include "nwgraph/build.hpp"

using namespace nw::graph;
using namespace nw::util;

int main() {
  edge_list<directedness::undirected> a{{0, 0}, {1, 0}, {4, 0}, {1, 2}, {1, 1}};
  edge_list<directedness::undirected> b{{0, 0}, {0, 1}, {0, 4}, {2, 1}, {1, 1}};
  edge_list<directedness::undirected> c{{0, 0}, {0, 1}, {0, 4}, {1, 0}, {4, 0}, {2, 1}, {1,2}, {2,3},{3,1}};
  
  std::cout << "Successor (upper)" << std::endl;
  a.stream_edges(std::cout);
  std::cout << std::endl;
  swap_to_triangular<0>(a, succession::successor);
  a.stream_edges(std::cout);
  std::cout << std::endl;
  
  std::cout << "Predecessor (lower)" << std::endl;
  b.stream_edges(std::cout);
  std::cout << std::endl;
  swap_to_triangular<0>(b, succession::predecessor);
  b.stream_edges(std::cout);
  std::cout << std::endl;
  
  std::cout << "Successor (upper)" << std::endl;
  c.stream_edges(std::cout);
  std::cout << std::endl;
  swap_to_triangular<0>(c, succession::successor);
  c.stream_edges(std::cout);
  std::cout << std::endl;
  
  std::cout << "Sort" << std::endl;
  lexical_sort_by<0>(c);
  c.stream_edges(std::cout);
  std::cout << std::endl;
  
  std::cout << "Uniq" << std::endl;
  uniq(c);
  c.stream_edges(std::cout);
  std::cout << std::endl;

  return 0;
}
