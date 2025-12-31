/**
 * @file dijkstra.cpp
 *
 * @brief Dijkstra abstraction penalty benchmark.
 *
 * Measures the overhead of various property access patterns in Dijkstra:
 * - Direct property access via std::get<>
 * - Property access via lambda function
 * - Target vertex access via CPO (customization point object)
 * - Combinations of the above
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

#include <iostream>
#include <limits>
#include <queue>
#include <vector>

#include "apb_common.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/graph_concepts.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <typename Adjacency, class WeightFn>
void run_dijkstra_benchmarks(Adjacency& graph, size_t ntrial, vertex_id_t<Adjacency> source, WeightFn weight_fn) {
  using vertex_id_type  = vertex_id_t<Adjacency>;
  using weight_type     = std::tuple_element_t<1, inner_value_t<Adjacency>>;
  using weighted_vertex = std::tuple<vertex_id_type, weight_type>;

  vertex_id_type N = num_vertices(graph);

  std::vector<weight_type> distance(N);

  auto reset = [&] { std::fill(distance.begin(), distance.end(), std::numeric_limits<weight_type>::max()); };

  auto compare = [](const weighted_vertex& a, const weighted_vertex& b) {
    return std::get<1>(a) > std::get<1>(b);
  };

  // Direct property access
  bench("direct property access", ntrial, reset, [&] {
    std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(compare)> Q(compare);
    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {
      auto u = std::get<0>(Q.top());
      Q.pop();

      for (auto&& e : graph[u]) {
        auto v = std::get<0>(e);
        auto w = std::get<1>(e);

        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }
  });

  // Property access through lambda function
  bench("property access via lambda", ntrial, reset, [&] {
    std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(compare)> Q(compare);
    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {
      auto u = std::get<0>(Q.top());
      Q.pop();

      for (auto&& e : graph[u]) {
        auto v = std::get<0>(e);
        auto w = weight_fn(e);

        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }
  });

  // Target access through CPO
  bench("target via CPO", ntrial, reset, [&] {
    std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(compare)> Q(compare);
    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {
      auto u = std::get<0>(Q.top());
      Q.pop();

      for (auto&& e : graph[u]) {
        auto v = target(graph, e);
        auto w = std::get<1>(e);

        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }
  });

  // All indirections (CPO + lambda)
  bench("all indirections (CPO + lambda)", ntrial, reset, [&] {
    std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(compare)> Q(compare);
    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {
      auto u = std::get<0>(Q.top());
      Q.pop();

      for (auto&& e : graph[u]) {
        auto v = target(graph, e);
        auto w = weight_fn(e);

        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }
  });

  // Structured binding (most idiomatic)
  bench("structured binding [v, w]", ntrial, reset, [&] {
    std::priority_queue<weighted_vertex, std::vector<weighted_vertex>, decltype(compare)> Q(compare);
    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {
      auto u = std::get<0>(Q.top());
      Q.pop();

      for (auto&& [v, w] : graph[u]) {
        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }
  });
}

int main(int argc, char* argv[]) {
  // Parse command-line arguments
  Args args(argc, argv);

  // Load graph with edge weights
  auto el = load_graph<directedness::directed, double>(args.file);

  if (args.verbose) {
    el.stream_stats();
  }

  // Build adjacency structure
  auto graph = [&] {
    nw::util::life_timer _("build adjacency");
    return adjacency<1, double>(el);
  }();

  if (args.verbose) {
    graph.stream_stats();
  }
  if (args.debug) {
    graph.stream_indices();
  }

  // Run benchmarks (source = 0)
  auto weight_fn = [](auto&& e) { return std::get<1>(e); };
  run_dijkstra_benchmarks(graph, args.ntrial, vertex_id_t<decltype(graph)>{0}, weight_fn);

  return 0;
}
