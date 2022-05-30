/**
 * @file ranges_and_concepts_test.cpp
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


#include <concepts>

#include "containers/aolos.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/vovos.hpp"
#include "nwgraph/graph_concepts.hpp"

#include "common/abstract_test.hpp"

/*template<typename GraphT>
requires AdjacencyGraph<GraphT>
void test_func(GraphT& G) {
  
}*/

TEST_CASE("test", "[test]") {

  adj_list<> A(5);
  A.push_back(0, 1);
  A.push_back(1, 2);
  A.push_back(0, 2);
  A.push_back(1, 3);
  A.push_back(1, 4);
  std::ranges::begin(A);
  for (auto&& out : A) {
    std::cout << "outer" << std::endl;
    for (auto&& in : out) {
      std::cout << "in" << std::endl;
    }
  }
  std::ranges::for_each(
      A, [](auto&& out) { std::ranges::for_each(out, [](auto&& in) { std::cout << "edge to " << std::get<0>(in) << std::endl; }); });
  //test_func(A);
}
