/**
 * @file ProcessEdgelist.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *   liux238
 *
 */

static constexpr const char USAGE[] =
    R"(process_edge_list : BGL17 graph input handler.
  Usage:
      process_edge_list (-h | --help)
      process_edge_list --version
      process_edge_list [-d <file>] [-u <file>] <input>

  Options:
      -h, --help      show this screen
      --version       driver version
      -d <file>       generate directed graph
      -u <file>       generate undirected graph
)";

#include "common.hpp"
#include "nwgraph/graph_base.hpp"
#include <docopt.h>

using namespace nw::graph;
using namespace nw::util;

template <directedness Directed, class... Attributes>
static void eval(std::string input, std::string output) {
  auto&& [read, aos] = nw::graph::bench::time_op([&] {
    std::cout << "Reading " << input << "\n";
    return read_mm<Directed, Attributes...>(input);
  });
  std::cout << read << " seconds\n";

  auto&& [write] = nw::graph::bench::time_op([&, aos = aos] {
    std::cout << "Writing " << output << "\n";
    aos.serialize(output);
  });
  std::cout << write << " seconds\n";
}

int main(int argc, char* const argv[]) {
  std::vector strings = std::vector<std::string>{argv + 1, argv + argc};
  std::map    args    = docopt::docopt(USAGE, strings, true);
  std::string input   = args["<input>"].asString();

  if (args["-d"]) {
    std::cout << "Processing directed graph\n";
    eval<nw::graph::directedness::directed, int>(input, args["-d"].asString());
  }

  if (args["-u"]) {
    std::cout << "Processing directed graph\n";
    eval<nw::graph::directedness::undirected, int>(input, args["-u"].asString());
  }

  return 0;
}
