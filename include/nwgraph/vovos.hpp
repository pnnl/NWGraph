/**
 * @file vovos.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Krzysztof Drewniak
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_VOVOS_HPP
#define NW_GRAPH_VOVOS_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <tuple>
#include <vector>

#include "edge_list.hpp"
#include "nwgraph/graph_base.hpp"

namespace nw {
namespace graph {


template <typename... Attributes>
class vector_of_vector_of_structs : public std::vector<std::vector<std::tuple<Attributes...>>> {

public:
  using base = std::vector<std::vector<std::tuple<Attributes...>>>;

  vector_of_vector_of_structs(size_t N) : base(N) {}

  // using inner_iterator       = typename std::vector<std::tuple<Attributes...>>::iterator;
  // using const_inner_iterator = typename std::vector<std::tuple<Attributes...>>::const_iterator;
  // using inner_container_ref  = typename std::vector<std::tuple<Attributes...>>&;
  // using outer_iterator       = typename std::vector<std::vector<std::tuple<Attributes...>>>::iterator;
  // using const_outer_iterator = typename std::vector<std::vector<std::tuple<Attributes...>>>::const_iterator;

  void open_for_push_back() {}
  void close_for_push_back() {}
  void push_back(size_t i, Attributes... attrs) { 
    if (i >= base::size()) {
      for (size_t j = base::size(); j <= i; ++j) {
        base::emplace_back();
      }
    }
    base::operator[](i).emplace_back(attrs...);
  }

  auto size() const { return base::size(); }
};


template <int idx, std::unsigned_integral vertex_id, typename... Attributes>
class index_vov : public unipartite_graph_base, public vector_of_vector_of_structs<vertex_id, Attributes...> {
  using base = vector_of_vector_of_structs<vertex_id, Attributes...>;

public:
  using vertex_id_type    = vertex_id;
  using graph_base        = unipartite_graph_base;
  using num_vertices_type = std::array<typename base::size_type, 1>;
  using num_edges_type    = typename base::size_type;

  using attributes_t = std::tuple<Attributes...>;

  static constexpr std::size_t getNAttr() { return sizeof...(Attributes); }

  index_vov(size_t N = 0) : base(N) {}

  index_vov(edge_list<directedness::directed, Attributes...>& A) : base(num_vertices(A)) { num_edges_ = fill_adj_list(A, *this); }

  index_vov(edge_list<directedness::undirected, Attributes...>& A) : base(num_vertices(A)) { num_edges_ = fill_adj_list(A, *this); }

private:
  num_edges_type num_edges_;
};

template <int idx, typename... Attributes>
using vov = index_vov<idx, default_vertex_id_type, Attributes...>;

}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_VOVOS_HPP
