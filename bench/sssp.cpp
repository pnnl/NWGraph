/**
 * @file sssp.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *   liux238
 *
 */

static constexpr const char USAGE[] =
    R"(sssp.exe : BGL17 page rank benchmark driver.
  Usage:
      sssp.exe (-h | --help)
      sssp.exe -f FILE [-r NODE | -s FILE] [-i NUM] [-n NUM] [-d NUM] [--seed NUM] [--version ID...] [--log FILE] [--log-header] [-vV] [--debug] [THREADS]...

  Options:
      -h, --help              show this screen
      -f FILE                 input file path
      -i NUM                  number of iteration [default: 1]
      -n NUM                  number of trials [default: 1]
      -r NODE                 start from node r
      -d, --delta NUM         value for delta [default: 2]
      -s, --sources FILE      sources file
      --seed NUM              random seed [default: 27491095]
      --version ID            algorithm version to run [default: 0]
      --log FILE              log times to a file
      --log-header            add a header to the log file
      --debug                 run in debug mode
      -v, --verify            verify results
      -V, --verbose           run in verbose mode
)";

#include "Log.hpp"
#include "nwgraph/algorithms/delta_stepping.hpp"
#include "nwgraph/experimental/algorithms/delta_stepping.hpp"
//#include "nwgraph/algorithms/dijkstra.hpp"
#include "common.hpp"
#include "nwgraph/util/make_priority_queue.hpp"
#include "nwgraph/util/traits.hpp"
#include <docopt.h>

using namespace nw::graph::bench;
using namespace nw::graph;
using namespace nw::util;

using distance_t = std::uint64_t;

/// Basic sequential sssp (Dijkstra) copied from GAP benchmark suite.
template <adjacency_list_graph Graph, class Weight>
static auto dijkstra(const Graph& graph, vertex_id_t<Graph> source,
  Weight weight = [](auto& e) -> auto& { return std::get<1>(e); }) {
  using vertex_id_type = vertex_id_t<Graph>;


  // Distances
  std::vector<distance_t> dist(num_vertices(graph), std::numeric_limits<distance_t>::max()/4);

  // Workqueue
  //  using WN = std::tuple<vertex_id_type, distance_t>;
  //  auto mq  = nw::graph::make_priority_queue<WN>([](const WN& a, const WN& b) { return std::get<1>(a) > std::get<1>(b); });
  auto mq  = nw::graph::make_priority_queue<vertex_id_type>([&dist](const vertex_id_type& a, vertex_id_type& b) { 
							      return dist[a] > dist[b]; 
							    });
  mq.emplace(source);
  dist[source] = 0;

  while (!mq.empty()) {
    auto u = mq.top();    // don't capture auto&& references here
    mq.pop();
    for (auto&& elt : graph[u]) {
      auto&& v = target(graph, elt);
      auto&& w = weight(elt);
      //assert(w > 0);
      //assert(td < td + w);
      auto tw = dist[u] + w;
      if (tw < dist[v]) {
	dist[v] = tw;
	mq.emplace(v);
      }
    }
  }
  return dist;
}

template <adjacency_list_graph Graph, class Dist, class Weight>
static bool SSSPVerifier(const Graph& graph, vertex_id_t<Graph> source, Dist&& dist, bool verbose,
Weight weight) {

  auto oracle = dijkstra(graph, source, weight);
  if (std::equal(dist.begin(), dist.end(), oracle.begin())) {
    return true;
  }

  std::cerr << "NWGraph: verification failed\n";

  if (verbose) {
    if (dist.size() != oracle.size()) {
      std::cerr << "NWGraph: verification found different graph sizes\n";
    }

    for (std::size_t i = 0, e = dist.size(); i != e; ++i) {
      if (dist[i] != oracle[i]) {
        std::cout << "failed: " << i << " " << dist[i] << " " << oracle[i] << "\n";
      }
    }
  }
  return false;
}

