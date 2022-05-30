/**
 * @file mis_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   Tony Liu
 *   liux238
 *
 */

#include <algorithm>
#include <cassert>
#include <iostream>

#include "nwgraph/graph_traits.hpp"

#include "nwgraph/algorithms/dag_based_mis.hpp"
#include "nwgraph/algorithms/maximal_independent_set.hpp"
#include "nwgraph/containers/aos.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/util.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

typedef adjacency<0> csr_graph;

TEST_CASE("Maximal independent set", "[mis]") {
  /*Read the edgelist*/
  auto aos_a = read_mm<directedness::undirected>(DATA_DIR "coloringData.mmio");

  swap_to_triangular<0, decltype(aos_a), succession::predecessor>(aos_a);
  sort_by<1>(aos_a);
  stable_sort_by<0>(aos_a);

  /*Construct the graph*/
  adjacency<0> A(aos_a);

  size_t              N = A.size();
  std::vector<size_t> independentSet;
  maximal_independent_set(A, independentSet);
  // for (auto v: independentSet)
  //   std::cout << v << " ";
  std::vector<size_t> result = {0, 3, 4, 5, 6};
  std::vector<size_t> result2 = {1, 2, 7};
  CHECK(independentSet == result);

  std::vector<bool> inIndependentSet(N, true);
  dag_based_mis(A, inIndependentSet);

  size_t              i = 0;
  std::vector<size_t> mis2;
  for (auto v : inIndependentSet) {
    if (v == true) {
      mis2.push_back(i);
      //     std::cout << i << " is in independent set" << std::endl;
    }
    i++;
  }
  CHECK_FALSE(mis2 == result);
  CHECK(mis2 == result2);
}
