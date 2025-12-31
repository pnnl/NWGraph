/**
 * @file tag_invoke_test.cpp
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

#include "nwgraph/util/tag_invoke.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

TEST_CASE("tag_invoke traits with nwgraph types", "[tag_invoke]") {

  SECTION("is_tag_invocable with num_vertices_tag and adjacency") {
    edge_list<directedness::directed> edges(4);
    edges.open_for_push_back();
    edges.push_back(0, 1);
    edges.push_back(1, 2);
    edges.push_back(2, 3);
    edges.close_for_push_back();

    adjacency<0> G(edges);

    // num_vertices should be tag-invocable on adjacency
    auto nv = num_vertices(G);
    REQUIRE(nv == 4);
  }

  SECTION("is_tag_invocable_v trait") {
    // Test that the trait correctly detects invocability
    REQUIRE(is_tag_invocable_v<num_vertices_tag, adjacency<0>&>);
  }
}

TEST_CASE("tag_t template", "[tag_invoke]") {

  SECTION("tag_t extracts type from CPO") {
    // tag_t takes a non-type template parameter (auto& CPO)
    using nv_tag = tag_t<num_vertices>;
    REQUIRE(std::is_same_v<nv_tag, num_vertices_tag>);
  }
}

TEST_CASE("DECL_TAG_INVOKE macro", "[tag_invoke]") {

  SECTION("Existing nwgraph tags are usable") {
    // These tags are created via DECL_TAG_INVOKE or similar
    REQUIRE(std::is_class_v<num_vertices_tag>);
    REQUIRE(std::is_class_v<target_tag>);
  }
}
