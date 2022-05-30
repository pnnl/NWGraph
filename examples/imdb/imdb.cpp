/**
 * @file imdb.cpp
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

#include <fstream>
#include <iostream>

#include <deque>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "xtensor/xcsv.hpp"

#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/util/timer.hpp"

int main() {
  const int s = 8;

  nw::util::timer               t0("load titles");
  std::string                   title_basics_tsv = "../data/title.basics.tsv";
  std::ifstream                 title_basics_stream(title_basics_tsv);
  auto                          titles     = xt::load_csv<std::string>(title_basics_stream, '\t');
  auto                          titles_shp = titles.shape();
  std::map<std::string, size_t> titles_map;
  for (size_t i = 0; i < titles_shp[0]; ++i) {
    if (titles(i, 1) == "movie") {
      titles_map[titles(i, 0)] = i;
    }
  }
  t0.stop();
  std::cout << t0 << std::endl;

  nw::util::timer               t1("load names");
  std::string                   name_basics_tsv = "../data/name.basics.tsv";
  std::ifstream                 name_basics_stream(name_basics_tsv);
  auto                          names     = xt::load_csv<std::string>(name_basics_stream, '\t');
  auto                          names_shp = names.shape();
  std::map<std::string, size_t> names_map;
  for (size_t i = 0; i < names_shp[0]; ++i) {
    names_map[names(i, 0)] = i;
  }
  t1.stop();
  std::cout << t1 << std::endl;

  nw::util::timer t2("load hypergraph");

  std::string   title_principals_tsv = "../data/title.principals.tsv";
  std::ifstream title_principals_stream(title_principals_tsv);
  auto          title_principals = xt::load_csv<std::string>(title_principals_stream, '\t');
  auto          shp              = title_principals.shape();

  t2.stop();
  std::cout << t2 << std::endl;

  nw::util::timer t3("build hypergraph");

  nw::graph::edge_list<nw::graph::directedness::directed> edges;
  edges.open_for_push_back();

  size_t title_counter = 0;
  size_t name_counter  = 0;
  for (size_t i = 1; i < shp[0]; ++i) {
    if (title_principals(i, 3) == "actor" || title_principals(i, 3) == "actress") {

      auto title = title_principals(i, 0);
      auto name  = title_principals(i, 2);

      if (name == "nm0837064") {
        auto idx = titles_map[title];
        auto aa  = titles(idx, 2);
        std::cout << title << " " << aa << std::endl;
      }
#if 0
      if (titles_map.find(title) == titles_map.end()) {
	titles_map[title] = title_counter++;
      }
      if (names_map.find(name) == names_map.end()) {
	names_map[name] = name_counter++;
      }
#endif
      edges.push_back(titles_map[title], names_map[name]);
    }
  }
  edges.close_for_push_back();

  t3.stop();
  std::cout << t3 << std::endl;
  edges.stream_stats();

  nw::util::timer t4("build biadjacencies");

  auto G = nw::graph::adjacency<0>(edges);
  auto H = nw::graph::adjacency<1>(edges);

  t4.stop();
  std::cout << t4 << std::endl;

  nw::util::timer t5("build s_overlap");

  nw::graph::edge_list<nw::graph::directedness::undirected, size_t> s_overlap;
  s_overlap.open_for_push_back();

  for (size_t i = 0; i < H.size(); ++i) {

    if ((i % 8192) == 0) {
      std::cout << i << std::endl;
    }

    for (auto&& [k] : H[i]) {
      for (auto&& [j] : G[k]) {
        if (j > i) {
          s_overlap.push_back(i, j, k);
        }
      }
    }
  }

  s_overlap.close_for_push_back();

  t5.stop();
  std::cout << t5 << std::endl;

  nw::util::timer t6("build s_overlap adjacency");

  auto L = nw::graph::adjacency<0, size_t>(s_overlap);

  t6.stop();
  std::cout << t6 << std::endl;

  // Kevin Bacon is nm0000102
  // David Suchet is nm0837064
  // Kyra Sedgwick is nm0001718

  size_t kevin_bacon   = names_map["nm0000102"];
  size_t david_suchet  = names_map["nm0837064"];
  size_t kyra_sedgwick = names_map["nm0001718"];

  std::vector<size_t> distance(L.size());
  std::vector<size_t> parents(L.size());
  std::vector<size_t> together_in(L.size());

  for (auto&& [u, v, k] : nw::graph::bfs_edge_range(L, kevin_bacon)) {
    distance[v]    = distance[u] + 1;
    parents[v]     = u;
    together_in[v] = k;
  }

  std::cout << "Kevin Bacon has a Bacon number of " << distance[kevin_bacon] << std::endl;

  std::cout << "Kyra Sedgwick has a Bacon number of " << distance[kyra_sedgwick] << std::endl;
  size_t d = distance[kyra_sedgwick];
  while (kyra_sedgwick != kevin_bacon) {
    std::cout << names(kyra_sedgwick, 1) << " starred with " << names(parents[kyra_sedgwick], 1) << " in "
              << titles(together_in[kyra_sedgwick], 2) << std::endl;
    kyra_sedgwick = parents[kyra_sedgwick];
    if (d-- == 0) {
      break;
    }
  }

  std::cout << "David Suchet has a Bacon number of " << distance[david_suchet] << std::endl;
  d = distance[david_suchet];
  while (david_suchet != kevin_bacon) {
    std::cout << names(david_suchet, 1) << " starred with " << names(parents[david_suchet], 1) << " in " << titles(together_in[david_suchet], 2)
              << std::endl;
    david_suchet = parents[david_suchet];
    if (d-- == 0) {
      break;
    }
  }

  return 0;
}
