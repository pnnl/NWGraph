/**
 * @file common.hpp
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

#ifndef NW_GRAPH_BENCH_COMMON_HPP
#define NW_GRAPH_BENCH_COMMON_HPP

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/graph_base.hpp"
#include "nwgraph/graph_traits.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/timer.hpp"
#include "nwgraph/util/traits.hpp"

#include <iomanip>
#include <map>
#include <random>
#include <string>
#include <tbb/global_control.h>
#include <tuple>
#include <vector>

namespace nw::graph {
namespace bench {

constexpr inline bool WITH_TBB = true;

auto set_n_threads(long n) {
  if constexpr (WITH_TBB) {
    return tbb::global_control(tbb::global_control::max_allowed_parallelism, n);
  } else {
    return 0;
  }
}

long get_n_threads() {
  if constexpr (WITH_TBB) {
    return tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism);
  } else {
    return 1;
  }
}

std::vector<long> parse_n_threads(const std::vector<std::string>& args) {
  std::vector<long> threads;
  if constexpr (WITH_TBB) {
    if (args.size() == 0) {
      threads.push_back(tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism));
    } else {
      for (auto&& n : args) {
        threads.push_back(std::stol(n));
      }
    }
  } else {
    threads.push_back(1);
  }
  return threads;
}

std::vector<long> parse_ids(const std::vector<std::string>& args) {
  std::vector<long> ids;
  for (auto&& n : args) {
    ids.push_back(std::stol(n));
  }
  return ids;
}

template <directedness Directedness, class... Attributes>
edge_list<Directedness, Attributes...> load_graph(std::string file) {
  std::ifstream in(file);
  std::string   type;
  in >> type;
  
  if (type == "NW") {
    nw::util::life_timer                   _("deserialize");
    edge_list<Directedness, Attributes...> aos_a(0);
    aos_a.deserialize(file);
    return aos_a;
  } else if (type == "%%MatrixMarket") {
    std::cout << "Reading matrix market input " << file << " (slow)\n";
    nw::util::life_timer _("read mm");
    return read_mm<Directedness, Attributes...>(file);
  } else {
    std::cerr << "Did not recognize graph input file " << file << "\n";
    exit(1);
  }
}

template <int Adj, class ExecutionPolicy = std::execution::parallel_unsequenced_policy, directedness Directedness, class... Attributes>
adjacency<Adj, Attributes...> build_adjacency(edge_list<Directedness, Attributes...>& graph, bool sort_adjacency = false, ExecutionPolicy&& policy = {}) {
  nw::util::life_timer _("build adjacency");
  return {graph, sort_adjacency, policy};
}

template <class Graph>
auto build_degrees(const Graph& graph) {
  using Id = typename nw::graph::vertex_id_t<std::decay_t<Graph>>;
  nw::util::life_timer _("degrees");
  std::vector<Id>      degrees(graph.size());
  tbb::parallel_for(edge_range(graph), [&](auto&& edges) {
    for (auto&& [i, j] : edges) {
      __atomic_fetch_add(&degrees[j], 1, __ATOMIC_ACQ_REL);
    }
  });
  return degrees;
}

template <class Graph>
auto build_random_sources(const Graph& graph, size_t n, long seed) {
  using Id = typename nw::graph::vertex_id_t<std::decay_t<Graph>>;

  auto sources = std::vector<Id>(n);
  auto degrees = build_degrees(graph);
  auto gen     = std::mt19937(seed);
  auto dis     = std::uniform_int_distribution<Id>(0, num_vertices(graph));

  for (auto& id : sources) {
    for (id = dis(gen); degrees[id] == 0; id = dis(gen)) {
    }
  }
  return sources;
}

/// Load a set of vertices from a file.
///
/// This will load a set of vertices from the passed `file` and verify that we
/// have the expected number `n`.
template <class Graph>
auto load_sources_from_file(const Graph&, std::string file, size_t n = 0) {
  using Id            = typename nw::graph::vertex_id_t<std::decay_t<Graph>>;
  std::vector sources = read_mm_vector<Id>(file);
  if (n && sources.size() != n) {
    std::cerr << file << " contains " << sources.size() << " sources, however options require " << n << "\n";
    exit(1);
  }
  return sources;
}

/// Helper to time an operation.
template <class Op>
auto time_op(Op&& op) {
  if constexpr (std::is_void_v<decltype(op())>) {
    auto start = std::chrono::high_resolution_clock::now();
    op();
    std::chrono::duration<double> end = std::chrono::high_resolution_clock::now() - start;
    return std::tuple{end.count()};
  } else {
    auto                          start = std::chrono::high_resolution_clock::now();
    auto                          e     = op();
    std::chrono::duration<double> end   = std::chrono::high_resolution_clock::now() - start;
    return std::tuple{end.count(), std::move(e)};
  }
}

template <class Op, class Check>
auto time_op_verify(Op&& op, Check&& check) {
  auto&& [t, e] = time_op(std::forward<Op>(op));
  return std::tuple(t, check(std::move(e)));
}

template <class... Extra>
class Times {
  using Sample                                                              = std::tuple<double, Extra...>;
  std::map<std::tuple<std::string, long, long>, std::vector<Sample>> times_ = {};

public:
  decltype(auto) begin() const { return times_.begin(); }
  decltype(auto) end() const { return times_.end(); }

  template <class Op>
  auto record(const std::string& file, long id, long thread, Op&& op, Extra... extra) {
    return std::apply(
        [&](auto time, auto&&... rest) {
          append(file, id, thread, time, extra...);
          return std::tuple{std::forward<decltype(rest)>(rest)...};
        },
        time_op(std::forward<Op>(op)));
  }

  template <class Op, class Verify>
  void record(const std::string& file, long id, long thread, Op&& op, Verify&& verify, Extra... extra) {
    auto&& [time, result] = time_op(std::forward<Op>(op));
    verify(std::forward<decltype(result)>(result));
    append(file, id, thread, time, extra...);
  }

  void append(std::string file, long id, long thread, double trial, Extra... extra) {
    times_[std::tuple(file, id, thread)].emplace_back(trial, extra...);
  }

  void print(std::ostream& out) const {
    std::size_t n = 4;
    for (auto&& [config, samples] : times_) {
      n = std::max(n, std::get<0>(config).size());
    }

    out << std::setw(n + 2) << std::left << "File";
    out << std::setw(10) << std::left << "Version";
    out << std::setw(10) << std::left << "Threads";
    out << std::setw(20) << std::left << "Min";
    out << std::setw(20) << std::left << "Avg";
    out << std::setw(20) << std::left << "Max";
    out << "\n";

    for (auto&& [config, samples] : times_) {
      auto [file, id, threads] = config;
      auto [min, max, avg]     = minmaxavg(samples);

      out << std::setw(n + 2) << std::left << file;
      out << std::setw(10) << std::left << id;
      out << std::setw(10) << std::left << threads;
      out << std::setw(20) << std::left << std::setprecision(6) << std::fixed << min;
      out << std::setw(20) << std::left << std::setprecision(6) << std::fixed << avg;
      out << std::setw(20) << std::left << std::setprecision(6) << std::fixed << max;
      out << "\n";
    }
  }

private:
  static auto average(const std::vector<Sample>& times) {
    double total = 0.0;
    for (auto&& sample : times) {
      total += std::get<0>(sample);
    }
    return total / times.size();
  }

  static auto minmax(const std::vector<Sample>& times) {
    return std::apply([](auto... minmax) { return std::tuple(std::get<0>(*minmax)...); },
                      std::minmax_element(times.begin(), times.end(), [](auto&& a, auto&& b) { return std::get<0>(a) < std::get<0>(b); }));
  }

  static auto minmaxavg(const std::vector<Sample>& times) {
    return std::apply([&](auto... minmax) { return std::tuple(minmax..., average(times)); }, minmax(times));
  }
};
}    // namespace bench
}    // namespace nw::graph

#endif    // NW_GRAPH_BENCH_COMMON_HPP
