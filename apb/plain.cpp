/**
 * @file plain.cpp
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

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

using namespace nw::graph;
using namespace nw::util;

template <typename Adjacency>
auto apb_adj(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type              N = num_vertices(graph);
  std::vector<vertex_id_type> degrees(N);

  {
    auto pr = plain_range(graph);

    std::cout << "plain_range" << std::endl;

    double   time = 0;
    ms_timer t1("iterator based for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t1.start();

      for (auto j = pr.begin(); j != pr.end(); ++j) {
        ++degrees[std::get<0>(*j)];
      }
      t1.stop();
      time += t1.elapsed();
    }
    std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t2("range based for loop auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t2.start();

      for (auto j : pr) {
        ++degrees[std::get<0>(j)];
      }
      t2.stop();
      time += t2.elapsed();
    }
    std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t3("range based for loop auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t3.start();

      for (auto&& j : pr) {
        ++degrees[std::get<0>(j)];
      }
      t3.stop();
      time += t3.elapsed();
    }
    std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t4("range based for loop compound initialization auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t4.start();

      for (auto [j] : pr) {
        ++degrees[j];
      }
      t4.stop();
      time += t4.elapsed();
    }
    std::cout << t4.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t5("range based for loop compound initialization auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t4.start();

      for (auto&& [j] : pr) {
        ++degrees[j];
      }
      t5.stop();
      time += t5.elapsed();
    }
    std::cout << t5.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t6("indexed for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t6.start();

      for (vertex_id_type j = 0; j < N; ++j) {
        ++degrees[j];
      }
      t6.stop();
      time += t6.elapsed();
    }
    std::cout << t6.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t7("std for_each auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t7.start();

      std::for_each(pr.begin(), pr.end(), [&](auto j) { ++degrees[std::get<0>(j)]; });
      t7.stop();
      time += t7.elapsed();
    }
    std::cout << t7.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t8("std for_each auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t8.start();

      std::for_each(pr.begin(), pr.end(), [&](auto&& j) { ++degrees[std::get<0>(j)]; });
      t8.stop();
      time += t8.elapsed();
    }
    std::cout << t8.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t9("counting iterator");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t9.start();

      std::for_each(counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(N), [&](auto j) { ++degrees[j]; });
      t9.stop();
      time += t9.elapsed();
    }
    std::cout << t9.name() << " " << time / ntrial << " ms" << std::endl;
  }
  {
    auto per = edge_range(graph);

    std::cout << "edge_range 0" << std::endl;

    double   time = 0;
    ms_timer t1("raw for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t1.start();

      auto ptr = graph.indices_.data();
      auto idx = std::get<0>(graph.to_be_indexed_).data();

      for (vertex_id_type i = 0; i < N; ++i) {
        for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
          ++degrees[i];
        }
      }
      t1.stop();
      time += t1.elapsed();
    }
    std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t2("iterator based for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t2.start();

      for (auto j = per.begin(); j != per.end(); ++j) {
        ++degrees[std::get<0>(*j)];
      }
      t2.stop();
      time += t2.elapsed();
    }
    std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t3("range based for loop auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t3.start();

      for (auto j : per) {
        ++degrees[std::get<0>(j)];
      }
      t3.stop();
      time += t3.elapsed();
    }
    std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t4("range based for loop auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t4.start();

      for (auto&& j : per) {
        ++degrees[std::get<0>(j)];
      }
      t4.stop();
      time += t4.elapsed();
    }
    std::cout << t4.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t5("range based for loop compound initialization auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t5.start();

      for (auto [j, i] : per) {
        ++degrees[j];
      }
      t5.stop();
      time += t5.elapsed();
    }
    std::cout << t5.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t6("range based for loop compound initialization auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t6.start();

      for (auto&& [i, j] : per) {
        ++degrees[i];
      }
      t6.stop();
      time += t6.elapsed();
    }
    std::cout << t6.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t7("indexed for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t7.start();

      for (vertex_id_type i = 0; i < N; ++i) {
        for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
          ++degrees[i];
        }
      }
      t7.stop();
      time += t7.elapsed();
    }
    std::cout << t7.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t8("std for_each auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t8.start();

      std::for_each(per.begin(), per.end(), [&](auto j) { ++degrees[std::get<0>(j)]; });
      t8.stop();
      time += t8.elapsed();
    }
    std::cout << t8.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t9("std for_each auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t9.start();

      std::for_each(per.begin(), per.end(), [&](auto&& j) { ++degrees[std::get<0>(j)]; });
      t9.stop();
      time += t9.elapsed();
    }
    std::cout << t9.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t10("counting iterator");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t10.start();

      std::for_each(counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(N), [&](auto i) {
        for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
          ++degrees[i];
        }
      });
      t10.stop();
      time += t10.elapsed();
    }
    std::cout << t10.name() << " " << time / ntrial << " ms" << std::endl;
  }

  {

    auto per = edge_range(graph);

    std::cout << "edge_range 1" << std::endl;

    double   time = 0;
    ms_timer t1("raw for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t1.start();

      auto ptr = graph.indices_.data();
      auto idx = std::get<0>(graph.to_be_indexed_).data();

      for (vertex_id_type i = 0; i < N; ++i) {
        for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
          ++degrees[idx[j]];
        }
      }
      t1.stop();
      time += t1.elapsed();
    }
    std::cout << t1.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t2("iterator based for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t2.start();

      for (auto j = per.begin(); j != per.end(); ++j) {
        ++degrees[std::get<1>(*j)];
      }
      t2.stop();
      time += t2.elapsed();
    }
    std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t3("range based for loop auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t3.start();

      for (auto j : per) {
        ++degrees[std::get<1>(j)];
      }
      t3.stop();
      time += t3.elapsed();
    }
    std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t4("range based for loop auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t4.start();

      for (auto&& j : per) {
        ++degrees[std::get<1>(j)];
      }
      t4.stop();
      time += t4.elapsed();
    }
    std::cout << t4.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t5("range based for loop compound initialization auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t5.start();

      for (auto [i, j] : per) {
        ++degrees[j];
      }
      t5.stop();
      time += t5.elapsed();
    }
    std::cout << t5.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t6("range based for loop compound initialization auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t6.start();

      for (auto&& [i, j] : per) {
        ++degrees[j];
      }
      t6.stop();
      time += t6.elapsed();
    }
    std::cout << t6.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t7("indexed for loop");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t7.start();

      for (vertex_id_type i = 0; i < N; ++i) {
        for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
          ++degrees[std::get<0>(*j)];
        }
      }
      t7.stop();
      time += t7.elapsed();
    }
    std::cout << t7.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t8("std for_each auto");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t8.start();

      std::for_each(per.begin(), per.end(), [&](auto j) { ++degrees[std::get<1>(j)]; });
      t8.stop();
      time += t8.elapsed();
    }
    std::cout << t8.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t9("std for_each auto &&");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t9.start();

      std::for_each(per.begin(), per.end(), [&](auto&& j) { ++degrees[std::get<1>(j)]; });
      t9.stop();
      time += t9.elapsed();
    }
    std::cout << t9.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t10("counting iterator with iterator for");
    for (size_t t = 0; t < ntrial; ++t) {
      degrees.clear();
      degrees.resize(N);
      t10.start();

      std::for_each(counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(N), [&](auto i) {
        for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
          ++degrees[std::get<0>(*j)];
        }
      });
      t10.stop();
      time += t10.elapsed();
    }
    std::cout << t10.name() << " " << time / ntrial << " ms" << std::endl;
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

  apb_adj(adj_a, ntrial);

  return 0;
}
