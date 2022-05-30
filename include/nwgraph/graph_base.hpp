/**
 * @file graph_base.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_GRAPH_BASE_HPP
#define NW_GRAPH_GRAPH_BASE_HPP

#include <array>
#include <cstddef>
#include <istream>

namespace nw {
namespace graph {

enum class succession { successor, predecessor };
enum class directedness { undirected, directed };

template <succession success>
struct other_succession {
  const succession cessor;
};

template <>
struct other_succession<succession::successor> {
  const succession cessor = succession::predecessor;
};

template <>
struct other_succession<succession::predecessor> {
  const succession cessor = succession::successor;
};

template <directedness dir>
struct other_direction {
  const directedness direction;
};

template <>
struct other_direction<directedness::undirected> {
  const directedness direction = directedness::directed;
};

template <>
class other_direction<directedness::directed> {
public:
  const directedness direction = directedness::undirected;
};

class unipartite_graph_base {
public:
  using vertex_cardinality_t = std::array<size_t, 1>;

  unipartite_graph_base(size_t d0 = 0) : vertex_cardinality{d0}, is_open(false) {}

  // auto num_edges() { return num_edges_; }
  /**
   * @brief Serialize the unipartite_graph_base into binary.
   * 
   * @param outfile The output ostream object.
   */
  void serialize(std::ostream& outfile) const {
    outfile.write(reinterpret_cast<const char*>(&vertex_cardinality[0]), sizeof(size_t));
    outfile.write(reinterpret_cast<const char*>(&is_open), sizeof(bool));
  }

  /**
   * @brief Deserialize the binary back to unipartite_graph_base.
   * 
   * @param infile The input istream object.
   */
  void deserialize(std::istream& infile) {
    size_t N;
    infile.read(reinterpret_cast<char*>(&N), sizeof(size_t));
    vertex_cardinality[0] = N;
    infile.read(reinterpret_cast<char*>(&is_open), sizeof(bool));
  }

protected:
  vertex_cardinality_t vertex_cardinality;
  bool                 is_open;    // can we mutate graph
};

class bipartite_graph_base {
public:
  using vertex_cardinality_t = std::array<size_t, 2>;

  bipartite_graph_base(size_t d0 = 0, size_t d1 = 0) : vertex_cardinality{d0, d1}, is_open(false) {}
  bipartite_graph_base(vertex_cardinality_t cardinality) : vertex_cardinality(cardinality), is_open(false) {}

  // auto num_edges() { return num_edges_; }
  /**
   * @brief Serialize the bipartite_graph_base into binary.
   * 
   * @param outfile The output ostream object.
   */
  void serialize(std::ostream& outfile) const {
    outfile.write(reinterpret_cast<const char*>(&vertex_cardinality[0]), sizeof(size_t));
    outfile.write(reinterpret_cast<const char*>(&vertex_cardinality[1]), sizeof(size_t));
    outfile.write(reinterpret_cast<const char*>(&is_open), sizeof(bool));
  }

  /**
   * @brief Deserialize the binary back to bipartite_graph_base.
   * 
   * @param infile The input istream object.
   */
  void deserialize(std::istream& infile) {
    size_t N0, N1;
    infile.read(reinterpret_cast<char*>(&N0), sizeof(size_t));
    infile.read(reinterpret_cast<char*>(&N1), sizeof(size_t));
    vertex_cardinality[0] = N0;
    vertex_cardinality[1] = N1;
    infile.read(reinterpret_cast<char*>(&is_open), sizeof(bool));
  }

protected:
  vertex_cardinality_t vertex_cardinality;    // ordinal limits
  bool                 is_open;               // can we mutate graph
};

// In NON-unipartite_graph_base, is_unipartite<unipartite_graph_base> == false
template <typename Graph_base>
struct is_unipartite {
  constexpr static const bool value = false;
};

// for unipartite_graph_base, is_unipartite<unipartite_graph_base> == true
template <>
struct is_unipartite<unipartite_graph_base> {
  constexpr static const bool value = true;
};

}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_GRAPH_BASE_HPP
