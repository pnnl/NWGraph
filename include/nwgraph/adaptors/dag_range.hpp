/**
 * @file dag_range.hpp
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

#ifndef NW_GRAPH_DAG_RANGE_HPP
#define NW_GRAPH_DAG_RANGE_HPP

#include <queue>
#include <vector>

#include "nwgraph/graph_traits.hpp"

namespace nw {
namespace graph {

enum ready_to_process { yes, no };

template <typename Graph, typename Queue = std::queue<vertex_id_t<Graph>>>
class dag_range {

public:
  using vertex_id_type = vertex_id_t<Graph>;

  typedef std::vector<std::vector<size_t>> VOV;

  dag_range(Graph& graph, VOV& pred_list, VOV& successor_list)
      : the_graph_(graph), _predecessor_list(pred_list), _successor_list(successor_list) {
    /*Set all pred done counter to zero*/
    _pred_done_counter.resize(the_graph_.size(), 0);
    /*Insert all the roots in the queue*/
    for (size_t i = 0; i < the_graph_.size(); i++) {
      if (pred_list[i].size() == 0) {
        Q_.push(i);
      }
    }
  }
  dag_range(const dag_range&)  = delete;
  dag_range(const dag_range&&) = delete;

  bool empty() { return Q_.empty(); }

  class dag_range_iterator {
  private:
    dag_range<Graph, Queue>&      the_range_;
    vertex_id_type                v_;
    std::vector<size_t>::iterator successor_iterator;

  public:
    dag_range_iterator(dag_range<Graph>& range)
        : the_range_(range), v_(the_range_.Q_.front()), successor_iterator(the_range_._successor_list[v_].begin()) {
      advance_next();
    }

    void advance_next() {
      auto& Q = the_range_.Q_;
      /*Check whether we have processed all the successors of the current vertex*/
      if (successor_iterator == the_range_._successor_list[v_].end()) {
        Q.pop(); /*remove current vertex*/
        while (!Q.empty()) {
          v_ = Q.front(); /*If not empty, get the next vertex*/
          /*If no successor, continue for the next*/
          if (the_range_._successor_list[v_].size() == 0) {
            Q.pop();
            continue;
          }
          /*We have to make sure that the last vertex in the queue*/
          if (the_range_._successor_list[v_].size() > 0) {
            /*Reset the iterator for the successor list*/
            successor_iterator = the_range_._successor_list[v_].begin();
            break;
          }
        }
      }
      if (the_range_._successor_list[v_].size() > 0) {
        the_range_._ready_to_process = no;
        the_range_._pred_done_counter[*successor_iterator] += 1;
        if (the_range_._pred_done_counter[*successor_iterator] == the_range_._predecessor_list[*successor_iterator].size()) {
          the_range_._ready_to_process = yes;
          Q.push(*successor_iterator);
        }
      }
    }

    dag_range_iterator& operator++() {
      ++successor_iterator;
      advance_next();
      return *this;
    }

    auto operator*() {
      return std::tuple<vertex_id_type, vertex_id_type, ready_to_process>(v_, *successor_iterator, the_range_._ready_to_process);
    }
    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };
    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }
  };
  auto begin() { return dag_range_iterator(*this); }
  auto end() { return typename dag_range_iterator::end_sentinel_type(); }

private:
  Graph&                the_graph_;
  Queue                 Q_;
  VOV                   _predecessor_list;
  VOV                   _successor_list;
  std::vector<uint64_t> _pred_done_counter;
  ready_to_process      _ready_to_process;
};

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_DAG_RANGE_HPP
