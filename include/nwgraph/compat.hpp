/**
 * @file compat.hpp
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

#ifndef NWGRAPH_COMPAT_HPP
#define NWGRAPH_COMPAT_HPP

#warning "compat.hpp is deprecated"

#if 0

#include <forward_list>
#include <list>
#include <tuple>
#include <vector>

#include "nwgraph/graph_traits.hpp"



namespace nw {
  namespace graph {


template <typename... Attributes>
struct graph_traits<std::forward_list<std::tuple<Attributes...>>> {
  using tuple_type = std::tuple<Attributes...>;

  using vertex_id_type = typename std::tuple_element<0, tuple_type>::type;
};


template <typename... Attributes>
struct graph_traits<std::list<std::tuple<Attributes...>>> {
  using tuple_type = std::tuple<Attributes...>;
  using outer_type = std::list<tuple_type>;

  using vertex_id_type = typename std::tuple_element<0, tuple_type>::type;
  using vertex_size_type  = typename outer_type::size_type;
  using num_vertices_type = std::array<vertex_size_type, 1>;
};



template <typename... Attributes>
auto tag_invoke(const num_edges_tag, const std::list<std::tuple<Attributes...>>& b) {
  return b.size();
}


template <typename... Attributes>
struct graph_traits<std::vector<std::tuple<Attributes...>>> {
  using tuple_type = std::tuple<Attributes...>;
  using outer_type = std::vector<tuple_type>;

  using vertex_id_type = typename std::tuple_element<0, tuple_type>::type;
  using vertex_size_type  = typename outer_type::size_type;
  using num_vertices_type = std::array<vertex_size_type, 1>;
};


template <typename... Attributes>
auto tag_invoke(const num_edges_tag, const std::vector<std::tuple<Attributes...>>& b) {
  return b.size();
}



// std::vector<forward_list>
template <typename... Attributes>
struct graph_traits<std::vector<std::forward_list<std::tuple<Attributes...>>>> {
  using tuple_type = std::tuple<Attributes...>;
  using inner_type = std::forward_list<tuple_type>;
  using outer_type = std::vector<inner_type>;

  using outer_iterator = typename outer_type::iterator;
  using inner_iterator = typename inner_type::iterator;

  using const_outer_iterator = typename outer_type::const_iterator;
  using const_inner_iterator = typename inner_type::const_iterator;

  using vertex_id_type    = typename std::tuple_element<0, tuple_type>::type;
  using vertex_size_type  = typename outer_type::size_type;
  using num_vertices_type = std::array<vertex_size_type, 1>;
};


// std::vector<list>
template <typename... Attributes>
struct graph_traits<std::vector<std::list<std::tuple<Attributes...>>>> {
  using tuple_type = std::tuple<Attributes...>;
  using inner_type = std::list<tuple_type>;
  using outer_type = std::vector<inner_type>;

  using outer_iterator = typename outer_type::iterator;
  using inner_iterator = typename inner_type::iterator;

  using const_outer_iterator = typename outer_type::const_iterator;
  using const_inner_iterator = typename inner_type::const_iterator;

  using vertex_id_type    = typename std::tuple_element<0, tuple_type>::type;
  using vertex_size_type  = typename outer_type::size_type;
  using num_vertices_type = std::array<vertex_size_type, 1>;
};

// std::vector<vector>
template <typename... Attributes>
struct graph_traits<std::vector<std::vector<std::tuple<Attributes...>>>> {
  using tuple_type = std::tuple<Attributes...>;
  using inner_type = std::vector<tuple_type>;
  using outer_type = std::vector<inner_type>;

  using outer_iterator = typename outer_type::iterator;
  using inner_iterator = typename inner_type::iterator;

  using const_outer_iterator = typename outer_type::const_iterator;
  using const_inner_iterator = typename inner_type::const_iterator;

  using vertex_id_type    = typename std::tuple_element<0, tuple_type>::type;
  using vertex_size_type  = typename outer_type::size_type;
  using num_vertices_type = std::array<vertex_size_type, 1>;
};




  }
}
#endif
#endif // NWGRAPH_COMPAT_HPP
