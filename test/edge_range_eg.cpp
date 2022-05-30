/**
 * @file edge_range_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *   liux238
 *
 */


#include <iostream>
#include <queue>

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/vovos.hpp"


using namespace nw::graph;
using namespace nw::util;


template <class Graph>
void foo(const Graph& g) {
  
  auto f = make_edge_range(g);
  for (auto&& [u, v] : f) {
    ;
  }

  for (auto&& [u, v] : make_edge_range(g)) {
    ;
  }

}


int main(int argc, char* argv[]) {

  edge_list<nw::graph::directedness::directed> a{{0, 3}, {1, 1}, {2, 4}, {3, 1}, {4, 5}, {3, 9}, {2, 2}, {1, 7}};

  auto adj = adjacency<0>(a);

  edge_list<nw::graph::directedness::directed> b;
  auto                                         edges = make_edge_range(adj);
  for (auto&& e : edges) {
    b.push_back(e);
  }
  b.close_for_push_back();

  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end());

  if (a == b) {
    std::cout << "pass" << std::endl;
  } else {
    std::cout << "fail" << std::endl;
  }

  edge_list<nw::graph::directedness::directed, double> c{{0, 3, 8.6}, {1, 1, 7.5}, {2, 4, 3.0}, {3, 1, 99},
                                                         {4, 5, 3.1}, {3, 9, .04}, {2, 2, 1.5}, {1, 7, 9}};

  auto d = adjacency<0, double>(c);


  auto f = make_edge_range(d);
  for (auto&& [u, v] : f) {
    ;
  }

  auto g = make_edge_range<0>(d);
  for (auto&& [u, v, w] : g) {
    ;
  }

  for (auto&& [u, v, w] : make_edge_range<0>(d)) {
    ;
  }

  for (auto&& [u, v, w] : make_edge_range<0>(d)) {
    ;
  }

  for (auto&& [u, v] : make_edge_range(std::vector<std::vector<std::tuple<size_t>>>{})) {
    ;
  }

  for (auto&& [u, v, w] : make_edge_range<0>(std::vector<std::vector<std::tuple<size_t, double>>>{})) {
    ;
  }

  for (auto&& [u, v, w] : make_edge_range<1>(std::vector<std::list<std::tuple<size_t, double, char>>>{})) {
    ;
  }

  foo(d);

  foo(std::vector<std::vector<std::tuple<int, double>>>{});

  foo(nw::graph::vov<0, double>(0));


  return 0;
}
