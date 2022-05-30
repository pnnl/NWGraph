/**
 * @file random_range.hpp
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

#ifndef NW_GRAPH_RANDOM_RANGE_HPP
#define NW_GRAPH_RANDOM_RANGE_HPP

#include "nwgraph/graph_traits.hpp"
#include "nwgraph/util/util.hpp"
#include <cmath>
#include <random>
#include <vector>

namespace nw {
namespace graph {

template <typename Graph>
class random_range {
public:
  random_range(Graph& g, size_t length = size_t(0xffffffffffffffffULL), vertex_id_t<Graph> first = 0, unsigned seed = 2049)
      : the_graph_(g), starting_vertex(first), length_(length), distribution(0, 1.0), generator(seed),
        dice(std::bind(distribution, generator)) {}

  class edge_range_iterator {
  private:
    using vertex_id_type = vertex_id_t<Graph>;

    random_range<Graph>&           the_range_;
    typename Graph::outer_iterator G;
    vertex_id_type                 current_vertex;
    size_t                         length_;

  public:
    edge_range_iterator(random_range<Graph>& range)
        : the_range_(range), G(the_range_.the_graph_.begin()), current_vertex(the_range_.starting_vertex), length_(the_range_.length_) {}

    edge_range_iterator& operator++() {
      --length_;
      typename Graph::inner_iterator u_begin = G[current_vertex].begin();
      typename Graph::inner_iterator u_end   = G[current_vertex].end();
      size_t                         N       = u_end - u_begin;
      vertex_id_type                 pick    = std::floor(N * the_range_.dice());

      current_vertex = std::get<0>(u_begin[pick]);

      return *this;
    }

    auto operator*() const { return current_vertex; }

    struct end_sentinel_type {};

    auto operator==(const end_sentinel_type&) const { return length_ == 0; }
    bool operator!=(const end_sentinel_type&) const { return length_ != 0; }
  };

  typedef edge_range_iterator iterator;

  auto begin() { return edge_range_iterator(*this); }
  auto end() { return typename edge_range_iterator::end_sentinel_type(); }

private:
  Graph& the_graph_;

  vertex_id_t<Graph> starting_vertex;
  size_t             length_;

  std::uniform_real_distribution<double> distribution;
  std::default_random_engine             generator;

  decltype(std::bind(distribution, generator)) dice;
};

}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_RANDOM_RANGE_HPP
