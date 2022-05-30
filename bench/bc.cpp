/**
 * @file bc.cpp
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
    R"(bc2.exe : BGL17 betweenness centrality benchmark driver.
  Usage:
      bc2.exe (-h | --help)
      bc2.exe -f FILE [-r NODE | -s FILE ] [-i NUM] [-n NUM] [--seed NUM] [--version ID...] [--log FILE] [--log-header] [-dvV] [THREADS]...

  Options:
      -h, --help              show this screen
      -f FILE                 input file path
      -i NUM                  number of iteration [default: 1]
      -n NUM                  number of trials [default: 1]
      -r NODE                 start from node r (default is random)
      -s, --sources FILE      sources file
      --seed NUM              random seed [default: 27491095]
      --version ID            algorithm version to run [default: 5]
      --log FILE              log times to a file
      --log-header            add a header to the log file
      -d, --debug             run in debug mode
      -v, --verify            verify results
      -V, --verbose           run in verbose mode
)";

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

#include "Log.hpp"
#include "nwgraph/algorithms/betweenness_centrality.hpp"
#include "nwgraph/experimental/algorithms/betweenness_centrality.hpp"
#include "common.hpp"
#include <docopt.h>

using namespace nw::graph::bench;
using namespace nw::graph;
using namespace nw::util;

using score_t = float;
using accum_t = double;

template <typename Vector>
void print_n_ranks(const Vector& centrality, size_t n) {
  auto perm = proxysort<size_t>(centrality, std::greater<float>());
  for (size_t i = 0; i < 10; ++i) {
    std::cout << std::to_string(perm[i]) + ": " << std::to_string(centrality[perm[i]]) << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::vector strings = std::vector<std::string>(argv + 1, argv + argc);
  std::map    args    = docopt::docopt(USAGE, strings, true);

  // Read the options
  bool        verify     = args["--verify"].asBool();
  bool        verbose    = args["--verbose"].asBool();
  bool        debug      = args["--debug"].asBool();
  long        trials     = args["-n"].asLong() ?: 1;
  long        iterations = args["-i"].asLong() ?: 1;
  std::string file       = args["-f"].asString();

  std::vector ids     = parse_ids(args["--version"].asStringList());
  std::vector threads = parse_n_threads(args["THREADS"].asStringList());

  auto aos_a = load_graph<nw::graph::directedness::directed>(file);

  if (verbose) {
    aos_a.stream_stats();
  }

  auto graph = build_adjacency<1>(aos_a);

  if (verbose) {
    graph.stream_stats();
  }

  if (debug) {
    graph.stream_indices();
  }

  // These are one set from web
  // std::vector<vertex_id_type> sources_vector = { 3355244, 33831269, 45124744, 16137877 };

  // Our sources could come from a file,
  using vertex_id_type = typename graph_traits<decltype(graph)>::vertex_id_type;

  std::vector<vertex_id_type> sources;
  if (args["--sources"]) {
    sources = load_sources_from_file(graph, args["--sources"].asString(), trials * iterations);
  } else if (args["-r"]) {
    sources.resize(trials * iterations);
    std::fill(sources.begin(), sources.end(), args["-r"].asLong());
  } else {
    sources = build_random_sources(graph, trials * iterations, args["--seed"].asLong());
  }

  Times times;

  // for each thread count
  //   for each algorithm id
  //     for each trial
  for (auto&& thread : threads) {
    auto _ = set_n_threads(thread);
    for (auto id : ids) {
      if (verbose) {
        std::cout << "version " << id << "\n";
      }

      for (int i = 0; i < trials; ++i) {
        std::vector<vertex_id_type> trial_sources(&sources[iterations * i], &sources[iterations * (i + 1)]);
        auto&& [centrality] = times.record(file, id, thread, [&]() -> std::vector<score_t> {
          switch (id) {
            case 0:
              return bc2_v0<decltype(graph), score_t, accum_t>(graph, trial_sources);
            case 1:
              return bc2_v1<decltype(graph), score_t, accum_t>(graph, trial_sources);
            case 2:
              return bc2_v2<decltype(graph), score_t, accum_t>(graph, trial_sources);
            case 3:
              return bc2_v3<decltype(graph), score_t, accum_t>(graph, trial_sources);
            case 4:
              return bc2_v4<score_t, accum_t>(graph, trial_sources, thread);
            case 5:
              return brandes_bc<score_t, accum_t>(graph, trial_sources, thread);
            case 6:
              return brandes_bc(graph);
            case 7:
              return approx_betweenness_brandes(graph, trial_sources);
            case 8:
              return exact_brandes_bc<score_t, accum_t, decltype(graph)>(graph, thread);
            default:
              std::cerr << "Invalid BC version " << id << "\n";
              return {};
          }
        });

        if (verify) {
          BCVerifier<score_t, accum_t>(graph, trial_sources, centrality);
        }
      }
    }
  }

  times.print(std::cout);

  if (args["--log"]) {
    auto file   = args["--log"].asString();
    bool header = args["--log-header"].asBool();
    log("bc", file, times, header, "Time(s)", "Iterations");
  }

  return 0;
}
