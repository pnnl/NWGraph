/**
 * @file page_rank_test.cpp
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


#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <vector>

#include "nwgraph/algorithms/page_rank.hpp"
#include "nwgraph/experimental/algorithms/page_rank.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/volos.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/timer.hpp"
#include "nwgraph/util/util.hpp"

#include "common/abstract_test.hpp"

using namespace nw::graph;
using namespace nw::util;

size_t const NUM_NODES = 34;

//****************************************************************************
void build_karate_edge_list(edge_list<directedness::directed, double>& edges) {
  std::vector<size_t> const i = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,
                                 2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,
                                 7,  7,  7,  7,  8,  8,  8,  8,  8,  9,  9,  10, 10, 10, 11, 12, 12, 13, 13, 13, 13, 13, 14, 14, 15, 15,
                                 16, 16, 17, 17, 18, 18, 19, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 25, 25, 25,
                                 26, 26, 27, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 32, 32, 32,
                                 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33};

  std::vector<size_t> const j = {1,  2,  3,  4,  5,  6,  7,  8,  10, 11, 12, 13, 17, 19, 21, 31, 0,  2,  3,  7,  13, 17, 19, 21, 30, 0,
                                 1,  3,  7,  8,  9,  13, 27, 28, 32, 0,  1,  2,  7,  12, 13, 0,  6,  10, 0,  6,  10, 16, 0,  4,  5,  16,
                                 0,  1,  2,  3,  0,  2,  30, 32, 33, 2,  33, 0,  4,  5,  0,  0,  3,  0,  1,  2,  3,  33, 32, 33, 32, 33,
                                 5,  6,  0,  1,  32, 33, 0,  1,  33, 32, 33, 0,  1,  32, 33, 25, 27, 29, 32, 33, 25, 27, 31, 23, 24, 31,
                                 29, 33, 2,  23, 24, 33, 2,  31, 33, 23, 26, 32, 33, 1,  8,  32, 33, 0,  24, 25, 28, 32, 33, 2,  8,  14,
                                 15, 18, 20, 22, 23, 29, 30, 31, 33, 8,  9,  13, 14, 15, 18, 19, 20, 22, 23, 26, 27, 28, 29, 30, 31, 32};

  edges.open_for_push_back();
  for (size_t ix = 0; ix < i.size(); ++ix) {
    edges.push_back(i[ix], j[ix], 1.0);
  }
  edges.close_for_push_back();
}

//****************************************************************************
//****************************************************************************
TEST_CASE("PageRank") {
  using RealT = double;

  edge_list<directedness::directed, double> A(0);
  build_karate_edge_list(A);

  // These are answers when a PR threshold of 1.e-7 is used
  std::vector<double> answer = {0.0972041, 0.0529611, 0.0570794,  0.0358651, 0.0220110, 0.0291618, 0.0291618, 0.0244569, 0.0297064,
                                0.0142740, 0.0220110, 0.00955609, 0.0146330, 0.0294733, 0.0144766, 0.0144766, 0.0168304, 0.0145342,
                                0.0144766, 0.0195506, 0.0144766,  0.0145342, 0.0144766, 0.0314621, 0.0210849, 0.0210187, 0.0150202,
                                0.0256012, 0.0195538, 0.0262432,  0.0245261, 0.0370686, 0.0718675, 0.101166};

  SECTION("adjacency") {
    adjacency<1, RealT> graph(A);
    //std::cout << "Number vertices: " << graph.size() << std::endl;
    std::vector<RealT> page_rank(graph.size());

    std::vector<default_vertex_id_type> degrees(graph.size());
    {
      life_timer _("degrees");
      tbb::parallel_for(edge_range(graph), [&](auto&& edges) {
        for (auto&& [i, j] : edges) {
          __atomic_fetch_add(&degrees[j], 1, __ATOMIC_ACQ_REL);
        }
      });
    }

    double const pr_threshold = 1.e-7;
    page_rank_v1(graph, degrees, page_rank, 0.85, pr_threshold, 100);

    // Check the answers
    REQUIRE(answer.size() == page_rank.size());

    double tolerance(0.005);
    for (size_t idx = 0; idx < page_rank.size(); ++idx) {
      //std::cout << "Vertex " << idx << ": PR = " << page_rank[idx] << std::endl;
      REQUIRE(page_rank[idx] == Approx(answer[idx]).epsilon(tolerance));
    }
  }
  SECTION("adj_list") {
    adj_list<0, RealT> graph(A);
    //std::cout << "Number vertices: " << graph.size() << std::endl;
    std::vector<RealT> page_rank(graph.size());

    std::vector<default_vertex_id_type> degrees(graph.size());
    {
      life_timer _("degrees");
      tbb::parallel_for(edge_range(graph), [&](auto&& edges) {
        for (auto&& [i, j] : edges) {
          __atomic_fetch_add(&degrees[j], 1, __ATOMIC_ACQ_REL);
        }
      });
    }

    double const pr_threshold = 1.e-7;
    page_rank_v1(graph, degrees, page_rank, 0.85, pr_threshold, 100);

    // Check the answers
    REQUIRE(answer.size() == page_rank.size());

    double tolerance(0.005);
    for (size_t idx = 0; idx < page_rank.size(); ++idx) {
      //std::cout << "Vertex " << idx << ": PR = " << page_rank[idx] << std::endl;
      REQUIRE(page_rank[idx] == Approx(answer[idx]).epsilon(tolerance));
    }
  }
}
