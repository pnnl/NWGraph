/**
 * @file kcore_test.cpp
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
 * /

#include <algorithm>
#include <vector>

#include "nwgraph/adaptors/new_dfs_range.hpp"
#include "nwgraph/algorithms/k_core.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/volos.hpp"
#include "nwgraph/io/mmio.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("k core", "[k-core]") {
  int              k     = 3;
  size_t           n_vtx = 10;
  std::vector<int> degree(n_vtx, 0);

  edge_list<directedness::undirected> E_list(n_vtx);
  E_list.push_back(0, 1);
  E_list.push_back(1, 2);
  E_list.push_back(2, 3);
  E_list.push_back(3, 4);
  E_list.push_back(4, 5);
  E_list.push_back(6, 7);
  E_list.push_back(7, 8);
  E_list.push_back(8, 9);
  E_list.push_back(2, 6);
  E_list.push_back(3, 7);
  E_list.push_back(4, 8);
  E_list.push_back(5, 9);
  E_list.push_back(2, 7);
  E_list.push_back(3, 6);

  SECTION("adjacency") {
    adjacency<0> A(E_list);
    auto         core   = k_core(A, k);
    auto         filter = std::get<0>(core);
    auto         remain = std::get<1>(core);

    REQUIRE(remain == 4);

    auto first = A.begin();
    auto last  = A.end();
    for (auto G = first; first != last; ++first) {
      for (auto v = (*first).begin(); v != (*first).end(); ++v) {
        Neighbors edge = make_my_pair(first - G, std::get<0>(*v));
        if (filter.find(edge) == filter.end()) degree[std::get<0>(*v)]++;
      }
    }
  }
  SECTION("adj_list") {
    adj_list<0> A(E_list);
    auto        core   = k_core(A, k);
    auto        filter = std::get<0>(core);
    auto        remain = std::get<1>(core);

    REQUIRE(remain == 4);

    auto first = A.begin();
    auto last  = A.end();
    for (auto G = first; first != last; ++first) {
      for (auto v = (*first).begin(); v != (*first).end(); ++v) {
        Neighbors edge = make_my_pair(first - G, std::get<0>(*v));
        if (filter.find(edge) == filter.end()) degree[std::get<0>(*v)]++;
      }
    }
  }

  REQUIRE(degree[0] == 0);
  REQUIRE(degree[1] == 0);
  REQUIRE(degree[2] == 3);
  REQUIRE(degree[3] == 3);
  REQUIRE(degree[4] == 0);
  REQUIRE(degree[5] == 0);
  REQUIRE(degree[6] == 3);
  REQUIRE(degree[7] == 3);
  REQUIRE(degree[8] == 0);
  REQUIRE(degree[9] == 0);
}

TEST_CASE("disconnected", "[disconnected]") {
  int              k     = 3;
  size_t           n_vtx = 9;
  std::vector<int> degree(n_vtx, 0);

  edge_list<directedness::undirected> E_list(n_vtx);
  E_list.push_back(0, 1);
  E_list.push_back(1, 2);
  E_list.push_back(2, 3);
  E_list.push_back(3, 4);
  E_list.push_back(5, 6);
  E_list.push_back(7, 8);
  E_list.push_back(0, 5);
  E_list.push_back(1, 6);
  E_list.push_back(3, 7);
  E_list.push_back(4, 8);
  E_list.push_back(0, 6);
  E_list.push_back(1, 5);
  E_list.push_back(3, 8);
  E_list.push_back(4, 7);

  SECTION("adjacency") {
    adjacency<0> A(E_list);
    auto         core   = k_core(A, k);
    auto         filter = std::get<0>(core);
    auto         remain = std::get<1>(core);

    REQUIRE(remain == 8);

    auto first = A.begin();
    auto last  = A.end();
    for (auto G = first; first != last; ++first) {
      for (auto v = (*first).begin(); v != (*first).end(); ++v) {
        Neighbors edge = make_my_pair(first - G, std::get<0>(*v));
        if (filter.find(edge) == filter.end()) degree[std::get<0>(*v)]++;
      }
    }
  }
  SECTION("adj_list") {
    adj_list<0> A(E_list);
    auto        core   = k_core(A, k);
    auto        filter = std::get<0>(core);
    auto        remain = std::get<1>(core);

    REQUIRE(remain == 8);

    auto first = A.begin();
    auto last  = A.end();
    for (auto G = first; first != last; ++first) {
      for (auto v = (*first).begin(); v != (*first).end(); ++v) {
        Neighbors edge = make_my_pair(first - G, std::get<0>(*v));
        if (filter.find(edge) == filter.end()) degree[std::get<0>(*v)]++;
      }
    }
  }
  REQUIRE(degree[0] == 3);
  REQUIRE(degree[1] == 3);
  REQUIRE(degree[2] == 0);
  REQUIRE(degree[3] == 3);
  REQUIRE(degree[4] == 3);
  REQUIRE(degree[5] == 3);
  REQUIRE(degree[6] == 3);
  REQUIRE(degree[7] == 3);
  REQUIRE(degree[8] == 3);
}
