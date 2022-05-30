/**
 * @file adjacency_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *
 */


#include <algorithm>
#include <ranges>
#include <vector>

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

int main() {
  size_t n_vtx = 5;

  edge_list<directedness::directed, double> A_list { { 0, 1, 1 }, 
						     { 1, 2, 2 },
						     { 2, 3, 3 },
						     { 3, 4, 4 }
  };

  adjacency<0, double> A(A_list);

  A.stream_indices();
  adjacency<1, double> AT(A_list);
  AT.stream_indices();

  for (auto&& [u, v, w] : make_edge_range<0>(A)) {
    std::cout << "edge " << u << " to " << v << " has weight " << w << std::endl;
  }

  edge_list<directedness::undirected, double> B_list(n_vtx);
  B_list.open_for_push_back();
  B_list.push_back(0, 1, 10);
  B_list.push_back(1, 2, 20);
  B_list.push_back(2, 3, 30);
  B_list.push_back(3, 4, 40);
  B_list.close_for_push_back();
  adjacency<0, double> B(B_list);

  B.stream_indices();

  adjacency<1, double> BT(B_list);
  BT.stream_indices();

  for (auto&& [u, v, w] : make_edge_range<0>(B)) {
    std::cout << "edge " << u << " to " << v << " has weight " << w << std::endl;
  }

  bi_edge_list<directedness::directed, double> C_list { { 0, 1, 1 }, 
						     { 1, 2, 2 },
						     { 2, 3, 3 },
						     { 3, 4, 4 }
  };
  // num_vertices CPO
  assert(4 == num_vertices(C_list, 0));
  assert(5 == num_vertices(C_list, 1));
  // bi_edge_list.num_vertices member function
  assert(4 == C_list.num_vertices()[0]);
  assert(5 == C_list.num_vertices()[1]);

  C_list.stream_stats();

  biadjacency<0, double> C0(C_list);
  C0.stream_indices();
  C0.stream_stats();
  assert(4 == C0.size());
  assert(4 == num_vertices(C0));
  assert(5 == num_vertices(C0, 1));
  assert(4 == C0.num_vertices()[0]);
  assert(5 == C0.num_vertices()[1]);
  biadjacency<1, double> C1(C_list);
  C1.stream_indices();
  C1.stream_stats();
  assert(5 == C1.size());
  assert(5 == num_vertices(C1));
  assert(4 == num_vertices(C1, 1));
  assert(4 == C1.num_vertices()[1]);
  assert(5 == C1.num_vertices()[0]);
  for (auto&& [u, v, w] : make_edge_range<0>(C0)) {
    std::cout << "edge " << u << " to " << v << " has weight " << w << std::endl;
  }
}
