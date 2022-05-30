/**
 * @file jones_plassmann_coloring.hpp
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

/*Implements Jones-Plassmann algorithm for coloring */
#ifndef JONES_PLASSMANN_COLORING_HPP
#define JONES_PLASSMANN_COLORING_HPP

#include "nwgraph/adaptors/dag_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"

#include "nwgraph/algorithms/maximal_independent_set.hpp"

namespace nw {
namespace graph {

/**
 * @brief Graph greedy coloring using maximal independent set.
 * 
 * @tparam Graph Type of graph.  Must meet the requirements of adjacency_list_graph concept.
 * @param A The input graph.
 * @param colors The array of colors of each vertex.
 */
template <adjacency_list_graph Graph>
void jones_plassmann_coloring(Graph& A, std::vector<size_t>& colors) {
  size_t N = num_vertices(A);
  //init every nodes' color to 0
  std::fill(colors.begin(), colors.end(), 0);

  //find the maximal independent set
  std::vector<size_t> independentSet;
  maximal_independent_set(A, independentSet);

  // Function for finding the first
  // missing positive number in an (unsorted) array
  auto firstMissingPositive = []<typename T>(std::vector<T> arr) -> T {
    size_t n = arr.size();
    // Loop to traverse the whole array
    for (size_t i = 0; i < n; ++i) {
      // Loop to check boundary
      // condition and for swapping
      while (1 <= arr[i] && arr[i] <= n && arr[i] != arr[arr[i] - 1]) {
        std::swap(arr[i], arr[arr[i] - 1]);
      }
    }

    // Checking any element which
    // is not equal to i+1
    for (size_t i = 0; i < n; ++i) {
      if (arr[i] != i + 1) {
        return i + 1;
      }
    }

    // Nothing is present return last index
    return n + 1;
  };

  //calculate colors based on the maximal independent set
  std::vector<std::vector<size_t>> neighbor_colors(N);
  for (auto& u : independentSet) {
    //memorize colors of every independent vertex's neighbors
    for (auto it = A[u].begin(); it != A[u].end(); ++it) {
      auto v = std::get<0>(*it);
      neighbor_colors[u].push_back(colors[v]);
    }
    //find the first unused color within every independent vertex's neighbors
    colors[u] = firstMissingPositive(neighbor_colors[u]);
  }
}

}    // namespace graph
}    // namespace nw
#endif    // JONES_PLASSMANN_COLORING_HPP
