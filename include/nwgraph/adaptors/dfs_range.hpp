/**
 * @file dfs_range.hpp
 * @brief Range adaptors for depth-first search traversal.
 *
 * This file provides range-based interfaces for DFS traversal over graphs:
 * - dfs_range: Iterates over vertices in DFS order
 * - dfs_edge_range: Iterates over edges (as (source, target) pairs) in DFS order
 *
 * These adaptors allow DFS traversal using C++ range-based for loops:
 * @code
 * for (auto v : dfs_range(graph, start_vertex)) {
 *     // Process vertex v in DFS order
 * }
 *
 * for (auto [u, v] : dfs_edge_range(graph, start_vertex)) {
 *     // Process tree edge (u, v) discovered during DFS
 * }
 * @endcode
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

#ifndef NW_GRAPH_DFS_RANGE_HPP
#define NW_GRAPH_DFS_RANGE_HPP

#include <cassert>
#include <stack>
#include <vector>

#include "nwgraph/graph_traits.hpp"
#include "nwgraph/util/util.hpp"

namespace nw {
namespace graph {

/// @brief Vertex coloring for DFS traversal (white=unvisited, grey=visiting, black=finished).
enum three_colors { black, white, grey };

/**
 * @brief Range adaptor for depth-first search vertex traversal.
 * @tparam Graph The graph type to traverse.
 * @tparam Stack The stack type for DFS (default: std::stack).
 *
 * Provides a range-based interface for visiting vertices in DFS order.
 * The traversal continues to unvisited components if the graph is disconnected.
 *
 * @note The range can only be traversed once (single-pass).
 */
template <typename Graph, typename Stack = std::stack<vertex_id_t<Graph>>>
class dfs_range {
  using vertex_id_type = vertex_id_t<Graph>;

public:
  /**
   * @brief Construct a DFS range starting from a seed vertex.
   * @param graph The graph to traverse.
   * @param seed Starting vertex for DFS (default: 0).
   */
  dfs_range(Graph& graph, vertex_id_type seed = 0) : the_graph_(graph), colors_(graph.end() - graph.begin(), white) {
    Q_.push(seed);
    colors_[seed] = grey;

    dfs_visit(the_graph_.begin(), Q_, colors_, seed);
  }

  dfs_range(const dfs_range&)  = delete;
  dfs_range(const dfs_range&&) = delete;

  bool empty() {
    return Q_.empty();
  }

private:
  template <typename GraphRange>
  static void dfs_visit(const GraphRange& G, Stack& Q, std::vector<three_colors>& colors, vertex_id_type v) {
    auto u     = G[v].begin();
    auto u_end = G[v].end();

    while (u != u_end) {
      if (colors[std::get<0>(*u)] == white) {
        Q.push(v);
        colors[v] = grey;

        v     = std::get<0>(*u);
        u     = G[v].begin();
        u_end = G[v].end();
      } else {
        ++u;
      }
      Q.push(v);
      colors[v] = black;
    }
  }

public:
  class dfs_range_iterator {
  public:
    dfs_range_iterator(dfs_range<Graph>& range) : the_range_(range), cursor_(0) {
    }

    dfs_range_iterator& operator++() {
      auto  G      = the_range_.the_graph_.begin();
      auto& Q      = the_range_.Q_;
      auto& colors = the_range_.colors_;

      while (!Q.empty() && colors[Q.top()] == black)
        Q.pop();

      if (!Q.empty()) {
        dfs_visit(G, Q, colors, Q.top());
      } else {
        while (colors[++cursor_] != white && cursor_ != the_range_.the_graph_.size())
          ;
        if (cursor_ != the_range_.the_graph_.size()) {
          Q.push(cursor_);
          colors[cursor_] = grey;
          dfs_visit(G, Q, colors, cursor_);
        }
      }

      return *this;
    }

    auto operator*() {
      return the_range_.Q_.top();
    }
    // auto operator*() { return active_; }

