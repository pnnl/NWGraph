/**
 * @file bfs_edge_range.hpp
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

#ifndef BFS_EDGE_RANGE_HPP
#define BFS_EDGE_RANGE_HPP



#include "nwgraph/graph_traits.hpp"

#include <cassert>
#include <queue>
#include <tuple>
#include <vector>

namespace nw {
namespace graph {


enum three_colors { black, white, grey };

#if 0
template <typename Graph, typename Queue = std::queue<vertex_id_t<Graph>>>
class bfs_edge_range {
private:
  using vertex_id_type = vertex_id_t<Graph>;

public:
  bfs_edge_range(Graph& graph, vertex_id_type seed = 0) : the_graph_(graph), colors_(graph.size(), white) {
    Q_.push(seed);
    colors_[seed] = grey;
  }

  bfs_edge_range(const bfs_edge_range&)  = delete;
  bfs_edge_range(const bfs_edge_range&&) = delete;

  bool empty() {
    bool b = Q_.empty();
    return b;
  }

  class bfs_edge_range_iterator {
  private:
    bfs_edge_range<Graph, Queue>&  the_range_;
    typename Graph::outer_iterator G;
    vertex_id_type                 v_;
    typename Graph::inner_iterator u_begin, u_end;

  public:
    bfs_edge_range_iterator(bfs_edge_range<Graph, Queue>& range)
        : the_range_(range), G(the_range_.the_graph_.begin()), v_(the_range_.Q_.front()), u_begin(G[v_].begin()), u_end(G[v_].end()) {}

    bfs_edge_range_iterator(const bfs_edge_range_iterator& ite)
        : the_range_(ite.the_range_), G(ite.G), v_(ite.v_), u_begin(u_begin), u_end(u_end) {}

    bfs_edge_range_iterator& operator++() {
      auto& Q      = the_range_.Q_;
      auto& colors = the_range_.colors_;

      Q.push(std::get<0>(*u_begin));
      colors[std::get<0>(*u_begin)] = grey;

      ++u_begin;
      while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
        ++u_begin;
      }

      while (u_begin == u_end) {
        colors[v_] = black;
        Q.pop();
        if (Q.empty()) break;

        v_ = Q.front();
        assert(colors[v_] == grey);
        u_begin = G[v_].begin();
        u_end   = G[v_].end();

        while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
          ++u_begin;
        }
      }

      return *this;
    }

    auto operator*() { return std::tuple_cat(std::make_tuple(v_), *u_begin); }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }
  };

  typedef bfs_edge_range_iterator iterator;

  auto begin() { return bfs_edge_range_iterator(*this); }
  auto end() { return typename bfs_edge_range_iterator::end_sentinel_type(); }

private:
  Graph&                    the_graph_;
  Queue                     Q_;
  std::vector<three_colors> colors_;
};

#else

template <typename Graph, typename Queue = std::queue<vertex_id_t<Graph>>>
class bfs_edge_range {
private:
  using vertex_id_type = vertex_id_t<Graph>;

public:
  bfs_edge_range(Graph& graph, vertex_id_type seed = 0) : the_graph_(graph), visited_(graph.size(), false) {
    visited_[seed] = true;
    Q_.push(seed);
  }

  bfs_edge_range(const bfs_edge_range&)  = delete;
  bfs_edge_range(const bfs_edge_range&&) = delete;

  bool empty() {
    bool b = Q_.empty();
    return b;
  }

  class bfs_edge_range_iterator {
  private:
    bfs_edge_range<Graph, Queue>&  the_range_;
    typename Graph::outer_iterator G;
    vertex_id_type                 v_;
    typename Graph::inner_iterator u_begin, u_end;

  public:
    bfs_edge_range_iterator(bfs_edge_range<Graph, Queue>& range)
        : the_range_(range), G(the_range_.the_graph_.begin()), v_(the_range_.Q_.front()), u_begin(G[v_].begin()), u_end(G[v_].end()) {}

    bfs_edge_range_iterator(const bfs_edge_range_iterator& ite)
        : the_range_(ite.the_range_), G(ite.G), v_(ite.v_), u_begin(ite.u_begin), u_end(ite.u_end) {}

    bfs_edge_range_iterator& operator++() {
      auto& Q      = the_range_.Q_;
      auto& visited = the_range_.visited_;


      visited[std::get<0>(*u_begin)] = true;
      Q.push(std::get<0>(*u_begin));

      ++u_begin;
      while (u_begin != u_end && visited[std::get<0>(*u_begin)] == true) {
        ++u_begin;
      }

      while (u_begin == u_end) {
        Q.pop();
        if (Q.empty()) break;

        v_ = Q.front();
        assert(visited[v_] == true);
        u_begin = G[v_].begin();
        u_end   = G[v_].end();

        while (u_begin != u_end && visited[std::get<0>(*u_begin)] == true) {
          ++u_begin;
        }
      }

      return *this;
    }

    // Better to store v_ as a tuple?
    auto operator*() { return std::tuple_cat(std::make_tuple(v_), *u_begin); }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }
  };

  typedef bfs_edge_range_iterator iterator;

  auto begin() { return bfs_edge_range_iterator(*this); }
  auto end() { return typename bfs_edge_range_iterator::end_sentinel_type(); }

private:
  Graph&                    the_graph_;
  Queue                     Q_;
  std::vector<bool> visited_;
};

  #endif
//****************************************************************************
// This range used by dijkstra
template <typename Graph, typename PriorityQueue>
class bfs_edge_range2 {
private:
  using vertex_id_type = typename Graph::vertex_id_type;

public:
  bfs_edge_range2(Graph& graph, PriorityQueue& Q, std::tuple<size_t, size_t> seed = {0, 0})
      : the_graph_(graph), Q_(Q), colors_(graph.end() - graph.begin(), white) {
    Q_.push(seed);
    colors_[std::get<0>(seed)] = grey;
  }

  bfs_edge_range2(const bfs_edge_range2&)  = delete;
  bfs_edge_range2(const bfs_edge_range2&&) = delete;

  bool empty() { return Q_.empty(); }

  class bfs_edge_range2_iterator {
  private:
    bfs_edge_range2<Graph, PriorityQueue>& the_range_;
    typename Graph::outer_iterator         G;
    vertex_id_type                         v_;
    typename Graph::inner_iterator         u_begin, u_end;

    // Graph -> v, u, w
    // Q -> v, d

  public:
    bfs_edge_range2_iterator(bfs_edge_range2<Graph, PriorityQueue>& range)
        : the_range_(range), G(the_range_.the_graph_.begin()), v_(std::get<0>(the_range_.Q_.top())), u_begin(G[v_].begin()),
          u_end(G[v_].end()) {}

    bfs_edge_range2_iterator& operator++() {
      auto& Q      = the_range_.Q_;
      auto& colors = the_range_.colors_;

      Q.push({std::get<0>(*u_begin), size_t(0xffffffffffffffffULL)});
      colors[std::get<0>(*u_begin)] = grey;

      ++u_begin;
      while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
        ++u_begin;
      }

      while (u_begin == u_end) {
        colors[v_] = black;

        while (colors[std::get<0>(Q.top())] == black && !Q.empty())
          Q.pop();

        if (Q.empty()) break;

        v_      = std::get<0>(Q.top());
        u_begin = G[v_].begin();
        u_end   = G[v_].end();

        while (u_begin != u_end && colors[std::get<0>(*u_begin)] != white) {
          ++u_begin;
        }
      }

      return *this;
    }

    auto operator*() { return std::tuple<vertex_id_type, vertex_id_type, size_t>(v_, std::get<0>(*u_begin), std::get<1>(*u_begin)); }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }
  };

  typedef bfs_edge_range2_iterator iterator;

  auto begin() { return bfs_edge_range2_iterator(*this); }
  auto end() { return typename bfs_edge_range2_iterator::end_sentinel_type(); }

private:
  Graph&                    the_graph_;
  PriorityQueue&            Q_;
  std::vector<three_colors> colors_;
};

template <typename Graph, typename Queue = std::queue<typename Graph::vertex_id_type>>
class bfs_edge_range3 {
private:
  using vertex_id_type = typename Graph::vertex_id_type;

public:
  bfs_edge_range3(Graph& graph, vertex_id_type seed = 0) : the_graph_(graph), colors_(graph.end() - graph.begin(), white) {
    Q_[0].push(seed);
    colors_[seed] = grey;
    //After adding seed, add the neighbors of seed to next queue
    for (auto ite = the_graph_[seed].begin(); ite != the_graph_[seed].end(); ++ite) {    // Explore neighbors
      auto u = std::get<0>(*ite);
      if (colors_[u] == white) {
        colors_[u] = grey;
        Q_[1].push(u);
      }
    }
  }

  bfs_edge_range3(const bfs_edge_range3&)  = delete;
  bfs_edge_range3(const bfs_edge_range3&&) = delete;

  bool empty() { return Q_[0].empty(); }

  class bfs_edge_range3_iterator {
  public:
    bfs_edge_range3_iterator(bfs_edge_range3<Graph>& range) : the_range_(range) {}

    bfs_edge_range3_iterator& operator++() {
      auto G       = the_range_.the_graph_.begin();
      Queue(&Q)[2] = the_range_.Q_;
      auto& colors = the_range_.colors_;

      if (!Q[1].empty()) {    // if the neighbor queue is not empty
                              // we proceed all the neighbors
        auto u = Q[1].front();
        Q[1].pop();                   // pop the child which has been visited
        colors[u] = black;            // mark the child as black (visited)
        Q[0].push(u);                 // put the visited child to current frontier (it becomes parent)
        while (Q[1].empty()) {        // keep filling neighbors until the
                                      // neighboring queue is not empty
          if (Q[0].empty()) break;    // break if both queue are empty

          auto v = Q[0].front();    // get parent vertex v

          for (auto ite = G[v].begin(); ite != G[v].end(); ++ite) {    // Explore neighbors
            u = std::get<0>(*ite);
            if (colors[u] == white) {
              colors[u] = grey;
              Q[1].push(u);    //add all the white neighbors of v
            }
          }                       //for
          if (Q[1].empty()) {     //if parent vertex v has no white neighbors, then v has done all traverse
            Q[0].pop();           //pop it out
            colors[v] = black;    //be done with parent vertex v
          }
          // but we still need to populate the child queue, so we continue populate with the next parent vertex
        }    // while
      } else {
        auto v = Q[0].front();
        Q[0].pop();    //if v has no neighbor, pop it
        colors[v] = black;
      }

      return *this;
    }

    auto operator*() {
      Queue(&Q)[2] = the_range_.Q_;
      //if v has no neighbor, return a self loop
      if (!Q[1].empty())
        return std::make_tuple(Q[0].front(), Q[1].front());
      else
        return std::make_tuple(Q[0].front(), Q[0].front());
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }

  private:
    bfs_edge_range3<Graph>& the_range_;
  };

  typedef bfs_edge_range3_iterator iterator;

  auto begin() { return bfs_edge_range3_iterator(*this); }
  auto end() { return typename bfs_edge_range3_iterator::end_sentinel_type(); }

private:
  Graph&                    the_graph_;
  std::vector<three_colors> colors_;    // Every node in Q_[0] and those who has been processed is black.
                                        // Every nobe in Q_[1] is grey.
                                        // Every unprocessed node is white.
  Queue Q_[2];                          //Q_[0] is processing queue, Q_[1] is neighbor queue
  //the reason to use two queues instead of one: we need to return <parent,child> tuple at the front of each queue respectively.
};
}    // namespace graph
}    // namespace nw

#endif    // BFS_EDGE_RANGE_HPP
