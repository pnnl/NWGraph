/**
 * @file graph_traits.hpp
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

#ifndef NW_GRAPH_GRAPH_TRAITS_HPP
#define NW_GRAPH_GRAPH_TRAITS_HPP

#include <type_traits>

namespace nw {
namespace graph {

namespace detail {

// Exposes vertex_id_type only when G provides it, so that instantiating the
// primary graph_traits for a non-graph type leaves an empty struct instead of
// a hard error.  This lets `typename vertex_id_t<G>` inside a concept fail
// softly (the concept evaluates to false) rather than aborting compilation.
template <class G, class = void>
struct default_graph_traits {};

template <class G>
struct default_graph_traits<G, std::void_t<typename G::vertex_id_type>> {
  using vertex_id_type = typename G::vertex_id_type;
};

}    // namespace detail

template <typename G>
struct graph_traits : detail::default_graph_traits<G> {

  // using vertex_size_type  = typename G::vertex_id_type;
  // using num_vertices_type = typename G::num_vertices_type;
  // using num_edges_type    = typename G::num_edges_type;

  //  using outer_iterator = typename G::outer_iterator;
  //  using inner_iterator = typename G::inner_iterator;

  // using const_outer_iterator = typename G::const_outer_iterator;
  // using const_inner_iterator = typename G::const_inner_iterator;
};

template <typename G>
using vertex_id_t = typename graph_traits<G>::vertex_id_type;

// template <typename G>
// using vertex_size_t = typename graph_traits<G>::vertex_size_type;

// template <typename G>
// using num_vertices_t = typename graph_traits<G>::num_vertices_type;

// template <typename G>
// using num_edges_t = typename graph_traits<G>::num_edges_type;

// template <typename G>
// using outer_iterator_t = typename graph_traits<G>::outer_iterator;

// template <typename G>
// using inner_iterator_t = typename graph_traits<G>::inner_iterator;

// template <typename G>
// using const_outer_iterator_t = typename graph_traits<G>::const_outer_iterator;

// template <typename G>
// using const_inner_iterator_t = typename graph_traits<G>::const_inner_iterator;

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_GRAPH_TRAITS_HPP
