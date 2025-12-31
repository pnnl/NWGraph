/**
 * @file exec.cpp
 *
 * @brief Execution policy abstraction penalty benchmark.
 *
 * Measures the overhead of C++ execution policies for SpMV:
 * - Sequential vs parallel outer loop
 * - Sequential vs parallel inner reduction
 * - Counting iterator vs graph iterator
 * - Raw for loop vs transform_reduce
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

#include <execution>
#include <iostream>
#include <numeric>
#include <vector>

#include "apb_common.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/containers/compressed.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <typename Adjacency, typename Exec1, typename Exec2>
void run_exec_benchmarks(Adjacency& graph, size_t ntrial, Exec1 exec1, Exec2 exec2) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type     N = num_vertices(graph);
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);

  auto reset = [&] { std::fill(y.begin(), y.end(), 0); };

  // Counting iterator with raw for loop
  bench("counting_iterator + raw for loop", ntrial, reset, [&] {
    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();
    auto dat = std::get<1>(graph.to_be_indexed_).data();

    std::for_each(exec1, counting_iterator<size_t>(0), counting_iterator<size_t>(N), [&](size_t i) {
      for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
        y[i] += x[idx[j]] * dat[j];
      }
    });
  });

  // Counting iterator with transform_reduce (raw pointers)
  bench("counting_iterator + transform_reduce (raw)", ntrial, reset, [&] {
    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();
    auto dat = std::get<1>(graph.to_be_indexed_).data();

    std::for_each(exec1, counting_iterator<size_t>(0), counting_iterator<size_t>(N), [&](size_t i) {
      y[i] += std::transform_reduce(exec2, counting_iterator<size_t>(ptr[i]), counting_iterator<size_t>(ptr[i + 1]),
                                    0.0f, std::plus<float>(), [&](size_t j) { return x[idx[j]] * dat[j]; });
    });
  });

  // Graph iterator with nested for loop
  bench("graph iterator + nested for loop", ntrial, reset, [&] {
    vertex_id_type k = 0;
    std::for_each(exec1, graph.begin(), graph.end(), [&](auto&& i) {
      for (auto j = i.begin(); j != i.end(); ++j) {
        y[k] += x[std::get<0>(*j)] * std::get<1>(*j);
      }
      ++k;
    });
  });

  // Counting iterator with transform_reduce (graph iterators)
  bench("counting_iterator + transform_reduce (graph)", ntrial, reset, [&] {
    std::for_each(exec1, counting_iterator<size_t>(0), counting_iterator<size_t>(N), [&](size_t i) {
      y[i] += std::transform_reduce(exec2, graph[i].begin(), graph[i].end(), 0.0f, std::plus<float>(),
                                    [&](auto&& j) { return x[std::get<0>(j)] * std::get<1>(j); });
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

  // Run benchmarks with seq/par execution policies
  std::cout << "=== seq outer, par inner ===" << std::endl;
  run_exec_benchmarks(graph, args.ntrial, std::execution::seq, std::execution::par);

  return 0;
}
