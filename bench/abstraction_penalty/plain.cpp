/**
 * @file plain.cpp
 *
 * @brief Plain range abstraction penalty benchmark.
 *
 * Measures the overhead of various C++ abstractions for degree counting:
 * - plain_range: Flat iteration over all neighbors
 * - edge_range: Iteration over (source, target) pairs
 * - Iterator-based vs range-based for loops
 * - std::get<> vs structured bindings
 * - std::for_each vs range-based for
 * - counting_iterator vs indexed loops
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

#include "apb_common.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/containers/compressed.hpp"

using namespace nw::graph;
using namespace nw::graph::apb;

template <typename Adjacency>
void run_plain_range_benchmarks(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type              N = num_vertices(graph);
  std::vector<vertex_id_type> degrees(N);

  auto reset = [&] {
    degrees.clear();
    degrees.resize(N);
  };

  auto pr = plain_range(graph);

  std::cout << "=== plain_range ===" << std::endl;

  bench("iterator based for loop", ntrial, reset, [&] {
    for (auto j = pr.begin(); j != pr.end(); ++j) {
      ++degrees[std::get<0>(*j)];
    }
  });

  bench("range for auto", ntrial, reset, [&] {
    for (auto j : pr) {
      ++degrees[std::get<0>(j)];
    }
  });

  bench("range for auto&&", ntrial, reset, [&] {
    for (auto&& j : pr) {
      ++degrees[std::get<0>(j)];
    }
  });

  bench("range for structured binding auto", ntrial, reset, [&] {
    for (auto [j] : pr) {
      ++degrees[j];
    }
  });

  bench("range for structured binding auto&&", ntrial, reset, [&] {
    for (auto&& [j] : pr) {
      ++degrees[j];
    }
  });

  bench("indexed for loop", ntrial, reset, [&] {
    for (vertex_id_type j = 0; j < N; ++j) {
      ++degrees[j];
    }
  });

  bench("std::for_each auto", ntrial, reset, [&] {
    std::for_each(pr.begin(), pr.end(), [&](auto j) { ++degrees[std::get<0>(j)]; });
  });

  bench("std::for_each auto&&", ntrial, reset, [&] {
    std::for_each(pr.begin(), pr.end(), [&](auto&& j) { ++degrees[std::get<0>(j)]; });
  });

  bench("counting_iterator", ntrial, reset, [&] {
    std::for_each(counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(N),
                  [&](auto j) { ++degrees[j]; });
  });
}

template <typename Adjacency>
void run_edge_range_source_benchmarks(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type              N = num_vertices(graph);
  std::vector<vertex_id_type> degrees(N);

  auto reset = [&] {
    degrees.clear();
    degrees.resize(N);
  };

  auto per = edge_range(graph);

  std::cout << "\n=== edge_range (source degrees) ===" << std::endl;

  bench("raw for loop", ntrial, reset, [&] {
    auto ptr = graph.indices_.data();
    for (vertex_id_type i = 0; i < N; ++i) {
      for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
        ++degrees[i];
      }
    }
  });

  bench("iterator based for loop", ntrial, reset, [&] {
    for (auto j = per.begin(); j != per.end(); ++j) {
      ++degrees[std::get<0>(*j)];
    }
  });

  bench("range for auto", ntrial, reset, [&] {
    for (auto j : per) {
      ++degrees[std::get<0>(j)];
    }
  });

  bench("range for auto&&", ntrial, reset, [&] {
    for (auto&& j : per) {
      ++degrees[std::get<0>(j)];
    }
  });

  bench("range for structured binding auto", ntrial, reset, [&] {
    for (auto [i, j] : per) {
      ++degrees[i];
    }
  });

  bench("range for structured binding auto&&", ntrial, reset, [&] {
    for (auto&& [i, j] : per) {
      ++degrees[i];
    }
  });

  bench("indexed nested for loop", ntrial, reset, [&] {
    for (vertex_id_type i = 0; i < N; ++i) {
      for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
        ++degrees[i];
      }
    }
  });

  bench("std::for_each auto", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto j) { ++degrees[std::get<0>(j)]; });
  });

  bench("std::for_each auto&&", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto&& j) { ++degrees[std::get<0>(j)]; });
  });

  bench("counting_iterator", ntrial, reset, [&] {
    std::for_each(counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(N), [&](auto i) {
      for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
        ++degrees[i];
      }
    });
  });
}

template <typename Adjacency>
void run_edge_range_target_benchmarks(Adjacency& graph, size_t ntrial) {
  using vertex_id_type = vertex_id_t<Adjacency>;

  vertex_id_type              N = num_vertices(graph);
  std::vector<vertex_id_type> degrees(N);

  auto reset = [&] {
    degrees.clear();
    degrees.resize(N);
  };

  auto per = edge_range(graph);

  std::cout << "\n=== edge_range (target degrees) ===" << std::endl;

  bench("raw for loop", ntrial, reset, [&] {
    auto ptr = graph.indices_.data();
    auto idx = std::get<0>(graph.to_be_indexed_).data();
    for (vertex_id_type i = 0; i < N; ++i) {
      for (auto j = ptr[i]; j < ptr[i + 1]; ++j) {
        ++degrees[idx[j]];
      }
    }
  });

  bench("iterator based for loop", ntrial, reset, [&] {
    for (auto j = per.begin(); j != per.end(); ++j) {
      ++degrees[std::get<1>(*j)];
    }
  });

  bench("range for auto", ntrial, reset, [&] {
    for (auto j : per) {
      ++degrees[std::get<1>(j)];
    }
  });

  bench("range for auto&&", ntrial, reset, [&] {
    for (auto&& j : per) {
      ++degrees[std::get<1>(j)];
    }
  });

  bench("range for structured binding auto", ntrial, reset, [&] {
    for (auto [i, j] : per) {
      ++degrees[j];
    }
  });

  bench("range for structured binding auto&&", ntrial, reset, [&] {
    for (auto&& [i, j] : per) {
      ++degrees[j];
    }
  });

  bench("indexed nested for loop", ntrial, reset, [&] {
    for (vertex_id_type i = 0; i < N; ++i) {
      for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
        ++degrees[std::get<0>(*j)];
      }
    }
  });

  bench("std::for_each auto", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto j) { ++degrees[std::get<1>(j)]; });
  });

  bench("std::for_each auto&&", ntrial, reset, [&] {
    std::for_each(per.begin(), per.end(), [&](auto&& j) { ++degrees[std::get<1>(j)]; });
  });

  bench("counting_iterator nested", ntrial, reset, [&] {
    std::for_each(counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(N), [&](auto i) {
      for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
        ++degrees[std::get<0>(*j)];
      }
    });
  });
}

int main(int argc, char* argv[]) {
  // Parse command-line arguments
  Args args(argc, argv);

  // Load graph
  auto el = load_graph<directedness::directed>(args.file);

  if (args.verbose) {
    el.stream_stats();
  }

  // Build adjacency structure
  auto graph = [&] {
    nw::util::life_timer _("build adjacency");
    return adjacency<1>(el);
  }();

  if (args.verbose) {
    graph.stream_stats();
  }
  if (args.debug) {
    graph.stream_indices();
  }

  // Run all benchmark suites
  run_plain_range_benchmarks(graph, args.ntrial);
  run_edge_range_source_benchmarks(graph, args.ntrial);
  run_edge_range_target_benchmarks(graph, args.ntrial);

  return 0;
}
