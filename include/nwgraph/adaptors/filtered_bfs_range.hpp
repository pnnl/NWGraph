/**
 * @file filtered_bfs_range.hpp
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

#ifndef NW_GRAPH_FILTERED_BFS_RANGE_HPP
#define NW_GRAPH_FILTERED_BFS_RANGE_HPP

#include "nwgraph/graph_traits.hpp"
#include "nwgraph/util/util.hpp"
#include <cassert>
#include <functional>
#include <queue>
#include <tuple>
#include <vector>

namespace nw {
namespace graph {
namespace filtered_bfs {

enum three_colors { black, white, grey };

template <typename Graph, typename Queue = std::queue<vertex_id_t<Graph>>, typename Filter = std::function<bool()>>
class filtered_bfs_edge_range {

public:
  using vertex_id_type = vertex_id_t<Graph>;

  filtered_bfs_edge_range(
      Graph& graph, vertex_id_type source, vertex_id_type target,
      Filter filter = [](vertex_id_type v, typename Graph::inner_iterator iter) { return false; })
      : the_graph_(graph), colors_(graph.end() - graph.begin(), white), target_(target), filter_(filter) {
    Q_.push(source);
    colors_[source] = grey;
  }

  filtered_bfs_edge_range(const filtered_bfs_edge_range&)  = delete;
  filtered_bfs_edge_range(const filtered_bfs_edge_range&&) = delete;

  bool empty() { return Q_.empty(); }
  bool found() { return colors_[target_] == grey; }
  bool done() { return (found() || unreachable); }
  bool unreachable = false;

  class filtered_bfs_edge_range_iterator {

  public:
    filtered_bfs_edge_range_iterator(filtered_bfs_edge_range<Graph, Queue, Filter>& range)
        : the_range_(range), G(the_range_.the_graph_.begin()), v_(the_range_.Q_.front()), u_begin(G[v_].begin()),
          iterator_target_(the_range_.target_), filter_(the_range_.filter_) {

      while (filter_(v_, u_begin)) {
        ++u_begin;
        if (u_begin == G[v_].end()) {
          the_range_.unreachable = true;
          break;
        }
      }
    }

    filtered_bfs_edge_range_iterator(const filtered_bfs_edge_range_iterator& ite)
        : the_range_(ite.the_range_), G(ite.G), v_(ite.v_), u_begin(ite.u_begin), filter_(ite.filter_) {}

    filtered_bfs_edge_range_iterator& operator++() {
      auto& Q      = the_range_.Q_;
      auto& colors = the_range_.colors_;

      Q.push(std::get<0>(*u_begin));
      colors[std::get<0>(*u_begin)] = grey;

      if (iterator_target_ == std::get<0>(*u_begin)) {
        colors[std::get<0>(*u_begin)] = grey;
        return *this;
      }
      ++u_begin;

      while (u_begin != G[v_].end()) {
        if (colors[std::get<0>(*u_begin)] == white && !filter_(v_, u_begin)) break;

        ++u_begin;
      }

      while (u_begin == G[v_].end()) {
        colors[v_] = black;
        Q.pop();
        if (Q.empty()) {
          the_range_.unreachable = true;
          break;
        }

        v_ = Q.front();
        assert(colors[v_] == grey);

        u_begin = G[v_].begin();

        while (u_begin != G[v_].end()) {
          if (colors[std::get<0>(*u_begin)] == white && !filter_(v_, u_begin)) break;

          ++u_begin;
        }
      }
      return *this;
    }

    auto operator*() { return *u_begin; }
    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto get_index() { return v_; };

    auto operator==(const end_sentinel_type&) const { return the_range_.done(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.done(); }

    bool found() { return the_range_.found(); }

  private:
    filtered_bfs_edge_range<Graph, Queue, Filter>& the_range_;
    typename Graph::outer_iterator                 G;
    vertex_id_type                                 v_;
    typename Graph::inner_iterator                 u_begin;
    vertex_id_type                                 iterator_target_;
    Filter                                         filter_;
  };

  typedef filtered_bfs_edge_range_iterator iterator;

  auto begin() { return filtered_bfs_edge_range_iterator(*this); }
  auto end() { return typename filtered_bfs_edge_range_iterator::end_sentinel_type(); }

private:
  Graph&                    the_graph_;
  Queue                     Q_;
  std::vector<three_colors> colors_;
  vertex_id_type            target_;
  Filter                    filter_;
};
}    // namespace filtered_bfs
}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_FILTERED_BFS_RANGE_HPP
