/**
 * @file pr.cpp
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
    R"(pr.exe: BGL17 page rank benchmark driver.
  Usage:
      pr.exe (-h | --help)
      pr.exe [--version ID...] -f FILE... [-i NUM] [-t NUM] [-n NUM] [-dvV] [--log FILE] [--log-header] [THREADS]...

  Options:
      -h, --help          show this screen
      --version ID        algorithm version to run [default: 11]
      -f FILE             input file path
      -i NUM              maximum iteration [default: 20]
      -t NUM              tolerance [default: 1e-4]
      -n NUM              number of trials [default: 1]
      --log FILE          log times to a file
      --log-header        add a header to the log file
      -d, --debug         run in debug mode
      -v, --verify        verify results
      -V, --verbose       run in verbose mode
)";

#include "Log.hpp"
#include "nwgraph/algorithms/page_rank.hpp"
#include "nwgraph/experimental/algorithms/page_rank.hpp"
#include "common.hpp"
#include <docopt.h>

using namespace nw::graph::bench;
using namespace nw::graph;
using namespace nw::util;

template <typename Vector>
void print_n_ranks(const Vector& rankings, size_t n) {
  auto perm = proxysort<size_t>(rankings, std::greater<float>());
  for (size_t i = 0; i < 10; ++i) {
    std::cout << std::to_string(perm[i]) + ": " << std::to_string(rankings[perm[i]]) << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::vector<std::string> strings(argv + 1, argv + argc);
  auto                     args = docopt::docopt(USAGE, strings, true);

  // Read the options
  bool  verify    = args["--verify"].asBool();
  bool  verbose   = args["--verbose"].asBool();
  bool  debug     = args["--debug"].asBool();
  long  trials    = args["-n"].asLong() ?: 1;
  long  max_iters = args["-i"].asLong() ?: 1;
  float tolerance = std::stof(args["-t"].asString());

  std::vector files   = args["-f"].asStringList();
  std::vector ids     = parse_ids(args["--version"].asStringList());
  std::vector threads = parse_n_threads(args["THREADS"].asStringList());

  Times times;

  for (auto&& file : files) {
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

    auto degrees = build_degrees(graph);

    std::vector<float> rankings(graph.size());

    for (auto thread : threads) {
      auto _ = set_n_threads(thread);
      for (auto id : ids) {
        for (size_t j = 0, e = trials; j < e; ++j) {
          times.record(file, id, thread, [&] {
            switch (id) {
              case 0:
                // page_rank_range_for(graph, rankings, 0.85f, tolerance, max_iters);
                break;

              case 1:
                page_rank_v1(graph, degrees, rankings, 0.85f, tolerance, max_iters);
                break;

              case 2:
                page_rank_v2(graph, degrees, rankings, 0.85f, tolerance, max_iters);
                break;

              case 3:
                page_rank_v3(graph, degrees, rankings, 0.85f, tolerance, max_iters);
                break;

              case 4:
                page_rank_v4(graph, degrees, rankings, 0.85f, tolerance, max_iters, thread);
                break;

              case 6:
                page_rank_v6(graph, degrees, rankings, 0.85f, tolerance, max_iters);
                break;

              case 7:
                page_rank_v7(graph, degrees, rankings, 0.85f, tolerance, max_iters);
                break;

              case 8:
                page_rank_v8(graph, degrees, rankings, 0.85f, tolerance, max_iters);
                break;

              case 9:
                page_rank_v9(graph, degrees, rankings, 0.85f, tolerance, max_iters, thread);
                break;

              case 10:
                page_rank_v10(graph, degrees, rankings, 0.85f, tolerance, max_iters, thread);
                break;

              case 11:
                page_rank(graph, degrees, rankings, 0.85f, tolerance, max_iters, thread);
                break;

              case 12:
                page_rank_v12(graph, degrees, rankings, 0.85f, tolerance, max_iters, thread);
                break;

              case 13:
                page_rank_v13(graph, degrees, rankings, 0.85f, tolerance, max_iters, thread);
                break;

              case 14:
                page_rank_v14(graph, degrees, rankings, 0.85f, tolerance, max_iters);
                break;

              default:
                std::cerr << "Unknown version id " << id << std::endl;
                break;
            }
          });
        }

        if (verify) {
          std::cout << "Verifying\n";
          print_n_ranks(rankings, 10);
        }
      }
    }
  }

  times.print(std::cout);

  if (args["--log"]) {
    auto file   = args["--log"].asString();
    bool header = args["--log-header"].asBool();
    log("pr", file, times, header, "Time(s)", "Tolerance");
  }

  return 0;
}
