/**
 * @file connected_components.hpp
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

#ifndef CONNECTED_COMPONENT_HPP
#define CONNECTED_COMPONENT_HPP

#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/util/atomic.hpp"
#include <iostream>
#include <random>
#include <unordered_map>

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#else
#include <algorithm>
#include <execution>
#endif

namespace nw {
namespace graph {

/**
 * @brief Verifies CC result by performing a BFS from a vertex in each component.
 * Asserts search does not reach a vertex with a different component label.
 * If the graph is directed, it performs the search as if it was undirected.
 * Asserts every vertex is visited (degree-0 vertex should have own label).
 * 
 * @tparam Graph Type of graph. Must meet the requirements of adjacency_list_graph concept.
 * @tparam Graph Type of transpose graph. Must meet the requirements of adjacency_list_graph concept.
 * @tparam Vector Container type for CC labelings.
 * @param graph Input graph.
 * @param xpose Input transpose graph.
 * @param comp CC labelings
 * @return true If the CC labelings are all correct.
 * @return false If the CC labelings are wrong.
 */
template <adjacency_list_graph Graph, adjacency_list_graph Transpose, class Vector>
static bool CCVerifier(const Graph& graph, const Transpose& xpose, Vector&& comp) {
  using NodeID = typename nw::graph::vertex_id_t<std::decay_t<Graph>>;
  std::unordered_map<NodeID, NodeID> label_to_source;
  for (auto&& [n] : plain_range(graph)) {
    label_to_source[comp[n]] = n;
  }
  std::vector<bool>   visited(graph.size() + 1);
  std::vector<NodeID> frontier;
  frontier.reserve(graph.size() + 1);
  for (auto&& [curr_label, source] : label_to_source) {
    frontier.clear();
    frontier.push_back(source);
    visited[source] = true;
    for (auto it = frontier.begin(); it != frontier.end(); it++) {
      NodeID u = *it;
      for (auto&& elt : graph[u]) {
        auto v = target(graph, elt);
        if (comp[v] != curr_label) {
          return false;
        }
        if (!visited[v]) {
          visited[v] = true;
          frontier.push_back(v);
        }
      }
      if (u < xpose.size()) {
        for (auto&& elt : xpose[u]) {
          auto v = target(xpose, elt);
          if (comp[v] != curr_label) {
            return false;
          }
          if (!visited[v]) {
            visited[v] = true;
            frontier.push_back(v);
          }
        }
      }
    }
  }
  NodeID i = 0;
  for (auto&& visited : visited) {
    if (!visited) {
      //return false;
      ++i;
    }
  }
  if (0 < i) {
    std::cout << "unvisited " << i << " " << graph.size() + 1 << " ";
    return false;
  }
  return true;
}

/**
 * @brief link the labels of the root parents of two vertices.
 * The vertex having smaller label will become the parent of vertex having the larger label.
 * This operation is also known as hook operation.
 * 
 * @tparam Vector CC labeling container type.
 * @tparam T CC labeling type.
 * @param u Vertex u.
 * @param v Vertex v.
 * @param comp CC labelings of every vertex.
 */
template <typename Vector, typename T>
static void link(T u, T v, Vector& comp) {
  T p1 = nw::graph::acquire(comp[u]);
  T p2 = comp[v];
  while (p1 != p2) {
    T high   = std::max(p1, p2);
    T low    = p1 + (p2 - high);
    T p_high = comp[high];

    if ((p_high == low) || (p_high == high && comp[high].compare_exchange_strong(high, low))) break;
    p1 = comp[p_high];
    p2 = comp[low];
  }
}

/**
 * @brief path compression.
 * The idea is to flatten the tree of each component.
 * Every leaf will be hooked to the root of the tree.
 * The tree of each component becomes a star at the end of the CC algorithm.
 * 
 * @tparam Execution execution policy type.
 * @tparam Vector CC labeling container type.
 * @param exec Execution policy.
 * @param comp CC labelings.
 */
