/**
 * @file index_soa_eg.cpp
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

#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

int main(int argc, char* argv[]) {

  indexed_struct_of_arrays<size_t, size_t> pre_A(7);

  {
    std::cout << "Nu" << std::endl;
    size_t i = 0;
    for (auto&& x : pre_A) {
      std::cout << "i: " << i++ << std::endl;
      for (auto&& [u] : x) {
        std::cout << "    " << u << std::endl;
      }
    }
  }

  pre_A.open_for_push_back();
  {
    std::cout << "A" << std::endl;
    size_t i = 0;
    for (auto&& x : pre_A) {
      std::cout << "i: " << i++ << std::endl;
      for (auto&& [u] : x) {
        std::cout << "    " << u << std::endl;
      }
    }
  }

  pre_A.push_back(0, 1);
  pre_A.push_back(0, 2);
  pre_A.push_back(0, 3);

  pre_A.push_back(1, 2);
  pre_A.push_back(1, 3);
  pre_A.push_back(1, 4);

  if (false) {
    std::cout << "B" << std::endl;
    size_t i = 0;
    for (auto&& x : pre_A) {
      std::cout << "i: " << i++ << std::endl;
      for (auto&& [u] : x) {
        std::cout << "    " << u << std::endl;
      }
    }
  }

  //  pre_A.push_back(0, 2);
  pre_A.close_for_push_back();
  {
    std::cout << "C" << std::endl;
    size_t i = 0;
    for (auto&& x : pre_A) {
      std::cout << "i: " << i++ << std::endl;
      for (auto&& [u] : x) {
        std::cout << "    " << u << std::endl;
      }
    }
  }

  // Should this work?
  {
    std::cout << "D" << std::endl;
    size_t i = 0;
    for (auto&& x : pre_A) {
      std::cout << "i: " << i++ << std::endl;
      for (auto&& [u] : x) {
        std::cout << "    " << u << std::endl;
      }
    }
  }
  // compressed_sparse<0, undirected> B(aos_a);
  // std::cout << "edge list size of undirected graph is " << aos_a.size() << std::endl;

  return 0;
}
