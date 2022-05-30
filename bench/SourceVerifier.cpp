/**
 * @file SourceVerifier.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   liux238
 *
 */

static constexpr const char USAGE[] =
    R"(sources : BGL17 gap sources file verifier.
  Usage:
      sources (-h | --help)
      sources --version
      sources -f FILE -s FILE -i NUM -n NUM [--seed NUM]

  Options:
      -h, --help          show this screen
      --version           driver version number
      -f FILE             input graph
      -s FILE             input source list to verify
      -i NUM              number of iteration
      -n NUM              number of trials
      --seed NUM          random seed [default: 27491095]
)";

#include "common.hpp"
#include "nwgraph/io/mmio.hpp"
#include <docopt.h>
#include <random>

using namespace nw::graph::bench;
using namespace nw::graph;
using namespace nw::util;

int main(int argc, char* const argv[]) {
  std::vector strings = std::vector<std::string>{argv + 1, argv + argc};
  std::map    args    = docopt::docopt(USAGE, strings, true);

  int64_t iterations = args["-i"].asLong();
  int64_t trials     = args["-n"].asLong();
  int64_t seed       = args["--seed"].asLong();

  auto sources = read_mm_vector<default_vertex_id_type>(args["-s"].asString());

  if (int64_t(sources.size()) != trials * iterations) {
    std::cerr << "Read " << sources.size() << " sources from " << args["-s"].asString() << " but expected " << trials * iterations << "\n";
    return 0;
  }

  auto aos    = load_graph<directedness::directed>(args["-f"].asString());
  auto graph  = build_adjacency<1>(aos);
  auto random = build_random_sources(graph, sources.size(), seed);

  std::cout << "Testing sources\n";
  int64_t failures = 0;
  for (size_t i = 0, e = sources.size(); i < e; ++i) {
    if (sources[i] != random[i]) {
      std::cerr << "sources[i]=" << sources[i] << " but random[i]=" << random[i] << "\n";
      ++failures;
    }
  }
  std::cout << failures << " failures\n";

  return 0;
}
