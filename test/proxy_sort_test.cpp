/**
 * @file proxy_sort_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   liux238
 *
 */


#include <iostream>
#include <vector>

#include "nwgraph/util/proxysort.hpp"

using namespace nw::graph;
using namespace nw::util;

template <typename Vector>
void print_n_ranks(const Vector& page_rank, size_t n) {
  auto perm = proxysort<size_t>(page_rank, std::greater<int>());
  for (size_t i = 0; i < 10; ++i) {
    std::cout << std::to_string(perm[i]) + ": " << std::to_string(page_rank[perm[i]]) << std::endl;
  }
}

int main() {

  std::vector<float> x{3., 1., .14, .15, 9};

  auto perm = proxysort(x);

  return 0;
}
