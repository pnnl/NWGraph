/**
 * @file exec.cpp
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
#include <vector>

#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

template <typename Adjacency, typename Exec1, typename Exec2>
auto apb_adj(Adjacency& graph, size_t ntrial, Exec1 exec1 = std::execution::seq, Exec2 exec2 = std::execution::seq) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type     N = num_vertices(graph);
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);

  double   time = 0;
  ms_timer t1("counting iterator with raw for loop");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(y.begin(), y.end(), 0);
    t1.start();

    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();
    auto dat = std::get<1>(graph.to_be_indexed_).data();

    std::for_each(exec1, counting_iterator<size_t>(0), counting_iterator<size_t>(N), [&](size_t i) {
      for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
        y[i] += x[idx[j]] * dat[j];
      }
    });
    t1.stop();
    time += t1.elapsed();
  }
  std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t2("counting iterator for_each with counting iterator transform_reduce");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(y.begin(), y.end(), 0);
    t2.start();

    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();
    auto dat = std::get<1>(graph.to_be_indexed_).data();

    std::for_each(exec1, counting_iterator<size_t>(0), counting_iterator<size_t>(N), [&](size_t i) {
      y[i] += std::transform_reduce(exec2, counting_iterator<size_t>(ptr[i]), counting_iterator<size_t>(ptr[i + 1]), 0.0, std::plus<float>(),
                                    [&](size_t j) { return x[idx[j]] * dat[j]; });
    });
    t2.stop();
    time += t2.elapsed();
  }
  std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t3("graph iterator for_each with iterator based for loop");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(y.begin(), y.end(), 0);
    t3.start();

    vertex_id_type k = 0;
    std::for_each(exec1, graph.begin(), graph.end(), [&](auto&& i) {
      for (auto j = (i).begin(); j != (i).end(); ++j) {
        y[k] += x[std::get<0>(*j)] * std::get<1>(*j);
      }
      ++k;
    });

    t3.stop();
    time += t3.elapsed();
  }
  std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

  time = 0;
  ms_timer t4("counting iterator for_each with counting iterator transform_reduce");
  for (size_t t = 0; t < ntrial; ++t) {
    std::fill(y.begin(), y.end(), 0);
    t4.start();

    std::for_each(exec1, counting_iterator<size_t>(0), counting_iterator<size_t>(N), [&](size_t i) {
      y[i] += std::transform_reduce(exec2, graph[i].begin(), graph[i].end(), 0.0, std::plus<float>(),
                                    [&](auto&& j) { return x[std::get<0>(j)] * std::get<1>(j); });
    });

    t4.stop();
    time += t4.elapsed();
  }
  std::cout << t4.name() << " " << time / ntrial << " ms" << std::endl;
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
  const size_t max_versions = 16;

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

  //  std::cout << "# seq seq" << std::endl;
  //  apb_adj(adj_a, std::execution::seq, std::execution::seq);

  std::cout << "# seq par" << std::endl;
  apb_adj(adj_a, ntrial, std::execution::seq, std::execution::par);

  //  std::cout << "# par seq" << std::endl;
  //  apb_adj(adj_a, std::execution::par, std::execution::seq);

  //  std::cout << "# par par" << std::endl;
  //  apb_adj(adj_a, std::execution::par, std::execution::par);

  return 0;
}
