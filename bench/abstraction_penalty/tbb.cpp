/**
 * @file tbb.cpp
 *
 * @brief TBB parallelization abstraction penalty benchmark.
 *
 * Measures the overhead of TBB parallel_for vs sequential approaches:
 * - Sequential for loop vs std::for_each vs tbb::parallel_for
 * - Degree counting (source and target)
 * - SpMV with edge_range
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
#include "nwgraph/containers/compressed.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <typename Adjacency>
void run_tbb_degree_benchmarks(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type              N = num_vertices(graph);
  std::vector<vertex_id_type> degrees(N);

  auto reset = [&] {
    degrees.clear();
    degrees.resize(N);
  };

  auto per = edge_range(graph);

  std::cout << "=== Source degree counting ===" << std::endl;

  bench("iterator based for loop", ntrial, reset, [&] {
    for (auto j = per.begin(); j != per.end(); ++j) {
      ++degrees[std::get<0>(*j)];
    }
  });

  bench("std::for_each", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto&& j) { ++degrees[std::get<0>(j)]; });
  });

  bench("tbb::parallel_for", ntrial, reset, [&] {
    tbb::parallel_for(per, [&](auto&& x) {
      std::for_each(x.begin(), x.end(), [&](auto&& x) { ++degrees[std::get<0>(x)]; });
    });
  });

  std::cout << "\n=== Target degree counting ===" << std::endl;

  bench("iterator based for loop", ntrial, reset, [&] {
    for (auto j = per.begin(); j != per.end(); ++j) {
      ++degrees[std::get<1>(*j)];
    }
  });

  bench("std::for_each", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto&& j) { ++degrees[std::get<1>(j)]; });
  });

  bench("tbb::parallel_for", ntrial, reset, [&] {
    tbb::parallel_for(per, [&](auto&& x) {
      std::for_each(x.begin(), x.end(), [&](auto&& x) { ++degrees[std::get<1>(x)]; });
    });
  });
}

template <typename Adjacency>
void run_tbb_spmv_benchmarks(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type     N = num_vertices(graph);
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);

  auto reset = [&] { std::fill(y.begin(), y.end(), 0); };

  auto per = make_edge_range<0>(graph);

  std::cout << "\n=== SpMV with edge_range ===" << std::endl;

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

  bench("std::for_each edge_range", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto&& j) {
      y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
    });
  });

  bench("tbb::parallel_for edge_range", ntrial, reset, [&] {
    tbb::parallel_for(per, [&](auto&& chunk) {
      std::for_each(chunk.begin(), chunk.end(), [&](auto&& j) {
        y[std::get<0>(j)] += x[std::get<1>(j)] * std::get<2>(j);
      });
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
  run_tbb_degree_benchmarks(graph, args.ntrial);
  run_tbb_spmv_benchmarks(graph, args.ntrial);

  return 0;
}
