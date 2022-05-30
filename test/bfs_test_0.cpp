/**
 * @file bfs_test_0.cpp
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


#include <iostream>
#include <queue>

#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/adaptors/bfs_range.hpp"
#include "nwgraph/containers/aos.hpp"

#include "common/abstract_test.hpp"

using namespace nw::graph;
using namespace nw::util;

// #define PRINT_OUT

//****************************************************************************
template <typename EdgeListT>
bool validate(EdgeListT& aos, size_t seed, std::vector<size_t> const& distance, std::vector<size_t> const& predecessor) {
  bool pass(true);

  /// @note Should we parse the orginal mmio file instead?
  for (auto&& [u, v] : aos) {
    int    localdiff = distance[u] - distance[v];
    size_t diff      = abs(localdiff);
#ifdef PRINT_OUT
    std::cout << "Checking edge: " << u << " (" << distance[u] << ", " << predecessor[u] << ") -> " << v << " (" << distance[v] << ", "
              << predecessor[v] << "), diff = " << diff << std::endl;
#endif
    if (diff > 1) {
#ifdef PRINT_OUT
      std::cerr << "ERROR: too far: dist[" << u << "] = " << distance[u] << " too far from dist[" << v << "] = " << distance[v] << std::endl;
#endif
      pass = false;
      break;
    }
  }

  //std::cout << "vertex\tdepth\tparent\tpass-fail" << std::endl;
  for (size_t vid = 0; vid < distance.size(); ++vid) {
    bool passed = ((vid == seed) && (distance[vid] == 0)) || (distance[vid] == (1 + distance[predecessor[vid]]));
    pass &= passed;
    //    std::cout << vid+1
    //            << ":\t" << distance[vid]
    //              << "\t" << predecessor[vid]+1
    //              << "\t" << (passed ? "PASS" : "FAIL") << std::endl;
  }

  return pass;
}

TEST_CASE("BFS traversal", "[bfs]") {

  test_util            util;
  auto                 aos_a = util.generate_directed_aos();
  directed_csr_graph_t A(aos_a);

  size_t              N = A.size();
  std::vector<size_t> distance(N, std::numeric_limits<size_t>::max());
  std::vector<size_t> predecessor(N);
  std::iota(predecessor.begin(), predecessor.end(), 0);
  
  SECTION("default seed") {
    vertex_id_t<directed_csr_graph_t> seed = 0;
    std::fill(distance.begin(), distance.end(), std::numeric_limits<size_t>::max());
    std::iota(predecessor.begin(), predecessor.end(), 0);
    distance[seed]      = 0;
    predecessor[seed]   = 0;
    bfs_edge_range ranges(A, seed);
    auto           ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v         = std::get<0>(*ite);
      auto u         = std::get<1>(*ite);
      if (u == seed) distance[u] = 0;
      else
      distance[u]    = distance[v] + 1;
      predecessor[u] = v;
    }

    REQUIRE(validate(aos_a, seed, distance, predecessor));
  }

  SECTION("another seed") {
    vertex_id_t<directed_csr_graph_t> seed = 1;
    std::fill(distance.begin(), distance.end(), std::numeric_limits<size_t>::max());
    std::iota(predecessor.begin(), predecessor.end(), 0);
    distance[seed]                         = 0;
    bfs_edge_range ranges(A, seed);
    auto           ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v      = std::get<0>(*ite);
      auto u      = std::get<1>(*ite);
      if (u == seed) distance[u] = 0;
      else
      distance[u] = distance[v] + 1;
      predecessor[u] = v;
    }

    REQUIRE(validate(aos_a, seed, distance, predecessor));
  }

#if 0
  SECTION("Top-down BFS default seed using bfs_range") {
    vertex_id_t<directed_csr_graph_t> seed = 0;
    distance[seed]      = 0;
    topdown_bfs_range ranges(A, seed);
    auto      ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v         = std::get<0>(*ite);
      auto u         = std::get<1>(*ite);
      distance[u]    = distance[v] + 1;
      predecessor[u] = v;
    }

    REQUIRE(validate(aos_a, seed, distance, predecessor));
  }
    SECTION("Top-down BFS another seed using bfs_range") {
    vertex_id_t<directed_csr_graph_t> seed = 1;
    distance[seed]      = 0;
    topdown_bfs_range ranges(A, seed);
    auto      ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v         = std::get<0>(*ite);
      auto u         = std::get<1>(*ite);
      distance[u]    = distance[v] + 1;
      predecessor[u] = v;
    }

    REQUIRE(validate(aos_a, seed, distance, predecessor));
  }


  SECTION("Bottom-up BFS default seed using bottomup_bfs_range") {
    vertex_id_t<directed_csr_graph_t> seed = 0;
    distance[seed]      = 0;
    bottomup_bfs_range ranges(A, seed);
    auto      ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v         = std::get<0>(*ite);
      auto parentofv         = std::get<1>(*ite);
      if (v == parentofv) distance[v] = 0;
      else
      distance[v]    = distance[parentofv] + 1;
      predecessor[v] = parentofv;
    }
  
    REQUIRE(validate(aos_a, seed, distance, predecessor));
  }
  
  SECTION("Bottom-up BFS another seed using bottomup_bfs_range") {
    vertex_id_t<directed_csr_graph_t> seed = 1;
    distance[seed]      = 0;
    bottomup_bfs_range ranges(A, seed);
    auto      ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v         = std::get<0>(*ite);
      auto parentofv         = std::get<1>(*ite);
      if (v == parentofv) distance[v] = 0;
      else
      distance[v]    = distance[parentofv] + 1;
      predecessor[v] = parentofv;
    }
    REQUIRE(validate(aos_a, seed, distance, predecessor));
  }
#endif
}

