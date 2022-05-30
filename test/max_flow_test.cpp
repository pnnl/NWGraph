/**
 * @file max_flow_test.cpp
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
#include <vector>

#include "nwgraph/adaptors/filtered_bfs_range.hpp"
#include "nwgraph/adaptors/reverse.hpp"
#include "nwgraph/algorithms/max_flow.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

#include "common/test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

TEST_CASE("max flow 1", "[MF1]") {
  size_t                                            source = 0, sink = 5;
  edge_list<directedness::directed, double, double> E_list(8);
  E_list.push_back(0, 1, 10, 0);
  E_list.push_back(1, 0, 10, 0);
  E_list.push_back(1, 7, 10, 0);
  E_list.push_back(7, 2, 10, 0);
  E_list.push_back(2, 5, 10, 0);
  E_list.push_back(0, 3, 10, 0);
  E_list.push_back(3, 4, 10, 0);
  E_list.push_back(4, 6, 10, 0);
  E_list.push_back(6, 5, 10, 0);
  E_list.push_back(3, 2, 1, 0);
  adjacency<0, double, double> A(E_list);
  back_edge_range              back(A);
  double                       maxflow = max_flow(back, source, sink);
  REQUIRE(maxflow == 20);
}

TEST_CASE("max flow 2", "[mf2]") {
  size_t source = 0, sink = 7;
  auto   aos_a = read_mm<directedness::directed, double>(DATA_DIR "flowtest.mtx");
  size_t n_vtx = aos_a.size();

  edge_list<directedness::directed, double, double> E_list(n_vtx);
  for (auto y : aos_a) {
    E_list.push_back(std::get<0>(y), std::get<1>(y), std::get<2>(y), 0);
  }

  adjacency<0, double, double> A(E_list);

  back_edge_range back(A);

  double maxflow = max_flow(back, source, sink);
  REQUIRE(maxflow == 29);
}

TEST_CASE("max flow3", "[mf3]") {
  size_t source = 0, sink = 7;

  auto   aos_a = read_mm<directedness::directed, double>(DATA_DIR "flowtest.mtx");
  size_t n_vtx = aos_a.size();

  edge_list<directedness::directed, double, double, double> E_list(n_vtx);
  for (auto y : aos_a) {
    E_list.push_back(std::get<0>(y), std::get<1>(y), std::get<2>(y), 3 * std::get<2>(y), 0);
  }

  adjacency<0, double, double, double> A(E_list);
  back_edge_range                      back(A);

  enum class dict { capacity_idx = 1, flow_idx = 3 };

  double maxflow = max_flow<dict>(back, source, sink);

  enum class dict2 { capacity_idx = 2, flow_idx = 3 };

  double maxflow2 = max_flow<dict2>(back, source, sink);

  REQUIRE(maxflow * 2 == maxflow2);
}

TEST_CASE("max flow 3", "[mf3]") {
  struct tree_edge {
    double* capacity;
    double* flow;
    size_t  predecessor;
  };

  size_t source = 0, sink = 7;

  auto   aos_a = read_mm<directedness::directed, double>(DATA_DIR "flowtest.mtx");
  size_t n_vtx = aos_a.size();

  edge_list<directedness::directed, double, double> A_list(n_vtx);
  for (auto y : aos_a) {
    A_list.push_back(std::get<0>(y), std::get<1>(y), std::get<2>(y), 0);
  }

  adjacency<0, double, double> A(A_list);
  std::vector<tree_edge>       search_tree(n_vtx);
  back_edge_range              back(A);

  double maxflow = 0;

  for (int i = 0; i <= 5; ++i) {
    auto mf_filter = [&back](vertex_id_type vtx, decltype(back)::inner_iterator edge) {
      return std::get<2>(*edge) >= std::get<1>(*edge) + std::get<2>(back.get_back_edge(vtx, edge));
    };
    filtered_bfs::filtered_bfs_edge_range ranges(back, source, sink, mf_filter);
    auto                                  ite = ranges.begin();
    for (; ite != ranges.end(); ++ite) {
      auto v = get_source(ite);
      auto u = get_target(ite);
      auto c = &std::get<1>(*ite);
      auto f = &std::get<2>(*ite);

      search_tree[u].capacity    = c;
      search_tree[u].flow        = f;
      search_tree[u].predecessor = v;
    }

    if (i < 5)
      REQUIRE(ite.found());
    else {
      REQUIRE(!ite.found());
      break;
    }

    double min_cap     = 100000;
    size_t path_length = 0;
    for (auto y : reverse_path(search_tree, sink, source)) {
      min_cap = std::min(min_cap, *(search_tree[std::get<0>(y)].capacity) - *(search_tree[std::get<0>(y)].flow));
      ++path_length;
    }

    maxflow += min_cap;

    if (i < 4) REQUIRE(path_length == 3);

    for (auto y : reverse_path(search_tree, sink, source)) {
      REQUIRE(*(search_tree[std::get<0>(y)].capacity) - *(search_tree[std::get<0>(y)].flow) >= min_cap);
    }

    for (auto y : reverse_path(search_tree, sink, source)) {
      *(search_tree[std::get<0>(y)].flow) += min_cap;
    }
    if (i == 0) {
      for (auto y : reverse_path(search_tree, sink, source)) {
        REQUIRE(*(search_tree[std::get<0>(y)].flow) == min_cap);
      }
    }
  }
  REQUIRE(maxflow == 29);
}