    class end_sentinel_type {
    public:
      end_sentinel_type() {
      }
    };

    auto operator==(const end_sentinel_type&) const {
      return the_range_.empty();
    }
    bool operator!=(const end_sentinel_type&) const {
      return !the_range_.empty();
    }

  private:
    dfs_range<Graph>& the_range_;
    vertex_id_type    cursor_;
  };

  typedef dfs_range_iterator iterator;

  auto begin() {
    return dfs_range_iterator(*this);
  }
  auto end() {
    return typename dfs_range_iterator::end_sentinel_type();
  }

private:
  Graph&                    the_graph_;
  Stack                     Q_;
  std::vector<three_colors> colors_;
};

/**
 * @brief Range adaptor for depth-first search edge traversal.
 * @tparam Graph The graph type to traverse.
 * @tparam Stack The stack type for DFS (default: std::stack).
 *
 * Provides a range-based interface for visiting tree edges in DFS order.
 * Each iteration yields a tuple representing the edge (source, target, ...).
 *
 * @note The range can only be traversed once (single-pass).
 */
template <typename Graph, typename Stack = std::stack<vertex_id_t<Graph>>>
class dfs_edge_range {
  using vertex_id_type = vertex_id_t<Graph>;

public:
  /**
   * @brief Construct a DFS edge range starting from a seed vertex.
   * @param graph The graph to traverse.
   * @param seed Starting vertex for DFS (default: 0).
   */
  dfs_edge_range(Graph& graph, vertex_id_type seed = 0) : the_graph_(graph), colors_(graph.end() - graph.begin(), white) {
    Q_.push(seed);
    colors_[seed] = grey;
  }

  dfs_edge_range(const dfs_edge_range&)  = delete;
  dfs_edge_range(const dfs_edge_range&&) = delete;

  bool empty() {
    return Q_.empty();
  }

  class dfs_edge_range_iterator {
  private:
    dfs_edge_range<Graph, Stack>&  the_range_;
    typename Graph::outer_iterator G;
    vertex_id_type                 v_;
    typename Graph::inner_iterator u_begin, u_end;

  public:
    dfs_edge_range_iterator(dfs_edge_range<Graph, Stack>& range)
        : the_range_(range), G(the_range_.the_graph_.begin()), v_(the_range_.Q_.top()), u_begin(G[v_].begin()), u_end(G[v_].end()) {
    }

    dfs_edge_range_iterator& operator++() {
      auto& Q      = the_range_.Q_;
      auto& colors = the_range_.colors_;

      Q.push(v_);
      colors[v_] = grey;

      v_      = std::get<0>(*u_begin);
      u_begin = G[v_].begin();
      u_end   = G[v_].end();

      // ++u_begin;
      while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
        ++u_begin;
      }

      while (u_begin == u_end) {
        colors[v_] = black;
        v_         = Q.top();
        Q.pop();
        if (Q.empty()) break;

        assert(colors[v_] == grey);
        u_begin = G[v_].begin();
        u_end   = G[v_].end();

        while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
          ++u_begin;
        }
      }

      return *this;
    }

    auto operator*() {
      return std::tuple_cat(std::make_tuple(v_), *u_begin);
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {
      }
    };

    auto operator==(const end_sentinel_type&) const {
      return the_range_.empty();
    }
    bool operator!=(const end_sentinel_type&) const {
      return !the_range_.empty();
    }
  };

  typedef dfs_edge_range_iterator iterator;

  auto begin() {
    return dfs_edge_range_iterator(*this);
  }
  auto end() {
    return typename dfs_edge_range_iterator::end_sentinel_type();
  }

private:
  Graph&                    the_graph_;
  Stack                     Q_;
  std::vector<three_colors> colors_;
};

}    // namespace graph
}    // namespace nw
#endif    //  NW_GRAPH_DFS_RANGE_HPP
