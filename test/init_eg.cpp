/**
 * @file init_eg.cpp
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
#include "karate.hpp"

using namespace nw::graph;
using namespace nw::util;

int main() {

  {
    edge_list<directedness::undirected> a{{0, 1}, {1, 2}};
    edge_list<directedness::directed>   b{{0, 2}, {1, 0}, {0, 1}};

    a.stream_edges(std::cout);
    std::cout << std::endl;
    b.stream_edges(std::cout);
    std::cout << std::endl;
  }

  {
    edge_list<directedness::undirected, double> a{{0, 1, 3.1}, {1, 2, 4.159}};
    edge_list<directedness::directed, double>   b{{0, 2, 42.2}, {1, 0, 24.4}, {0, 1, 81.9}};


    a.stream_edges(std::cout);
    std::cout << std::endl;
    b.stream_edges(std::cout);
    std::cout << std::endl;
  }

  return 0;
}
