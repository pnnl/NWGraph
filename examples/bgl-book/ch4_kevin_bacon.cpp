/**
 * @file ch4_kevin_bacon.cpp
 *
 * @brief Six Degrees of Kevin Bacon (BGL Book Chapter 4.1)
 *
 * This example demonstrates BFS to compute the "Bacon number" - the number
 * of connections from any actor to Kevin Bacon through co-starring in movies.
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

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

auto parse_buffer(const std::string& buffer) {
  std::stringstream link(buffer);
  std::string       actor_one, movie_name, actor_two;
  getline(link, actor_one, ';');
  getline(link, movie_name, ';');
  getline(link, actor_two);
  return std::make_tuple(actor_one, movie_name, actor_two);
}

auto read_imdb(const std::string& path, std::map<std::string, size_t>& actor_id_map) {
  std::string buffer;
  size_t      actor_vertex_counter = 0;
  std::ifstream datastream(path);

  edge_list<directedness::undirected, std::string> imdb(0);
  imdb.open_for_push_back();
  while (getline(datastream, buffer)) {
    auto&& [actor_one, movie_name, actor_two] = parse_buffer(buffer);

    auto&& [key_val_one, inserted_one] = actor_id_map.insert({ actor_one, actor_vertex_counter });
    if (inserted_one) ++actor_vertex_counter;
    auto&& [dummy_one, index_one] = *key_val_one;

    auto&& [key_val_two, inserted_two] = actor_id_map.insert({ actor_two, actor_vertex_counter });
    if (inserted_two) ++actor_vertex_counter;
    auto&& [dummy_two, index_two] = *key_val_two;

    imdb.push_back(index_one, index_two, movie_name);
  }
  imdb.close_for_push_back();

  adjacency<0, std::string> A(imdb);
  return A;
}

int main(int argc, char* argv[]) {
  std::cout << "=== Six Degrees of Kevin Bacon ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 4.1" << std::endl << std::endl;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " scsv_file" << std::endl;
    return -1;
  }

  std::map<std::string, size_t> actor_id_map;
  auto&&                        A = read_imdb(argv[1], actor_id_map);

  size_t              N = A.size();
  std::vector<size_t> bacon_number(N);

  auto it = actor_id_map.find("Kevin Bacon");
  if (it == actor_id_map.end()) {
    std::cerr << "Kevin Bacon not found in the database" << std::endl;
    return -1;
  }
  auto&& [kb, kb_id] = *it;

  for (auto&& [v, u, movie] : bfs_edge_range(A, kb_id)) {
    bacon_number[u] = bacon_number[v] + 1;
  }

  std::cout << "Bacon numbers:" << std::endl;
  for (auto&& [actor, id] : actor_id_map) {
    std::cout << "  " << actor << " has Bacon number of " << bacon_number[id] << std::endl;
  }

  return 0;
}
