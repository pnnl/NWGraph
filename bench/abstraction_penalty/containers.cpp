/**
 * @file containers.cpp
 *
 * @brief Container abstraction penalty benchmark.
 *
 * Compares iteration performance across different graph storage formats:
 * - CSR (Compressed Sparse Row) via adjacency<>
 * - VOV (Vector of Vectors) via std::vector<std::vector<>>
 * - VOL (Vector of Lists) via adj_list<>
 * - VOF (Vector of Forward Lists) via adj_flist<>
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Tony Liu
 *
 */

static constexpr const char USAGE[] =
    R"(containers.exe: containers abstraction penalty benchmark driver.
  Usage:
      containers.exe (-h | --help)
      containers.exe -f FILE [-n NUM]  [--format FORMAT...] [-dvV]

  Options:
      -h, --help            show this screen
      -f FILE               input file path
      -n NUM                number of trials [default: 1]
      --format FORMAT       specify which graph storage format [default: CSR]
      -d, --debug           run in debug mode
      -V, --verbose         run in verbose mode
)";

#include <docopt.h>

#include <iostream>
#include <numeric>

#include "apb_common.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/build.hpp"
#include "nwgraph/vofos.hpp"
#include "nwgraph/volos.hpp"
#include "nwgraph/vovos.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <directedness Directedness, class... Attributes>
edge_list<Directedness, Attributes...> load_graph_docopt(const std::string& file) {
  std::ifstream in(file);
  std::string   type;
  in >> type;

  if (type == "BGL17") {
    nw::util::life_timer                        _(std::string("deserialize ") + file);
    edge_list<Directedness, Attributes...> el(0);
    el.deserialize(file);
    return el;
  } else if (type == "%%MatrixMarket") {
    std::cout << "Reading matrix market input " << file << " (slow)\n";
    nw::util::life_timer _(std::string("read_mm ") + file);
    return read_mm<Directedness, Attributes...>(file);
  } else {
    std::cerr << "Did not recognize graph input file " << file << "\n";
    exit(1);
  }
}

template <adjacency_list_graph Graph>
auto compress(edge_list<nw::graph::directedness::undirected, double>& A) {
  nw::util::life_timer _(__func__);
  Graph                B(num_vertices(A));
  push_back_fill(A, B);
  return B;
}

// Specialization for std::vector<std::vector<...>>
template <typename T>
auto compress_vov(edge_list<nw::graph::directedness::undirected, double>& A) {
  nw::util::life_timer _("compress_vov");
  T                    B(num_vertices(A));
  for (auto&& [u, v, w] : A) {
    B[u].emplace_back(v, w);
    B[v].emplace_back(u, w);  // undirected
  }
  return B;
}

template <adjacency_list_graph Graph>
void run_container_benchmarks(Graph& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Graph>;

  vertex_id_type     N = num_vertices(graph);
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);

  auto reset = [&] { std::fill(y.begin(), y.end(), 0); };

  // Iterator-based nested loops
  bench("iterator based nested loop", ntrial, reset, [&] {
    vertex_id_type k = 0;
    for (auto i = graph.begin(); i != graph.end(); ++i) {
      for (auto j = (*i).begin(); j != (*i).end(); ++j) {
        y[k] += x[std::get<0>(*j)] * std::get<1>(*j);
      }
      ++k;
    }
  });

  // Range-based for with structured binding
  bench("range for with structured binding", ntrial, reset, [&] {
    vertex_id_type k = 0;
    for (auto&& i : graph) {
      for (auto&& [j, v] : i) {
        y[k] += x[j] * v;
      }
      ++k;
    }
  });

  // Nested std::for_each
  bench("nested std::for_each", ntrial, reset, [&] {
    vertex_id_type k = 0;
    std::for_each(graph.begin(), graph.end(), [&](auto&& nbhd) {
      std::for_each(nbhd.begin(), nbhd.end(), [&](auto&& elt) {
        auto&& [j, v] = elt;
        y[k] += x[j] * v;
      });
      ++k;
    });
  });

  // neighbor_range adaptor
  bench("neighbor_range adaptor", ntrial, reset, [&] {
    for (auto&& [k, u_neighbors] : make_neighbor_range(graph)) {
      for (auto&& [j, v] : u_neighbors) {
        y[k] += x[j] * v;
      }
    }
  });
}

template <adjacency_list_graph Graph, typename EdgeList>
void run_bench(size_t ntrial, EdgeList& el_a) {
  auto graph = compress<Graph>(el_a);
  run_container_benchmarks(graph, ntrial);
}

// Specialization for vector<vector<...>>
template <typename VovType, typename EdgeList>
void run_bench_vov(size_t ntrial, EdgeList& el_a) {
  auto graph = compress_vov<VovType>(el_a);
  // Run benchmarks manually since vertex_id_t won't work for plain vectors
  using vertex_id_type = uint32_t;
  vertex_id_type     N = graph.size();
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);

  auto reset = [&] { std::fill(y.begin(), y.end(), 0); };

  bench("iterator based nested loop", ntrial, reset, [&] {
    vertex_id_type k = 0;
    for (auto i = graph.begin(); i != graph.end(); ++i) {
      for (auto j = (*i).begin(); j != (*i).end(); ++j) {
        y[k] += x[std::get<0>(*j)] * std::get<1>(*j);
      }
      ++k;
    }
  });

  bench("range for with structured binding", ntrial, reset, [&] {
    vertex_id_type k = 0;
    for (auto&& i : graph) {
      for (auto&& [j, v] : i) {
        y[k] += x[j] * v;
      }
      ++k;
    }
  });

  bench("nested std::for_each", ntrial, reset, [&] {
    vertex_id_type k = 0;
    std::for_each(graph.begin(), graph.end(), [&](auto&& nbhd) {
      std::for_each(nbhd.begin(), nbhd.end(), [&](auto&& elt) {
        auto&& [j, v] = elt;
        y[k] += x[j] * v;
      });
      ++k;
    });
  });
}

int main(int argc, char* argv[]) {
  std::vector<std::string> strings(argv + 1, argv + argc);
  auto                     args = docopt::docopt(USAGE, strings, true);

  // Read the easy options
  bool   verbose = args["--verbose"].asBool();
  bool   debug   = args["--debug"].asBool();
  size_t ntrial  = args["-n"].asLong() ?: 1;
  (void)debug;

  // Read the more complex options
  std::string file = args["-f"].asString();
  std::cout << "processing " << file << "\n";

  auto el_a = load_graph_docopt<nw::graph::directedness::undirected, double>(file);

  if (verbose) {
    el_a.stream_stats();
  }

  std::vector formats = args["--format"].asStringList();
  for (auto& f : formats) {
    std::cout << "\n=== " << f << " format ===" << std::endl;
    if ("CSR" == f) {
      run_bench<adjacency<0, double>>(ntrial, el_a);
    } else if ("VOV" == f) {
      run_bench_vov<std::vector<std::vector<std::tuple<uint32_t, double>>>>(ntrial, el_a);
    } else if ("VOL" == f) {
      run_bench<adj_list<0, double>>(ntrial, el_a);
    } else if ("VOF" == f) {
      run_bench<adj_flist<0, double>>(ntrial, el_a);
    } else {
      std::cerr << "bad format: " << f << std::endl;
      return -1;
    }
  }
  return 0;
}