/// The heart of the SSSP benchmark, dispatches to the right algorithm version
/// and verifies the result, based on the verifier. Returns the time it took to
/// run, as well as a boolean indicating if we passed verification.
template <adjacency_list_graph Graph, class Weight, class Verifier>
static std::tuple<double, bool> sssp(int id, const Graph& graph, vertex_id_t<Graph> source, distance_t delta, Weight weight,
                                     Verifier&& verifier) {
  switch (id) {
    case 0:
      return time_op_verify([&] { return delta_stepping<distance_t>(graph, source, delta, weight); }, std::forward<Verifier>(verifier));
    case 1:
      return time_op_verify([&] { return delta_stepping_m1<distance_t>(graph, source, delta, weight); }, std::forward<Verifier>(verifier));
    case 6:
      return time_op_verify([&] { return delta_stepping_v6<distance_t>(graph, source, delta); }, std::forward<Verifier>(verifier));
    case 8:
      return time_op_verify([&] { return delta_stepping_v8<distance_t>(graph, source, delta, weight); }, std::forward<Verifier>(verifier));
    case 9:
      return time_op_verify([&] { return delta_stepping_v9<distance_t>(graph, source, delta, weight); }, std::forward<Verifier>(verifier));
    case 10:
      return time_op_verify([&] { return delta_stepping_v10<distance_t>(graph, source, delta, weight); }, std::forward<Verifier>(verifier));
    case 11:
      return time_op_verify([&] { return delta_stepping_v11<distance_t>(graph, source, delta, weight); }, std::forward<Verifier>(verifier));
    case 12:
      return time_op_verify([&] { return delta_stepping<distance_t>(graph, source, delta); }, std::forward<Verifier>(verifier));
    case 13:
      return time_op_verify([&] { return dijkstra<Graph, Weight>(graph, source, weight); }, std::forward<Verifier>(verifier));
    default:
      std::cerr << "Invalid SSSP version " << id << "\n";
      return std::tuple(0.0, true);
  }
}

int main(int argc, char* argv[]) {
  std::vector strings = std::vector<std::string>(argv + 1, argv + argc);
  std::map    args    = docopt::docopt(USAGE, strings, true);

  // Read the options
  bool        verify     = args["--verify"].asBool();
  bool        verbose    = args["--verbose"].asBool();
  bool        debug      = args["--debug"].asBool();
  long        trials     = args["-n"].asLong() ?: 1;    // at least one trial
  long        iterations = args["-i"].asLong() ?: 1;    // at least one iteration
  std::string file       = args["-f"].asString();
  std::size_t delta      = args["--delta"].asLong() ?: 1;    // at least one

  std::vector ids     = parse_ids(args["--version"].asStringList());
  std::vector threads = parse_n_threads(args["THREADS"].asStringList());

  auto aos_a = load_graph<nw::graph::directedness::directed, int>(file);

  if (verbose) {
    aos_a.stream_stats();
  }

  auto graph = build_adjacency<0>(aos_a);
  auto weight = [](auto& e) -> auto& { return std::get<1>(e); };

  if (verbose) {
    graph.stream_stats();
  }

  if (debug) {
    graph.stream_indices();
  }

  std::vector<vertex_id_t<decltype(graph)>> sources;
  if (args["--sources"]) {
    sources = load_sources_from_file(graph, args["--sources"].asString());
    assert(size_t(trials * iterations) <= sources.size());
  } else if (args["-r"]) {
    sources.resize(trials);
    std::fill(sources.begin(), sources.end(), args["-r"].asLong());
  } else {
    sources = build_random_sources(graph, trials, args["--seed"].asLong());
  }

  Times times;

  for (auto&& thread : threads) {
    auto _ = set_n_threads(thread);
    for (auto&& id : ids) {
      for (int i = 0; i < trials; ++i) {
        std::cout << "running version: " << id << " trial: " << i << "\n";

        double time = 0;
        for (int j = 0; j < iterations; ++j) {
          auto source = sources[i * iterations + j];
          if (verbose) {
            std::cout << "iteration: " << j << " source: " << source << "\n";
          }
          auto [t, v] = sssp(id, graph, source, delta, weight, [&](auto&& dist) {
            if (verify) {
              return SSSPVerifier(graph, source, std::forward<decltype(dist)>(dist), verbose, weight);
            }
            return true;
          });
          time += t;
        }
        times.append(file, id, thread, time);
      }
    }
  }

  times.print(std::cout);

  if (args["--log"]) {
    auto file   = args["--log"].asString();
    bool header = args["--log-header"].asBool();
    log("sssp", file, times, header, "Time(s)");
  }

  return 0;
}
