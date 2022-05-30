/**
 * @file containers.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Tony Liu
 *
 */

static constexpr const char USAGE[] =
    R"(containers.exe: containers abstraction penalty benchmark driver.
  Usage:
      containers.exe (-h | --help)
      containers.exe -f FILE [-n NUM]  [--format FORMAT...] [-dvV] 

  Options:
      -h, --help            show this screen
      -f FILE               input file path
      -n NUM                number of trials [default: 1]
      --format FORMAT       specify which graph storage format [default: CSR]
      -d, --debug           run in debug mode
      -V, --verbose         run in verbose mode
)";

#include <iostream>
#include <docopt.h>

#include "nwgraph/build.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/volos.hpp"
#include "nwgraph/vovos.hpp"
#include "nwgraph/vofos.hpp"

#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"


using namespace nw::graph;
using namespace nw::util;

template <directedness Directedness, class... Attributes>
edge_list<Directedness, Attributes...> load_graph(std::string file) {
  std::ifstream in(file);
  std::string   type;
  in >> type;

  if (type == "BGL17") {
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

template <adjacency_list_graph Graph>
auto compress(edge_list<nw::graph::directedness::undirected, double>& A) {
  life_timer _(__func__);
  Graph      B(num_vertices(A));
  push_back_fill(A, B);
  return B;
}

template <adjacency_list_graph Graph, typename EdgeList>
void run_bench(int argc, char* argv[], EdgeList& el_a) {
  std::vector<std::string> strings(argv + 1, argv + argc);
  auto                     args = docopt::docopt(USAGE, strings, true);

  // Read the easy options
  bool verbose = args["--verbose"].asBool();
  bool debug   = args["--debug"].asBool();
  long ntrial  = args["-n"].asLong() ?: 1;


  auto graph = compress<Graph>(el_a);

  using vertex_id_type = vertex_id_t<Graph>;

  vertex_id_type     N = num_vertices(graph);
  std::vector<float> x(N), y(N);
  std::iota(x.begin(), x.end(), 0);
  {
    double               time = 0;
    ms_timer t2("iterator based for loop with iterator based for loop");
    for (long t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t2.start();

      vertex_id_type k = 0;
      for (auto i = graph.begin(); i != graph.end(); ++i) {
        for (auto j = (*i).begin(); j != (*i).end(); ++j) {
          y[k] += x[std::get<0>(*j)] * std::get<1>(*j);
        }
        ++k;
      }
      t2.stop();
      time += t2.elapsed();
    }
    std::cout << t2.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer t3("range based for loop with range based for loop with structured binding");
    for (long t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      t3.start();

      vertex_id_type k = 0;
      for (auto&& i : graph) {
        for (auto&& [j, v] : i) {
          y[k] += x[j] * v;
        }
        ++k;
      }
      t3.stop();
      time += t3.elapsed();
    }
    std::cout << t3.name() << " " << time / ntrial << " ms" << std::endl;

    time = 0;
    ms_timer ta("nested std::for_each auto&&");
    for (long t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      ta.start();

      vertex_id_type k = 0;
      std::for_each(graph.begin(), graph.end(), [&](auto&& nbhd) {
	  std::for_each(nbhd.begin(), nbhd.end(), [&] (auto&& elt) {
	    auto&& [j, v] = elt;
	    y[k] += x[j] * v;
	  });
	  ++k;
	});
      ta.stop();
      time += ta.elapsed();
    }
    std::cout << ta.name() << " " << time / ntrial << " ms" << std::endl;


    time = 0;
    ms_timer tb("nested range for with neighbor_range");
    for (long t = 0; t < ntrial; ++t) {
      std::fill(y.begin(), y.end(), 0);
      tb.start();

      vertex_id_type k = 0;

      for (auto&& [k, u_neighbors] : make_neighbor_range(graph)) {
        for (auto &&[j, v] : u_neighbors) {
          y[k] += x[j] * v;
        }
      }
      tb.stop();

      time += tb.elapsed();
    }
    std::cout << tb.name() << " " << time / ntrial << " ms" << std::endl;

  }
}

int main(int argc, char* argv[]) {
  std::vector<std::string> strings(argv + 1, argv + argc);
  auto                     args = docopt::docopt(USAGE, strings, true);

  // Read the more complex options
  std::string file = args["-f"].asString();
  std::cout << "processing " << file << "\n";

  auto el_a = load_graph<nw::graph::directedness::undirected, double>(file);

  std::vector formats = args["--format"].asStringList();
  for (auto& f : formats) {
    std::cout << f << " format" << std::endl;
    if ("CSR" == f) {
      run_bench<adjacency<0, double>>(argc, argv, el_a);
    } else if ("VOV" == f) {
      //      run_bench<vov<0, double>>(argc, argv, el_a);
      run_bench<std::vector<std::vector<std::tuple<uint32_t, double>>>>(argc, argv, el_a);
    } else if ("VOL" == f) {
      run_bench<adj_list<0, double>>(argc, argv, el_a);
    } else if ("VOF" == f) {
      run_bench<adj_flist<0, double>>(argc, argv, el_a);
    } else {
      std::cerr << "bad format" << std::endl;
      return -1;
    }
  }
  return 0;
}
