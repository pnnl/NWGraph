/**
 * @file dag_based_mis.hpp
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
 *
 */

#ifndef DAG_BASED_MIS_HPP
#define DAG_BASED_MIS_HPP

#include <iostream>
#include <vector>

#include "nwgraph/adaptors/dag_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"

namespace nw {
namespace graph {

/**
 * Compute maximal independent set, using DAG adaptor.
 *
 * @tparam Graph Type of the input graph.  Must meet requirements of adjacency_list_graph concept.
 * @param A The input graph.
 * @param mis (out) Boolean vector indicating whether corresponding vertex is in the maximal independent set.
 */
template <adjacency_list_graph Graph>
void dag_based_mis(Graph& A, std::vector<bool>& mis) {
  size_t N = A.size();
#ifdef PRINT_DEBUG
  std::cout << "size: " << N << std::endl;
#endif

  std::vector<size_t> degrees(N, 0);
  // std::vector<size_t> colors(N, std::numeric_limits<std::uint32_t>::max());

  std::vector<std::vector<size_t>> predecessor_list(N);
  std::vector<std::vector<size_t>> successor_list(N);
  std::vector<size_t>              degree_list(N, 0);
  /*Need a "Degree range"*/
  for (auto&& [v, deg] : plain_degree_range(A)) {
#ifdef PRINT_DEBUG
    std::cout << "v: " << v + 1 << " deg: " << deg << std::endl;
#endif
    degrees[v] = deg;
  }

  /*plain edge range will be used to calculate predecessor/successors*/
  /*edge_range returns a tuple every time iterated through*/

  for (auto&& [v, u] : edge_range(A)) {
    if (degrees[v] > degrees[u]) {
      successor_list[v].push_back(u);
      predecessor_list[u].push_back(v);
    } else if (degrees[v] < degrees[u]) {
      predecessor_list[v].push_back(u);
      successor_list[u].push_back(v);
    } else {
      if (v > u) {
        predecessor_list[v].push_back(u);
        successor_list[u].push_back(v);
      } else {
        successor_list[v].push_back(u);
        predecessor_list[u].push_back(v);
      }
    }
  }

  for (uint64_t vtx = 0; vtx < N; vtx++) {
    /*Set the color of the roots to be zero.*/
    if (predecessor_list[vtx].size() == 0) {
      mis[vtx] = true;
#ifdef PRINT_DEBUG
      std::cout << vtx + 1 << " is in mis: " << mis[vtx] << std::endl;
#endif
    }
  }

  for (auto&& [v, u, ready_to_process] : dag_range(A, predecessor_list, successor_list)) {
#ifdef PRINT_DEBUG
    std::cout << "V: " << v + 1 << " U:" << u + 1 << std::endl;
#endif
    if (mis[v] == true) mis[u] = false;
  }

  return;
}

}    // namespace graph
}    // namespace nw
#endif    // DAG_BASED_MIS_HPP
