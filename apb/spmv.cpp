/**
 * @file spmv.cpp
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

#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

template <typename Adjacency>
auto apb_adj(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type     N = num_vertices(graph);
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);

  {
    auto per = make_edge_range<0>(graph);

    std::cout << "edge_range\n";

    double   time = 0;
    ms_timer t1("raw for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t1.start();

      auto ptr = graph.indices_.data();
      auto idx = std::get<0>(graph.to_be_indexed_).data();
      auto dat = std::get<1>(graph.to_be_indexed_).data();

      for (vertex_id_type i = 0; i < N; ++i) {
        for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
          y[i] += x[idx[j]] * dat[j];
        }
      }
      t1.stop();
      time += t1.elapsed();
    }
    std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t2("iterator based for loop with iterator based for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t2.start();

      vertex_id_type k = 0;
      for (auto i = graph.begin(); i != graph.end(); ++i) {
        for (auto j = (*i).begin(); j != (*i).end(); ++j) {
          y[k] += x[std::get<0>(*j)] * std::get<1>(*j);
        }
        ++k;
      }
      t2.stop();
      time += t2.elapsed();
    }
    std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t3("range based for loop with range based for loop with structured binding");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t3.start();

      vertex_id_type k = 0;
      for (auto&& i : graph) {
        for (auto&& [j, v] : i) {
          y[k] += x[j] * v;
        }
        ++k;
      }
      t3.stop();
      time += t3.elapsed();
    }
    std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer ta("nested std::for_each auto&&");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      ta.start();

      vertex_id_type k = 0;
      std::for_each(graph.begin(), graph.end(), [&](auto&& nbhd) {
	  std::for_each(nbhd.begin(), nbhd.end(), [&] (auto&& elt) {
	    auto&& [j, v] = elt;
	    y[k] += x[j] * v;
	  });
	  ++k;
	});
      ta.stop();
      time += ta.elapsed();
    }
    std::cout << ta.name() << " " << time / ntrial << " ms" << std::endl;


    time = 0;
    ms_timer tb("nested range for with neighbor_range");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      tb.start();

      vertex_id_type k = 0;

      for (auto&& [k, u_neighbors] : make_neighbor_range(graph)) {
        for (auto &&[j, v] : u_neighbors) {
          y[k] += x[j] * v;
        }
      }
      tb.stop();

      time += tb.elapsed();
    }
    std::cout << tb.name() << " " << time / ntrial << " ms" << std::endl;


    time = 0;
    ms_timer t4("range based for loop edge range auto");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t4.start();

      for (auto j : per) {
        y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
      }
      t4.stop();
      time += t4.elapsed();
    }
    std::cout << t4.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t5("range based for loop edge range auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t5.start();

      for (auto&& j : per) {
        y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
      }
      t5.stop();
      time += t5.elapsed();
    }
    std::cout << t5.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t6("range based for loop structured binding edge range auto");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t6.start();

      for (auto [i, j, v] : per) {
        y[i] += x[j] * v;
      }
      t6.stop();
      time += t6.elapsed();
    }
    std::cout << t6.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t7("range based for loop structured binding edge range auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t7.start();

      for (auto&& [i, j, v] : per) {
        y[i] += x[j] * v;
      }
      t7.stop();
      time += t7.elapsed();
    }
    std::cout << t7.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t8("std for_each edge range auto");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t8.start();

      std::for_each(per.begin(), per.end(), [&](auto j) { y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j); });
      t8.stop();
      time += t8.elapsed();
    }
    std::cout << t8.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t9("std for_each edge range auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t9.start();

      std::for_each(per.begin(), per.end(), [&](auto&& j) { y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j); });
      t9.stop();
      time += t9.elapsed();
    }
    std::cout << t9.name() << " " << time / ntrial << " ms" << std::endl;
  }
  if constexpr (false) {
    auto per = edge_range(graph);

    std::cout << "edge_range" << std::endl;

    double   time = 0;
    ms_timer t1("raw for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t1.start();

      auto ptr = graph.indices_.data();
      auto idx = std::get<0>(graph.to_be_indexed_).data();
      auto dat = std::get<1>(graph.to_be_indexed_).data();

      for (vertex_id_type i = 0; i < N; ++i) {
        for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
          y[i] += x[idx[j]] * dat[j];
        }
      }
      t1.stop();
      time += t1.elapsed();
    }
    std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t2("iterator based for loop with iterator based for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t2.start();

      vertex_id_type k = 0;
      for (auto i = graph.begin(); i != graph.end(); ++i) {
        for (auto j = (*i).begin(); j != (*i).end(); ++j) {
          y[k] += x[std::get<0>(*j)] * std::get<1>(*j);
        }
        ++k;
      }
      t2.stop();
      time += t2.elapsed();
    }
    std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t3("range based for loop auto");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t3.start();

      for (auto j : per) {
        y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
      }
      t3.stop();
      time += t3.elapsed();
    }
    std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t4("range based for loop auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t4.start();

      for (auto&& j : per) {
        y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
      }
      t4.stop();
      time += t4.elapsed();
    }
    std::cout << t4.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t5("range based for loop structured binding auto");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t5.start();

      for (auto [i, j, v] : per) {
        y[i] += x[j] * v;
      }
      t5.stop();
      time += t5.elapsed();
    }
    std::cout << t5.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t6("range based for loop structured binding auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t6.start();

      for (auto&& [i, j, v] : per) {
        y[i] += x[j] * v;
      }
      t6.stop();
      time += t6.elapsed();
    }
    std::cout << t6.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t7("std for_each auto");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t7.start();

      std::for_each(per.begin(), per.end(), [&](auto j) { y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j); });
      t7.stop();
      time += t7.elapsed();
    }
    std::cout << t7.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t8("std for_each auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t8.start();

      std::for_each(per.begin(), per.end(), [&](auto&& j) { y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j); });
      t8.stop();
      time += t8.elapsed();
    }
    std::cout << t8.name() << " " << time / ntrial << " ms" << std::endl;
  }
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

  apb_adj(adj_a, ntrial);

  return 0;
}
