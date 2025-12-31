/**
 * @file apb_common.hpp
 *
 * @brief Common utilities for abstraction penalty benchmarks.
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */

#ifndef NW_GRAPH_APB_COMMON_HPP
#define NW_GRAPH_APB_COMMON_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/timer.hpp"

namespace nw::graph::apb {

/**
 * @brief Run a benchmark with setup and timed work phases.
 *
 * @tparam Setup  Callable for per-trial setup (not timed)
 * @tparam Work   Callable for timed work
 * @param name    Benchmark name for output
 * @param ntrial  Number of trials to run
 * @param setup   Setup function called before each trial (not timed)
 * @param work    Work function to time
 * @return Average time per trial in milliseconds
 */
template <typename Setup, typename Work>
double bench(const std::string& name, size_t ntrial, Setup&& setup, Work&& work) {
  double             time = 0;
  nw::util::ms_timer t(name);
  for (size_t i = 0; i < ntrial; ++i) {
    setup();
    t.start();
    work();
    t.stop();
    time += t.elapsed();
  }
  std::cout << t.name() << " " << time / ntrial << " ms" << std::endl;
  return time / ntrial;
}

/**
 * @brief Run a benchmark without setup phase.
 *
 * @tparam Work   Callable for timed work
 * @param name    Benchmark name for output
 * @param ntrial  Number of trials to run
 * @param work    Work function to time
 * @return Average time per trial in milliseconds
 */
template <typename Work>
double bench(const std::string& name, size_t ntrial, Work&& work) {
  return bench(name, ntrial, [] {}, std::forward<Work>(work));
}

/**
 * @brief Load a graph from file (Matrix Market or serialized BGL17 format).
 *
 * @tparam Directedness  Graph directedness (directed or undirected)
 * @tparam Attributes    Edge attribute types
 * @param file           Path to graph file
 * @return edge_list containing the graph
 */
template <nw::graph::directedness Directedness, class... Attributes>
nw::graph::edge_list<Directedness, Attributes...> load_graph(const std::string& file) {
  std::ifstream in(file);
  std::string   type;
  in >> type;

  if (type == "BGL17") {
    nw::util::life_timer                                _(std::string("deserialize ") + file);
    nw::graph::edge_list<Directedness, Attributes...> el(0);
    el.deserialize(file);
    return el;
  } else if (type == "%%MatrixMarket") {
    nw::util::life_timer _(std::string("read_mm ") + file);
    return nw::graph::read_mm<Directedness, Attributes...>(file);
  } else {
    std::cerr << "Error: Unrecognized graph file format: " << file << "\n";
    std::cerr << "Expected 'BGL17' or '%%MatrixMarket' header\n";
    exit(1);
  }
}

/**
 * @brief Print usage message and exit.
 *
 * @param prog  Program name (argv[0])
 * @param msg   Optional additional message
 */
inline void usage(const std::string& prog, const std::string& msg = "") {
  if (!msg.empty()) {
    std::cerr << "Error: " << msg << "\n";
  }
  std::cerr << "Usage: " << prog << " -f FILE [-n NTRIALS] [-v] [-d]\n";
  std::cerr << "  -f, --file FILE    Input graph file (Matrix Market or BGL17)\n";
  std::cerr << "  -n, --ntrials N    Number of trials [default: 1]\n";
  std::cerr << "  -v, --verbose      Verbose output\n";
  std::cerr << "  -d, --debug        Debug output\n";
  exit(1);
}

/**
 * @brief Simple command-line argument parser for APB benchmarks.
 */
struct Args {
  std::string file;
  size_t      ntrial  = 1;
  bool        verbose = false;
  bool        debug   = false;

  Args(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
      std::string arg(argv[i]);

      if (arg == "-f" || arg == "--file" || arg == "-i" || arg == "--edgelistfile") {
        if (++i >= argc) usage(argv[0], "Missing argument for " + arg);
        file = argv[i];
      } else if (arg == "-n" || arg == "--ntrials" || arg == "--ntrial") {
        if (++i >= argc) usage(argv[0], "Missing argument for " + arg);
        ntrial = std::stoul(argv[i]);
      } else if (arg == "-v" || arg == "--verbose") {
        verbose = true;
      } else if (arg == "-d" || arg == "--debug") {
        debug = true;
      } else if (arg == "-h" || arg == "--help") {
        usage(argv[0]);
      } else {
        usage(argv[0], "Unknown option: " + arg);
      }
    }

    if (file.empty()) {
      usage(argv[0], "Input file required (-f)");
    }
  }
};

}  // namespace nw::graph::apb

#endif  // NW_GRAPH_APB_COMMON_HPP
