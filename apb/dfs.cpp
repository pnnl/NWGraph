/**
 * @file dfs.cpp
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

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

template <typename Adjacency>
auto apb_adj(Adjacency& graph, size_t ntrial, vertex_id_t<Adjacency> seed) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type    N = num_vertices(graph);
  std::vector<bool> visited(N);

  double   time = 0;
  ms_timer t1("raw for loop");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(visited.begin(), visited.end(), false);
    t1.start();

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
    t1.stop();
    time += t1.elapsed();
  }
  std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t2("iterator based for loop");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(visited.begin(), visited.end(), false);
    t2.start();

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
    t2.stop();
    time += t2.elapsed();
  }
  std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t3("range based for loop");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(visited.begin(), visited.end(), false);
    t3.start();

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
    t3.stop();
    time += t3.elapsed();
  }
  std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t4("range based for loop with compound initializer");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(visited.begin(), visited.end(), false);
    t4.start();

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
    t4.stop();
    time += t4.elapsed();
  }
  std::cout << t4.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t5("std::for_each");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(visited.begin(), visited.end(), false);
    t5.start();

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
    t5.stop();
    time += t5.elapsed();
  }
  std::cout << t5.name() << " " << time / ntrial << " ms" << std::endl;
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
      life_timer                        _("deserialize");
      edge_list<directedness::directed> el_a(0);
      el_a.deserialize(read_processed_edgelist);
      return el_a;
    } else if (edgelistFile != "") {
      life_timer _("read mm");
      return read_mm<directedness::directed>(edgelistFile);
    } else {
      usage(argv[0]);
      return edge_list<directedness::directed>(0);
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
    return adjacency<1>(el_a);
  }();

  if (verbose) {
    adj_a.stream_stats();
  }
  if (debug) {
    adj_a.stream_indices();
  }

  apb_adj(adj_a, ntrial, seed);

  return 0;
}
