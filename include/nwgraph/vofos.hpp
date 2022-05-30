/**
 * @file vofos.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Krzysztof Drewniak
 *
 */

#ifndef NW_GRAPH_VOFOS_HPP
#define NW_GRAPH_VOFOS_HPP

#include <cassert>
#include <forward_list>
#include <tuple>
#include <vector>

#include "nwgraph/build.hpp"
#include "edge_list.hpp"
#include "nwgraph/graph_base.hpp"
#include "nwgraph/graph_traits.hpp"

namespace nw {
namespace graph {

template <typename... Attributes>
class vector_of_flist_of_structs : public std::vector<std::forward_list<std::tuple<Attributes...>>> {

public:
  using base  = std::vector<std::forward_list<std::tuple<Attributes...>>>;
  using inner = std::forward_list<std::tuple<Attributes...>>;

  using inner_container_ref  = inner&;
  using outer_iterator       = typename base::iterator;
  using const_outer_iterator = typename base::const_iterator;
  using inner_iterator       = typename inner::iterator;
  using const_inner_iterator = typename inner::const_iterator;

  vector_of_flist_of_structs(size_t N) : base(N) {}
};

template <int idx, std::unsigned_integral vertex_id, typename... Attributes>
class index_adj_flist : public unipartite_graph_base, public vector_of_flist_of_structs<vertex_id, Attributes...> {
public:
  using vertex_id_type    = vertex_id;
  using base              = vector_of_flist_of_structs<vertex_id_type, Attributes...>;
  using graph_base        = unipartite_graph_base;
  using num_vertices_type = std::array<typename base::size_type, 1>;
  using num_edges_type    = typename base::size_type;

  index_adj_flist(size_t N = 0) : base(N) {}

  index_adj_flist(edge_list<directedness::directed, Attributes...>& A) : base(A.num_vertices()[0]) { num_edges_ = fill_adj_flist(A, *this); }

  index_adj_flist(edge_list<directedness::undirected, Attributes...>& A) : base(A.num_vertices()[0]) { num_edges_ = fill_adj_flist(A, *this); }

  using iterator = typename base::outer_iterator;

  using outer_iterator       = typename base::outer_iterator;
  using inner_iterator       = typename base::inner_iterator;
  using const_outer_iterator = typename base::const_outer_iterator;
  using const_inner_iterator = typename base::const_inner_iterator;

  using attributes_t = std::tuple<Attributes...>;
  static constexpr std::size_t getNAttr() { return sizeof...(Attributes); }

  void open_for_push_back() { graph_base::is_open = true; }
  void close_for_push_back() { graph_base::is_open = false; }
  void push_back(size_t i, size_t j, Attributes... attrs) {
    if (i >= base::size()) {
      for (size_t k = base::size(); k <= i; ++k) {
        base::emplace_back();
      }
    }
    base::operator[](i).emplace_front(j, attrs...);
  }

  num_vertices_type num_vertices() const { return {base::size()}; };
  num_edges_type    num_edges() const { return num_edges_; }

private:
  num_edges_type num_edges_;
};

template <int idx, typename... Attributes>
using adj_flist = index_adj_flist<idx, default_vertex_id_type, Attributes...>;

template <int idx, std::unsigned_integral vertex_id, typename... Attributes>
struct graph_traits<index_adj_flist<idx, vertex_id, Attributes...>> {

  using my_type = index_adj_flist<idx, vertex_id, Attributes...>;

  using tuple_type = std::tuple<Attributes...>;
  using inner_type = std::forward_list<tuple_type>;
  using outer_type = std::vector<inner_type>;

  using outer_iterator = typename outer_type::iterator;
  using inner_iterator = typename inner_type::iterator;

  using const_outer_iterator = typename outer_type::const_iterator;
  using const_inner_iterator = typename inner_type::const_iterator;

  using vertex_id_type    = typename my_type::vertex_id_type;
  using vertex_size_type  = typename outer_type::size_type;
  using num_vertices_type = std::array<vertex_size_type, 1>;
};

template <int idx, std::unsigned_integral vertex_id, typename... Attributes>
auto tag_invoke(const num_vertices_tag, index_adj_flist<idx, vertex_id, Attributes...>& b) {
  return b.num_vertices()[0];
}

#if 0

template <typename... Attributes>
graph_traits<std::vector<std::forward_list<std::tuple<Attributes...>>>>::num_vertices_type
num_vertices(const typename std::vector<std::forward_list<std::tuple<Attributes...>>>& g) {
  return { g.size() };
}
#endif

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_VOFOS_HPP
