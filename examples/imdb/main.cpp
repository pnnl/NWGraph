/**
 * @file main.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
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

#include "containers/compressed.hpp"
#include "containers/edge_list.hpp"
#include "io/mmio.hpp"
#include "util/timer.hpp"

int main(int argc, char* argv[]) {

#if 0
  nw::util::timer               t0("load titles");
  std::string                   title_basics_tsv = "../data/title.basics.tsv";
  std::ifstream                 title_basics_stream(title_basics_tsv);
  auto                          titles     = xt::load_csv<std::string>(title_basics_stream, '\t');
  auto                          titles_shp = titles.shape();
  std::map<std::string, size_t> titles_map;
  for (size_t i = 0; i < titles_shp[0]; ++i) {
    titles_map[titles(i, 0)] = i;
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
  auto   shp   = title_principals.shape();

  t2.stop();
  std::cout << t2 << std::endl;

  nw::util::timer t3("build hypergraph");

  nw::graph::edge_list<nw::graph::directed> edges;
  edges.open_for_push_back();

  size_t title_counter = 0;
  size_t name_counter  = 0;
  for (size_t i = 1; i < shp[0]; ++i) {
    auto title = title_principals(i, 0);
    auto name  = title_principals(i, 2);
    if (titles_map.find(title) == titles_map.end()) {
      titles_map[title] = title_counter++;
    }
    if (names_map.find(name) == names_map.end()) {
      names_map[name] = name_counter++;
    }
    edges.push_back(titles_map[title], names_map[name]);
  }
  edges.close_for_push_back();

  t3.stop(); 
  std::cout << t3 << std::endl;
  edges.stream_stats();


  //  nw::graph::edge_list<nw::graph::directed> edges;
  //  edges.deserialize("foo.nw.graph");

#if 0
  nw::util::timer t3a("relabel");
  edges.relabel_by_degree<0>("ascending");
  t3a.stop(); 
  std::cout << t3a << std::endl;
#endif
#endif

  nw::graph::edge_list<nw::graph::directed> edges = nw::graph::read_mm<nw::graph::directed>(argv[1]);

  nw::util::timer t4("build biadjacencies");

  auto H = nw::graph::adjacency<0>(edges);
  auto G = nw::graph::adjacency<1>(edges);

  t4.stop();
  std::cout << t4 << std::endl;

  nw::util::timer t5("build s_overlap");

  nw::graph::edge_list<nw::graph::undirected> s_overlap;
  s_overlap.open_for_push_back();

  size_t s = 1;

#pragma omp parallel for schedule(dynamic, 64)

  for (size_t i = 0; i < H.size(); ++i) {

    if ((i % 8192) == 0) {
      std::cout << i << std::endl;
    }

    std::map<size_t, size_t> K;

    for (auto&& [k] : H[i]) {
      for (auto&& [j] : G[k]) {
        if (j > i) {
          K[j]++;
        }
      }
    }

#pragma omp critical
    for (auto&& [key, val] : K) {
      if (val >= s) {
        s_overlap.push_back(i, key);
      }
    }
  }
  s_overlap.close_for_push_back();

  t5.stop();
  std::cout << t5 << std::endl;

  nw::util::timer t6("build s_overlap adjacency");

  auto L = nw::graph::adjacency<0>(s_overlap);

  t6.stop();
  std::cout << t6 << std::endl;

#if 0
  // Kevin Bacon is nm0000102
  // David Suchet is nm0837064
  // Kyra Sedgwick is nm0001718

  size_t kevin_bacon = names_map["nm0000102"];
  size_t david_suchet = names_map["nm0837064"];
  size_t kyra_sedgwick = names_map["nm0001718"];

  {
    using vertex_id_type = nw::graph::vertex_id_type;
    
    vertex_id_type root = kevin_bacon;

    std::deque<vertex_id_type>  q1, q2;
    std::vector<vertex_id_type> level(L.max() + 1, std::numeric_limits<vertex_id_type>::max());
    std::vector<vertex_id_type> parents(L.max() + 1, std::numeric_limits<vertex_id_type>::max());
    size_t                   lvl = 0;
    
    q1.push_back(root);
    level[root]   = lvl++;
    parents[root] = root;
    
    auto g = L.begin();
    
    while (!q1.empty()) {
      
      std::for_each(q1.begin(), q1.end(), [&](vertex_id_type u) {
	std::for_each(g[u].begin(), g[u].end(), [&](auto&& x) {
	  vertex_id_type v = std::get<0>(x);
	  if (level[v] == std::numeric_limits<vertex_id_type>::max()) {
	    q2.push_back(v);
	    level[v]   = lvl;
	    parents[v] = u;
	  }
	});
      });
      std::swap(q1, q2);
      q2.clear();
      ++lvl;
    }
    
    std::cout << "Kevin Bacon: " << level[kevin_bacon] << std::endl;
    std::cout << "Kyra Sedgwick: " << level[kyra_sedgwick] << std::endl;
    std::cout << "David Suchet: " << level[david_suchet] << std::endl;
  }
#endif

  // \For {$i \in U$}
  // \State{$\boldsymbol{K} \gets \varnothing$}
  // \For {$k \in \boldsymbol{H}.Adj[i]$} \Comment{$H_{ik}$}
  // \For {$j \in \boldsymbol{H}^{\top}.Adj[k]$} \Comment{$H^{\top}_{kj}$}
  // \State{$\boldsymbol{K} \gets \boldsymbol{K} \cup j$}
  // \EndFor
  // \EndFor
  // \State{$\displaystyle  \boldsymbol{L} \gets \boldsymbol{L} \cup (i, j), j\in \boldsymbol{K}(\boldsymbol{K}\geq s)$} \Comment{Use map}
  // \EndFor

#if 0
  std::ostream& mtx_file = std::cout;
  mtx_file << "%%MatrixMarket matrix coordinate pattern general" << std::endl;
  mtx_file << titles.size() << " " << names.size() << " " << edges.size() << std::endl;

  for (auto&& j : edges) {
    mtx_file << std::get<0>(j) << " " << std::get<1>(j) << std::endl;
  }

  auto title_basics     = xt::load_csv<std::string>(title_basics_stream, '\t');
  auto name_basics      = xt::load_csv<std::string>(name_basics_stream, '\t');

  auto shp = name_basics.shape();
  std::cout << shp[0] << " rows and " << shp[1] << " columns" << std::endl;
  std::cout << name_basics.size() << " " << name_basics.rank << std::endl;

  auto bb = name_basics(0, 1);
  auto cc = name_basics(22, 1);
  auto dd = name_basics(23, 1);
  auto ee = name_basics(24, 1);
  std::cout << bb << " " << cc << " " << dd << " " << ee << std::endl;
#endif

  return 0;
}
