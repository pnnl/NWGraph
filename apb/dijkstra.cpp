/**
 * @file dijkstra.cpp
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
#include <queue>
#include <vector>

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/adaptors/bfs_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/tag_invoke.hpp"


using namespace nw::graph;
using namespace nw::util;

namespace nw::graph {

#if 0
DECL_TAG_INVOKE(out_edges);
DECL_TAG_INVOKE(target);

template <typename Graph, typename Int>
auto tag_invoke(const out_edges_tag, const Graph& g, Int u) {
  return g[u];
}

template <typename Graph, typename Edge>
auto tag_invoke(const target_tag, const Graph& g, Edge e) {
  return std::get<0>(e);
}
#endif

}    // namespace nw::graph

template <typename Adjacency, class F>
auto apb_adj(Adjacency& graph, size_t ntrial, vertex_id_t<Adjacency> seed, F fun) {
  double time = 0;

  using vertex_id_type  = vertex_id_t<Adjacency>;
  using weight_type     = std::tuple_element_t<1, inner_value_t<Adjacency>>;
  using weighted_vertex = std::tuple<vertex_id_type, weight_type>;

  vertex_id_type N      = num_vertices(graph);
  vertex_id_type source = seed;

  std::vector<weight_type> distance(N, std::numeric_limits<weight_type>::max());

  std::priority_queue<weighted_vertex, std::vector<weighted_vertex>,
                      decltype([](auto&& a, auto&& b) { return (std::get<1>(a) > std::get<1>(b)); })>
      Q;

  ms_timer t1("Access stored properties directly");
  for (size_t t = 0; t < ntrial; ++t) {

    std::fill(distance.begin(), distance.end(), std::numeric_limits<weight_type>::max());

    t1.start();

    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {

      auto u = std::get<0>(Q.top());
      Q.pop();

      // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

      for (auto&& e : graph[u]) {
        auto v = std::get<0>(e);    // target vertex (neighbor)
        auto w = std::get<1>(e);    // edge weight

        // relax
        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }

    t1.stop();
    time += t1.elapsed();
  }
  std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t2("Access stored properties through (inlinable) lambda function");

  for (size_t t = 0; t < ntrial; ++t) {

    std::fill(distance.begin(), distance.end(), std::numeric_limits<weight_type>::max());

    t2.start();

    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {

      auto u = std::get<0>(Q.top());
      Q.pop();

      // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

      for (auto&& e : graph[u]) {
        auto v = std::get<0>(e);    // target vertex (neighbor)
        auto w = fun(e);            // edge weight

        // relax
        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }

    t2.stop();
    time += t2.elapsed();
  }
  std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t3("access stored properties directly and neighbors through CPO");
  for (size_t t = 0; t < ntrial; ++t) {

    std::fill(distance.begin(), distance.end(), std::numeric_limits<weight_type>::max());

    t3.start();

    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {

      auto u = std::get<0>(Q.top());
      Q.pop();

      // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

      for (auto&& e : graph[u]) {
        auto v = std::get<0>(e);         // target vertex (neighbor)
        auto w = std::get<1>(e);    // edge weight

        // relax
        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }

    t3.stop();
    time += t3.elapsed();
  }
  std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t4("access stored properties directly and target through CPO");
  for (size_t t = 0; t < ntrial; ++t) {

    std::fill(distance.begin(), distance.end(), std::numeric_limits<weight_type>::max());

    t4.start();

    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {

      auto u = std::get<0>(Q.top());
      Q.pop();

      // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

      for (auto&& e : graph[u]) {
        auto v = target(graph, e);    // target vertex (neighbor)
        auto w = std::get<1>(e);      // edge weight

        // relax
        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }

    t4.stop();
    time += t4.elapsed();
  }

  time = 0;
  ms_timer t5("All indirections");
  for (size_t t = 0; t < ntrial; ++t) {

    std::fill(distance.begin(), distance.end(), std::numeric_limits<weight_type>::max());

    t5.start();

    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {

      auto u = std::get<0>(Q.top());
      Q.pop();

      // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

      for (auto&& e : graph[u]) {
        auto v = target(graph, e);    // target vertex (neighbor)
        auto w = fun(e);              // edge weight

        // relax
        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }

    t5.stop();
    time += t5.elapsed();
  }
  std::cout << t5.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t6("No indirections (again)");
  for (size_t t = 0; t < ntrial; ++t) {

    std::fill(distance.begin(), distance.end(), std::numeric_limits<weight_type>::max());

    t6.start();

    distance[source] = 0;
    Q.push({source, distance[source]});

    while (!Q.empty()) {

      auto u = std::get<0>(Q.top());
      Q.pop();

      // for (auto&& [v, w] : g[u]) -- pretty but would only allow one property

      for (auto&& e : graph[u]) {
        auto v = std::get<0>(e);    // target vertex (neighbor)
        auto w = std::get<1>(e);    // edge weight

        // relax
        if (distance[u] + w < distance[v]) {
          distance[v] = distance[u] + w;
          Q.push({v, distance[v]});
        }
      }
    }

    t6.stop();
    time += t6.elapsed();
  }
  std::cout << t6.name() << " " << time / ntrial << " ms" << std::endl;
}

void usage(const std::string& msg = "") { std::cout << std::string("Usage: ") + msg + " " << std::endl; }

int main(int argc, char* argv[]) {
  std::string edgelistFile             = "";
  std::string versions                 = "0";
  std::string read_processed_edgelist  = "";
  std::string write_processed_edgelist = "";

  bool   verbose = false;
  bool   debug   = false;
  size_t nthread = 1;
  (void)nthread;    // silence warnings
  size_t ntrial = 1;
  (void)ntrial;    // silence warnings
  default_vertex_id_type seed         = 0;
  const size_t           max_versions = 16;

  for (int argIndex = 1; argIndex < argc; ++argIndex) {
    std::string arg(argv[argIndex]);

    if (arg == "--edgelistfile" || arg == "-f" || arg == "-i") {
      if (++argIndex == argc) {
        usage(argv[0]);
      }
      edgelistFile = std::string(argv[argIndex]);
    } else if (arg == "--read_processed_edgelist") {
      if (++argIndex == argc) {
        usage(argv[0]);
      }
      read_processed_edgelist = std::string(argv[argIndex]);
    } else if (arg == "--write_processed_edgelist") {
      if (++argIndex == argc) {
        usage(argv[0]);
      }
      write_processed_edgelist = std::string(argv[argIndex]);
    } else if (arg == "--ntrial" || arg == "--ntrials") {
      if (++argIndex == argc) {
        usage(argv[0]);
      }
      ntrial = std::stoi(argv[argIndex]);
    } else if (arg == "--nthread" || arg == "--nthreads") {
      if (++argIndex == argc) {
        usage(argv[0]);
      }
      nthread = std::stoi(argv[argIndex]);
    } else if (arg == "--version" || arg == "--versions") {
      if (++argIndex == argc) {
        usage(argv[0]);
      }
      versions = std::string(argv[argIndex]);
    } else if (arg == "-s") {
      if (++argIndex == argc) {
        usage(argv[0]);
      }
      seed = std::stoi(argv[argIndex]);
    } else if (arg == "-d") {
      debug = true;
    } else if (arg == "-v") {
      verbose = true;
    } else {
      usage(argv[0]);
      return -1;
    }
  }

  auto el_a = [&]() {
    if (read_processed_edgelist != "") {
      life_timer                                _("deserialize");
      edge_list<directedness::directed, double> el_a(0);
      el_a.deserialize(read_processed_edgelist);
      return el_a;
    } else if (edgelistFile != "") {
      life_timer _("read mm");
      return read_mm<directedness::directed, double>(edgelistFile);
    } else {
      usage(argv[0]);
      return edge_list<directedness::directed, double>(0);
    }
  }();

  if (verbose) {
    el_a.stream_stats();
  }

  if (write_processed_edgelist != "") {
    el_a.serialize(write_processed_edgelist);
  }

  //  apb_el(el_a);

  auto adj_a = [&]() {
    life_timer _("build");
    return adjacency<1, double>(el_a);
  }();

  if (verbose) {
    adj_a.stream_stats();
  }
  if (debug) {
    adj_a.stream_indices();
  }

  apb_adj(adj_a, ntrial, seed, [](auto&& elt) { return std::get<1>(elt); });

  return 0;
}
