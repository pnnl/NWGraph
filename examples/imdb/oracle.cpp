/**
 * @file oracle.cpp
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

#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/algorithms/betweenness_centrality.hpp"
#include "nwgraph/algorithms/page_rank.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/util/proxysort.hpp"
#include "nwgraph/util/timer.hpp"

std::string delink(const std::string& link) {
  auto opening = link.find("[[");
  if (opening == std::string::npos) {
    return link;
  }
  auto closing = link.find("]]");
  if (opening == std::string::npos) {
    return link + " hm";
  }
  auto delinked = link.substr(opening + 2, closing - opening - 2);
  auto bar      = delinked.find("|");
  if (bar == std::string::npos) {
    return delinked;
  }
  return delinked.substr(bar + 1);
}

template <class Graph>
auto build_random_sources(Graph&& graph, size_t n, long seed) {
  using Id = typename nw::graph::vertex_id_t<std::decay_t<Graph>>;

  auto sources = std::vector<Id>(n);
  auto degrees = graph.degrees();
  auto gen     = std::mt19937(seed);
  auto dis     = std::uniform_int_distribution<Id>(0, graph.max());

  for (auto& id : sources) {
    for (id = dis(gen); degrees[id] == 0; id = dis(gen)) {
    }
  }
  return sources;
}

int main() {
  std::ifstream ifs("../data/oracle.json");

  nw::util::timer   t2("read oracle.json");
  std::vector<json> jsons;
  while (!ifs.eof() && ifs.peek() != EOF) {
    std::string str;
    std::getline(ifs, str);
    json jf = json::parse(str);

    jsons.emplace_back(jf);
  }
  t2.stop();
  std::cout << t2 << std::endl;

  nw::util::timer t3("build hypergraph");

  std::map<std::string, size_t> titles_map;
  std::map<std::string, size_t> names_map;
  std::vector<std::string>      titles;
  std::vector<std::string>      names;

  nw::graph::edge_list<nw::graph::directedness::directed> edges;
  edges.open_for_push_back();

  size_t title_counter = 0;
  size_t name_counter  = 0;

  for (auto& j : jsons) {
    auto title = j["title"];

    if (titles_map.find(title) == titles_map.end()) {
      titles.emplace_back(title);
      titles_map[title] = title_counter++;
      assert(titles.size() == title_counter);
    }

    for (auto& k : j["cast"]) {
      auto name = delink(k);

      if (names_map.find(name) == names_map.end()) {

        names.emplace_back(name);
        names_map[name] = name_counter++;

        assert(names.size() == name_counter);
        assert(names[name_counter - 1] == name);
      }

      edges.push_back(titles_map[title], names_map[name]);
    }
  }
  edges.close_for_push_back();

  t3.stop();
  std::cout << t3 << std::endl;

  nw::util::timer t3a("verify names and titles");

  for (auto&& [k, v] : names_map) {
    if (names[v] != k) {
      std::cout << names[v] << " != " << k << " ( " << v << " )" << std::endl;
    }
  }
  for (auto&& [k, v] : titles_map) {
    if (titles[v] != k) {
      std::cout << titles[v] << " != " << k << " ( " << v << " )" << std::endl;
    }
  }

  t3a.stop();
  std::cout << t3a << std::endl;

  nw::util::timer t4("build biadjacencies");

  // NB: edge_list may have different max index values (min, max) for each "side"

  // edges are pairs of ( title, name )
  auto G = nw::graph::adjacency<0>(edges);    // title: names
  auto H = nw::graph::adjacency<1>(edges);    // names: title

  t4.stop();
  std::cout << t4 << std::endl;

  nw::util::timer t5("build s_overlap");

  nw::graph::edge_list<nw::graph::directedness::undirected, size_t> s_overlap;
  s_overlap.open_for_push_back();

  for (size_t i = 0; i < H.size(); ++i) {    // foreach name
    for (auto&& [k] : H[i]) {                //   foreach title
      for (auto&& [j] : G[k]) {              //     foreach name
        if (j > i) {
          s_overlap.push_back(i, j, k);    //       edge from i -- j with value k
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

  size_t kevin_bacon = names_map["Kevin Bacon"];

  std::vector<size_t> distance(L.size());
  std::vector<size_t> parents(L.size());
  std::vector<size_t> together_in(L.size());

  for (auto&& [u, v, k] : nw::graph::bfs_edge_range(L, kevin_bacon)) {
    distance[v]    = distance[u] + 1;
    parents[v]     = u;
    together_in[v] = k;
  }

  auto path_to_bacon_idx = [&](size_t the_actor) {
    std::cout << names[the_actor] << " has a Bacon number of " << distance[the_actor] << std::endl;

    size_t d = distance[the_actor];
    while (the_actor != kevin_bacon) {
      std::cout << "  " << names[the_actor] << " starred with " << names[parents[the_actor]] << " in " << titles[together_in[the_actor]]
                << std::endl;
      the_actor = parents[the_actor];
      if (d-- == 0) {
        break;
      }
    }
  };

  auto path_to_bacon = [&](const std::string& name) {
    size_t the_actor = names_map[name];
    if (names[the_actor] != name) {
      std::cout << names[the_actor] << " != " << name << " ( " << the_actor << " )" << std::endl;
    }
    path_to_bacon_idx(the_actor);
  };

  std::cout << std::endl;

  size_t max_distance = *(std::max_element(distance.begin(), distance.end()));
  std::cout << "Furthest distance is " << max_distance << std::endl;

  for (size_t i = 0; i < distance.size(); ++i) {
    if (distance[i] == max_distance) {
      path_to_bacon_idx(i);
    }
  }

  std::cout << std::endl;
  path_to_bacon("Kevin Bacon");
  path_to_bacon("Kyra Sedgwick");
  path_to_bacon("David Suchet");
  path_to_bacon("Julie Kavner");
  path_to_bacon("Samuel L. Jackson");
  path_to_bacon("William Shatner");
  path_to_bacon("Oona O'Neill");

#if 0
  path_to_bacon("Danica McKellar");
  path_to_bacon("William Rufus Shafter");


  if constexpr (false) {
    auto                L_t = nw::graph::adjacency<1, size_t>(s_overlap);
    std::vector<double> page_rank(L.size());

    page_rank_v8(L_t, L.degrees(), page_rank, 0.85, 1.e-4, 20);

    auto perm = nw::util::proxysort<size_t>(page_rank, std::greater<float>());
    for (size_t i = 0; i < 10; ++i) {
      std::cout << std::to_string(perm[i]) + ": " << names[perm[i]] << std::endl;
    }
  }

  if constexpr (false) {
    auto sources = build_random_sources(L, 1024, 98195);
    auto tweens = nw::graph::bc2_v2<decltype(L), double, double>(L, sources);
    
    auto perm = nw::util::proxysort<size_t>(tweens, std::greater<float>());
    for (size_t i = 0; i < 10; ++i) {
      std::cout << std::to_string(perm[i]) + ": " << names[perm[i]] << std::endl;
    }
  }
#endif

  return 0;
}