template <typename Execution, typename Vector>
static void compress(Execution& exec, Vector& comp) {
  std::for_each(exec, counting_iterator(0ul), counting_iterator(comp.size()), [&](auto n) {
    while (comp[n] != comp[comp[n]]) {
      auto foo = nw::graph::acquire(comp[n]);
      auto bar = nw::graph::acquire(comp[foo]);
      nw::graph::release(comp[n], bar);
    }
  });
}

/**
 * @brief Subgraph sampling to find the most commonly appeared labeling 
 * within the sampled subgraph.
 * 
 * @tparam Vector CC labeling container type.
 * @tparam T type of the CC labeling.
 * @param comp CC labelings.
 * @param num_samples Number of vertices in the sampled subgraph.
 * @return T the potential largest intermediate component ID.
 */
template <typename Vector, typename T>
static T sample_frequent_element(const Vector& comp, size_t num_samples = 1024) {
  std::unordered_map<T, int>       counts(32);
  std::mt19937                                  gen;
  std::uniform_int_distribution<T> distribution(0, comp.size() - 1);

  for (size_t i = 0; i < num_samples; ++i) {
    T n = distribution(gen);
    counts[comp[n]]++;
  }

  auto&& [num, count] = *std::max_element(counts.begin(), counts.end(), [](auto&& a, auto&& b) { return std::get<1>(a) < std::get<1>(b); });
  float frac_of_graph = static_cast<float>(count) / num_samples;
  std::cout << "Skipping largest intermediate component (ID: " << num << ", approx. " << int(frac_of_graph * 100) << "% of the graph)\n";
  return num;
}

/**
 * @brief Afforest algorithm based on Shiloach-Vishkin with subgraph sampling to
 * skip the largest intermediate component.  Labels each vertex in the graph
 * with a component identifier.
 * 
 * @tparam Execution execution policy type.
 * @tparam Graph1 Type of input graph.  Must meet requirements of adjacency_list_graph concept.
 * @tparam Graph2 Type of transpose input graph.  Must meet requirements of adjacency_list_graph concept.
 * @param exec Parallel execution policy.
 * @param graph Input graph.
 * @param t_graph Transpose of the input graph.
 * @param neighbor_rounds The number of rounds to do neighborhood subgraph sampling.
 * @return Vector of component labelings.
 */
template <typename Execution, adjacency_list_graph Graph1, adjacency_list_graph Graph2>
static auto afforest(Execution& exec, const Graph1& graph, const Graph2& t_graph, const size_t neighbor_rounds = 2) {
  using vertex_id_type = vertex_id_t<Graph1>;
  std::vector<std::atomic<vertex_id_type>> comp(graph.size() + 1);
  std::for_each(exec, counting_iterator(0ul), counting_iterator(comp.size()), [&](vertex_id_type n) { comp[n] = n; });
  for (size_t r = 0; r < neighbor_rounds; ++r) {
    std::for_each(exec, counting_iterator(0ul), counting_iterator(comp.size()), [&](vertex_id_type u) {
      if (r < graph[u].size()) {
        link(u, std::get<0>(graph[u].begin()[r]), comp);
      }
    });
    compress(exec, comp);
  }

  vertex_id_type c = sample_frequent_element<std::vector<std::atomic<vertex_id_type>>, vertex_id_type>(comp);

  std::for_each(exec, counting_iterator(0ul), counting_iterator(comp.size()), [&](vertex_id_type u) {
    if (comp[u] == c) return;

    if (neighbor_rounds < graph[u].size()) {
      for (auto v = graph[u].begin() + neighbor_rounds; v != graph[u].end(); ++v) {
        link(u, std::get<0>(*v), comp);
      }
    }

    if (t_graph.size() != 0) {
      for (auto&& elt : t_graph[u]) {
        auto v = target(t_graph, elt);
        link(u, v, comp);
      }
    }
  });

  compress(exec, comp);

  return comp;
}

}    // namespace graph
}    // namespace nw
#endif    // CONNECTED_COMPONENT_HPP
