/**
 * @file dfs.cpp
 *
 * @brief DFS abstraction penalty benchmark.
 *
 * Measures the overhead of various C++ abstractions for DFS traversal:
 * - Raw pointer-based iteration vs iterators vs range-based for
 * - std::get<> vs structured bindings
 * - std::for_each vs range-based for
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
#include <stack>
#include <vector>

#include "apb_common.hpp"
#include "nwgraph/containers/compressed.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <typename Adjacency>
void run_dfs_benchmarks(Adjacency& graph, size_t ntrial, vertex_id_t<Adjacency> seed) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type    N = num_vertices(graph);
  std::vector<bool> visited(N);

  auto reset = [&] { std::fill(visited.begin(), visited.end(), false); };

  // Baseline: raw pointer-based access
  bench("raw for loop", ntrial, reset, [&] {
    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();

    std::stack<vertex_id_type> S;
    S.push(seed);
    while (!S.empty()) {
      vertex_id_type vtx = S.top();
      S.pop();
      if (!visited[vtx]) {
        visited[vtx] = true;
      }

      for (auto n = ptr[vtx]; n < ptr[vtx + 1]; ++n) {
        if (!visited[idx[n]]) {
          S.push(idx[n]);
        }
      }
    }
  });

  // Iterator-based for loop
  bench("iterator based for loop", ntrial, reset, [&] {
    std::stack<vertex_id_type> S;
    S.push(seed);
    while (!S.empty()) {
      vertex_id_type vtx = S.top();
      S.pop();
      if (!visited[vtx]) {
        visited[vtx] = true;
      }

      for (auto n = graph[vtx].begin(); n != graph[vtx].end(); ++n) {
        if (!visited[std::get<0>(*n)]) {
          S.push(std::get<0>(*n));
        }
      }
    }
  });

  // Range-based for with std::get<>
  bench("range based for loop", ntrial, reset, [&] {
    std::stack<vertex_id_type> S;
    S.push(seed);
    while (!S.empty()) {
      vertex_id_type vtx = S.top();
      S.pop();
      if (!visited[vtx]) {
        visited[vtx] = true;
      }

      for (auto n : graph[vtx]) {
        if (!visited[std::get<0>(n)]) {
          S.push(std::get<0>(n));
        }
      }
    }
  });

  // Range-based for with structured binding
  bench("range for with structured binding", ntrial, reset, [&] {
    std::stack<vertex_id_type> S;
    S.push(seed);
    while (!S.empty()) {
      vertex_id_type vtx = S.top();
      S.pop();
      if (!visited[vtx]) {
        visited[vtx] = true;
      }

      for (auto&& [n] : graph[vtx]) {
        if (!visited[n]) {
          S.push(n);
        }
      }
    }
  });

  // std::for_each
  bench("std::for_each", ntrial, reset, [&] {
    std::stack<vertex_id_type> S;
    S.push(seed);
    while (!S.empty()) {
      vertex_id_type vtx = S.top();
      S.pop();
      if (!visited[vtx]) {
        visited[vtx] = true;
      }

      std::for_each(graph[vtx].begin(), graph[vtx].end(), [&](auto&& n) {
        if (!visited[std::get<0>(n)]) {
          S.push(std::get<0>(n));
        }
      });
    }
  });
}

int main(int argc, char* argv[]) {
  // Parse command-line arguments
  Args args(argc, argv);

  // Load graph
  auto el = load_graph<directedness::directed>(args.file);

  if (args.verbose) {
    el.stream_stats();
  }

  // Build adjacency structure
  auto graph = [&] {
    nw::util::life_timer _("build adjacency");
    return adjacency<1>(el);
  }();

  if (args.verbose) {
    graph.stream_stats();
  }
  if (args.debug) {
    graph.stream_indices();
  }

  // Run benchmarks (seed = 0)
  run_dfs_benchmarks(graph, args.ntrial, vertex_id_t<decltype(graph)>{0});

  return 0;
}
