/**
 * @file graph_traits_test.cpp
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

#include <type_traits>

#include "common/test_header.hpp"

#include "nwgraph/graph_traits.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

TEST_CASE("Graph traits vertex_id_t", "[graph_traits]") {

  SECTION("Adjacency graph vertex_id_type") {
    using G = adjacency<0>;
    using vid_t = vertex_id_t<G>;

    // vertex_id_t should be the same as G::vertex_id_type
    REQUIRE(std::is_same_v<vid_t, typename G::vertex_id_type>);
  }

  SECTION("CSC graph vertex_id_type") {
    using G = adjacency<1>;
    using vid_t = vertex_id_t<G>;

    REQUIRE(std::is_same_v<vid_t, typename G::vertex_id_type>);
  }
}

TEST_CASE("Graph traits specialization", "[graph_traits]") {

  SECTION("graph_traits has vertex_id_type") {
    using G = adjacency<0>;
    using traits = graph_traits<G>;

    // Check that graph_traits exposes vertex_id_type
    REQUIRE(std::is_same_v<typename traits::vertex_id_type, typename G::vertex_id_type>);
  }
}
