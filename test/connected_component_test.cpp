/**
 * @file connected_component_test.cpp
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


#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <list>

#include "nwgraph/algorithms/connected_components.hpp"
#include "nwgraph/experimental/algorithms/connected_components.hpp"
#include "nwgraph/containers/aos.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/util.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

using csr_graph = adjacency<0>;

TEST_CASE("connected component", "[cc]") {
  /* Read the edgelist */
  auto aos_a = read_mm<nw::graph::directedness::directed>(DATA_DIR "coloringData.mmio");
  // aos_a.triangularize<predecessor>();
  // aos_a.sort_by<1>();
  // aos_a.stable_sort_by<0>();

  /*Construct the graph*/
  csr_graph A(aos_a);

  size_t                              N = A.size();
  std::vector<vertex_id_t<csr_graph>> component_ids(N, std::numeric_limits<vertex_id_t<csr_graph>>::max());

  SECTION("v1 cc_push") {
    component_ids = compute_connected_components_v1(A);
    for (auto component_id : component_ids) {
      //std::cout << component_id << std::endl;
      REQUIRE(component_id == 0);
    }
  }

  SECTION("v2 cc_pull") {
    component_ids = compute_connected_components_v2(A);
    for (auto component_id : component_ids) {
      //std::cout << component_id << std::endl;
      REQUIRE(component_id == 0);
    }
  }
  SECTION("v5 cc_pull + subgraph sampling") {
    component_ids = ccv5(A);
    for (auto component_id : component_ids) {
      //std::cout << component_id << std::endl;
      REQUIRE(component_id == 0);
    }
  }
  SECTION("v6 sv") {
    component_ids = sv_v6(A);
    for (auto component_id : component_ids) {
      //std::cout << component_id << std::endl;
      REQUIRE(component_id == 0);
    }
  }
  SECTION("v8 sv") {
    component_ids = sv_v8(A);
    for (auto component_id : component_ids) {
      //std::cout << component_id << std::endl;
      REQUIRE(component_id == 0);
    }
  }
}
