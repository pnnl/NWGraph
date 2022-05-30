/**
 * @file bfs_range.hpp
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

#ifndef NW_GRAPH_BFS_RANGE_HPP
#define NW_GRAPH_BFS_RANGE_HPP

#include <cassert>
#include <queue>
#include <vector>

#include "nwgraph/util/util.hpp"
#include "nwgraph/graph_traits.hpp"

namespace nw {
namespace graph {

enum status { ready = 0, waiting, processed };

#if 0
//****************************************************************************
template <typename Graph, typename Queue = std::queue<vertex_id_t<Graph>>>
class topdown_bfs_range {
  using vertex_id_type = vertex_id_t<Graph>;

  using Colors = std::vector<status>;

  Graph& graph_;     //!< underlying graph
  Colors colors_;    //!< auxiliary color state
  Queue  queue_;     //!< worklist

  /// Initialize the range with a vertex id.
  void init(vertex_id_type u) {
    queue_.push(u);
    colors_[u] = waiting;
  }

  /// Visit a vertex by enumerating its ready neighbors.
  void visit(vertex_id_type v) {
    for (auto&& e : graph_[v]) {
      auto u = std::get<0>(e);
      if (colors_[u] == ready) {
        colors_[u] = waiting;
        queue_.push(u);
      }
    }
  }

  /// Process the next vertex by marking the head as processed, popping it, and
  /// visiting the neighbors.
  void process() {
    if (empty()) return;

    auto u     = queue_.front();
    colors_[u] = processed;
    queue_.pop();

    auto v = queue_.front();
    visit(v);
  }

  /// Get the head vertex.
  decltype(auto) head() { return queue_.front(); }

public:
  topdown_bfs_range(Graph& graph, vertex_id_type seed = 0) : graph_(graph), colors_(graph.size()), queue_() {
    init(seed);
    visit(seed);
  }

  topdown_bfs_range(const topdown_bfs_range&) = delete;

  struct end_sentinel_type {};

  class iterator {
  public:
    iterator(topdown_bfs_range& range) : range_(range) {}

    iterator& operator++() {
      range_.process();
      return *this;
    }

    decltype(auto) operator*() { return range_.head(); }

    bool operator==(const end_sentinel_type&) const { return range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !range_.empty(); }

  private:
    topdown_bfs_range& range_;
  };

  iterator          begin() { return {*this}; }
  end_sentinel_type end() { return {}; }
  bool              empty() { return queue_.empty(); }
};    // class topdown_bfs_range

#else

//****************************************************************************
template <typename Graph, typename Queue = std::queue<vertex_id_t<Graph>>>
class topdown_bfs_range {
  using vertex_id_type = vertex_id_t<Graph>;

  Graph&            graph_;     //!< underlying graph
  std::vector<bool> visited_;
  Queue             queue_;     //!< worklist

  /// Initialize the range with a vertex id.
  void init(vertex_id_type u) {
    visited_[u] = true;
    queue_.push(u);
  }

  /// Visit a vertex by enumerating its ready neighbors.
  void visit(vertex_id_type v) {
    //    for (auto&& e : graph_[v]) {
    // auto u = std::get<0>(e);
    std::for_each(graph_[v].begin(), graph_[v].end(), [&] (auto&& e) {

      if (visited_[std::get<0>(e)] == false) {
        visited_[std::get<0>(e)] = true;
        queue_.push(std::get<0>(e));
      }
    });
  }

  /// Process the next vertex by marking the head as processed, popping it, and
  /// visiting the neighbors.
  void process() {
    if (empty()) return;

    auto v = queue_.front();
    queue_.pop();
    visit(v);
  }

  /// Get the head vertex.
  decltype(auto) head() { return queue_.front(); }

public:
  topdown_bfs_range(Graph& graph, vertex_id_type seed = 0) : graph_(graph), visited_(graph.size()), queue_() {
    init(seed);
    visit(seed);
  }

  topdown_bfs_range(const topdown_bfs_range&) = delete;

  struct end_sentinel_type {};

  class iterator {
  public:
    iterator(topdown_bfs_range& range) : range_(range) {}

    iterator& operator++() {
      range_.process();
      return *this;
    }

    decltype(auto) operator*() { return range_.head(); }

    bool operator==(const end_sentinel_type&) const { return range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !range_.empty(); }

  private:
    topdown_bfs_range& range_;
  };

  iterator          begin() { return {*this}; }
  end_sentinel_type end() { return {}; }
  bool              empty() { return queue_.empty(); }
};    // class topdown_bfs_range


#endif


template <typename Graph>
class bottomup_bfs_range {
  using vertex_id_type = vertex_id_t<Graph>;
  using Colors         = std::vector<status>;

  Graph&         graph_;
  Colors         colors_;
  vertex_id_type v_;
  vertex_id_type parent_v_;
  size_t         n_;    // number of "processed" vertices

  void advance() {
    // mark current v processed (visited)
    colors_[v_] = processed;
    ++n_;
    ++v_;
    v_ %= graph_.size();
    colors_[v_] = waiting;
    //test v has neighors or not
    if (0 == graph_[v_].size()) {
      parent_v_ = v_;
    }
    else {
      //if it has neighbor
      for (auto&& [u] : graph_[v_]) {
        if (colors_[u] == processed) {
          parent_v_ = u;    //mark the first non-processed/unvisited neighbor as parent
          break;            //done with v's neighbors
        }
      }    // for
    }
/*
    // if no valid parent_v, meaning we have travese to the leaf, we mark v
    // processed and be done with it
    if (v_ == parent_v_) {
      colors_[v_] = processed;
      ++n_;
    }
    */
  }

  decltype(auto) next() { return std::tuple(v_, parent_v_); }

public:
  //n_ should be 0, assume seed has been processed
  bottomup_bfs_range(Graph& graph, vertex_id_type seed = 0) : graph_(graph), colors_(graph.size()), v_(seed), n_(0) {
    colors_[v_] = waiting;
    //we set the first neighbor as parent
    //but what if v_ has no neighbors?
    if (0 == graph_[v_].size())
      parent_v_ = v_;
    else
      parent_v_ = std::get<0>(*(graph_[seed].begin()));
  }

  bottomup_bfs_range(const bottomup_bfs_range&) = delete;

  struct end_sentinel_type {};

  class iterator {
  public:
    iterator(bottomup_bfs_range& range) : range_(range) {}

    iterator& operator++() {
      range_.advance();
      return *this;
    }

    decltype(auto) operator*() { return range_.next(); }

    bool operator==(const end_sentinel_type&) const { return range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !range_.empty(); }

  private:
    bottomup_bfs_range& range_;
  };

  iterator          begin() { return {*this}; }
  end_sentinel_type end() { return {}; }

  bool empty() { return n_ == graph_.size(); }
};    //class bottomup_bfs_range

}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_BFS_RANGE_HPP
