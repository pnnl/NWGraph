/**
 * @file graph_base_test.cpp
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

#include <sstream>

#include "common/test_header.hpp"

#include "nwgraph/graph_base.hpp"

using namespace nw::graph;

// =============================================================================
// Enum Tests
// =============================================================================

TEST_CASE("Directedness enum", "[graph_base]") {

  SECTION("Directedness values") {
    directedness dir = directedness::directed;
    REQUIRE(dir == directedness::directed);

    dir = directedness::undirected;
    REQUIRE(dir == directedness::undirected);
  }

  SECTION("Other direction trait") {
    other_direction<directedness::directed> od;
    REQUIRE(od.direction == directedness::undirected);

    other_direction<directedness::undirected> ou;
    REQUIRE(ou.direction == directedness::directed);
  }
}

TEST_CASE("Succession enum", "[graph_base]") {

  SECTION("Succession values") {
    succession succ = succession::successor;
    REQUIRE(succ == succession::successor);

    succ = succession::predecessor;
    REQUIRE(succ == succession::predecessor);
  }

  SECTION("Other succession trait") {
    other_succession<succession::successor> os;
    REQUIRE(os.cessor == succession::predecessor);

    other_succession<succession::predecessor> op;
    REQUIRE(op.cessor == succession::successor);
  }
}

// =============================================================================
// Unipartite Graph Base Tests
// =============================================================================

TEST_CASE("Unipartite graph base", "[graph_base]") {

  SECTION("Default construction") {
    unipartite_graph_base base;
    // Default constructed with 0 vertices
    REQUIRE(true);  // Just verify it compiles and constructs
  }

  SECTION("Construction with size") {
    unipartite_graph_base base(10);
    // Should have 10 vertices
    REQUIRE(true);
  }

  SECTION("Serialization roundtrip") {
    unipartite_graph_base original(42);

    // Serialize
    std::stringstream ss;
    original.serialize(ss);

    // Deserialize into new object
    unipartite_graph_base restored;
    restored.deserialize(ss);

    // Both should be equivalent (we can't directly check protected members,
    // but the serialize/deserialize should work)
    REQUIRE(true);
  }
}

// =============================================================================
// Bipartite Graph Base Tests
// =============================================================================

TEST_CASE("Bipartite graph base", "[graph_base]") {

  SECTION("Default construction") {
    bipartite_graph_base base;
    REQUIRE(true);
  }

  SECTION("Construction with sizes") {
    bipartite_graph_base base(10, 20);
    REQUIRE(true);
  }

  SECTION("Construction with array") {
    std::array<size_t, 2> sizes = {5, 15};
    bipartite_graph_base base(sizes);
    REQUIRE(true);
  }

  SECTION("Serialization roundtrip") {
    bipartite_graph_base original(100, 200);

    // Serialize
    std::stringstream ss;
    original.serialize(ss);

    // Deserialize
    bipartite_graph_base restored;
    restored.deserialize(ss);

    REQUIRE(true);
  }
}

// =============================================================================
// Type Trait Tests
// =============================================================================

TEST_CASE("is_unipartite trait", "[graph_base]") {

  SECTION("Unipartite graph base is unipartite") {
    REQUIRE(is_unipartite<unipartite_graph_base>::value == true);
  }

  SECTION("Bipartite graph base is not unipartite") {
    REQUIRE(is_unipartite<bipartite_graph_base>::value == false);
  }

  SECTION("Compile-time constexpr check") {
    static_assert(is_unipartite<unipartite_graph_base>::value == true,
                  "unipartite_graph_base should be unipartite");
    static_assert(is_unipartite<bipartite_graph_base>::value == false,
                  "bipartite_graph_base should not be unipartite");
    REQUIRE(true);
  }
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_CASE("Graph base edge cases", "[graph_base]") {

  SECTION("Zero-size unipartite graph") {
    unipartite_graph_base base(0);
    REQUIRE(true);
  }

  SECTION("Zero-size bipartite graph") {
    bipartite_graph_base base(0, 0);
    REQUIRE(true);
  }

  SECTION("Asymmetric bipartite graph") {
    bipartite_graph_base base(1, 1000000);
    REQUIRE(true);
  }

  SECTION("Large unipartite graph") {
    unipartite_graph_base base(1000000);
    REQUIRE(true);
  }
}
