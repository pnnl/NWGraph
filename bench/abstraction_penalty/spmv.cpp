/**
 * @file spmv.cpp
 *
 * @brief SpMV (sparse matrix-vector multiply) abstraction penalty benchmark.
 *
 * Measures the overhead of various C++ abstractions for SpMV:
 * - Raw pointer-based iteration vs iterators vs range-based for
 * - std::get<> vs structured bindings
 * - std::for_each vs range-based for
 * - edge_range vs neighbor_range adaptors
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
#include <numeric>
#include <vector>

#include "apb_common.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/containers/compressed.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <typename Adjacency>
void run_spmv_benchmarks(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type     N = num_vertices(graph);
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);

  auto reset = [&] { std::fill(y.begin(), y.end(), 0); };

  auto per = make_edge_range<0>(graph);

  std::cout << "=== Nested loop variants ===" << std::endl;

  // Baseline: raw pointer-based access
  bench("raw for loop", ntrial, reset, [&] {
    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();
    auto dat = std::get<1>(graph.to_be_indexed_).data();

    for (vertex_id_type i = 0; i < N; ++i) {
      for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
        y[i] += x[idx[j]] * dat[j];
      }
    }
  });

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

  std::cout << "\n=== edge_range variants ===" << std::endl;

  // edge_range with auto
  bench("edge_range auto", ntrial, reset, [&] {
    for (auto j : per) {
      y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
    }
  });

  // edge_range with auto&&
  bench("edge_range auto&&", ntrial, reset, [&] {
    for (auto&& j : per) {
      y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
    }
  });

  // edge_range with structured binding auto
  bench("edge_range structured binding auto", ntrial, reset, [&] {
    for (auto [i, j, v] : per) {
      y[i] += x[j] * v;
    }
  });

  // edge_range with structured binding auto&&
  bench("edge_range structured binding auto&&", ntrial, reset, [&] {
    for (auto&& [i, j, v] : per) {
      y[i] += x[j] * v;
    }
  });

  // edge_range with std::for_each auto
  bench("edge_range std::for_each auto", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto j) {
      y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
    });
  });

  // edge_range with std::for_each auto&&
  bench("edge_range std::for_each auto&&", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto&& j) {
      y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
    });
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

  // Run benchmarks
  run_spmv_benchmarks(graph, args.ntrial);

  return 0;
}
