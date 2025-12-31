/**
 * @file bfs.cpp
 *
 * @brief BFS abstraction penalty benchmark.
 *
 * Measures the overhead of various C++ abstractions for BFS traversal:
 * - Raw pointer-based iteration vs iterators vs range-based for
 * - std::get<> vs structured bindings
 * - std::for_each vs range-based for
 * - std::views::elements vs manual tuple access
 * - NWGraph adjacency vs std::vector<std::vector<>>
 * - NWGraph BFS range adaptors
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
#include <ranges>
#include <vector>

#include "apb_common.hpp"
#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/adaptors/bfs_range.hpp"
#include "nwgraph/containers/compressed.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <typename Adjacency>
void run_bfs_benchmarks(Adjacency& graph, size_t ntrial, vertex_id_t<Adjacency> seed) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type    N = num_vertices(graph);
  std::vector<bool> visited(N);

  auto reset = [&] { std::fill(visited.begin(), visited.end(), false); };

  // Baseline: raw pointer-based access
  bench("raw for loop", ntrial, reset, [&] {
    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();

    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto n = ptr[vtx]; n < ptr[vtx + 1]; ++n) {
        if (!visited[idx[n]]) {
          visited[idx[n]] = true;
          Q.push(idx[n]);
        }
      }
    }
  });

  // Iterator-based nested loops
  bench("iterator based for loop", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto n = graph[vtx].begin(); n != graph[vtx].end(); ++n) {
        if (!visited[std::get<0>(*n)]) {
          visited[std::get<0>(*n)] = true;
          Q.push(std::get<0>(*n));
        }
      }
    }
  });

  // Range-based for with std::get<>
  bench("range based for loop", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto n : graph[vtx]) {
        if (!visited[std::get<0>(n)]) {
          visited[std::get<0>(n)] = true;
          Q.push(std::get<0>(n));
        }
      }
    }
  });

  // Range-based for with structured binding
  bench("range for with structured binding", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto&& [n] : graph[vtx]) {
        if (!visited[n]) {
          visited[n] = true;
          Q.push(n);
        }
      }
    }
  });

  // std::for_each
  bench("std::for_each", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      std::for_each(graph[vtx].begin(), graph[vtx].end(), [&](auto&& n) {
        if (!visited[std::get<0>(n)]) {
          visited[std::get<0>(n)] = true;
          Q.push(std::get<0>(n));
        }
      });
    }
  });

  // std::views::elements<0>
  bench("range for with views::elements<0>", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto&& n : std::views::elements<0>(graph[vtx])) {
        if (!visited[n]) {
          visited[n] = true;
          Q.push(n);
        }
      }
    }
  });

  // Build vector<vector<int>> representation for comparison
  std::vector<std::vector<int>> vgraph(N);
  for (vertex_id_type i = 0; i < N; ++i) {
    vgraph[i].reserve(graph[i].size());
    for (auto&& [j] : graph[i]) {
      vgraph[i].push_back(j);
    }
  }

  bench("vector<vector<int>>", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto&& n : vgraph[vtx]) {
        if (!visited[n]) {
          visited[n] = true;
          Q.push(n);
        }
      }
    }
  });

  // Build vector<vector<tuple<int>>> representation for comparison
  std::vector<std::vector<std::tuple<int>>> tgraph(N);
  for (vertex_id_type i = 0; i < N; ++i) {
    tgraph[i].reserve(graph[i].size());
    for (auto&& [j] : graph[i]) {
      tgraph[i].emplace_back(j);
    }
  }

  bench("vector<vector<tuple<int>>>", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto&& t : tgraph[vtx]) {
        auto n = std::get<0>(t);
        if (!visited[n]) {
          visited[n] = true;
          Q.push(n);
        }
      }
    }
  });

  bench("vector<vector<tuple<int>>> + elements<0>", ntrial, reset, [&] {
    std::queue<vertex_id_type> Q;
    Q.push(seed);
    visited[seed] = true;
    while (!Q.empty()) {
      vertex_id_type vtx = Q.front();
      Q.pop();
      for (auto&& n : std::views::elements<0>(tgraph[vtx])) {
        if (!visited[n]) {
          visited[n] = true;
          Q.push(n);
        }
      }
    }
  });

  // NWGraph BFS range adaptors
  bench("topdown_bfs_range", ntrial, reset, [&] {
    for (auto&& j : topdown_bfs_range(graph))
      ;
  });

  bench("bfs_edge_range", ntrial, reset, [&] {
    for (auto&& j : bfs_edge_range(graph))
      ;
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
  run_bfs_benchmarks(graph, args.ntrial, vertex_id_t<decltype(graph)>{0});

  return 0;
}
