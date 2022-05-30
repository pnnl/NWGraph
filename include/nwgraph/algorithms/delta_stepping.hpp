/**
 * @file delta_stepping.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   Tony Liu
 *
 */

#ifndef DELTA_STEPPING_HPP
#define DELTA_STEPPING_HPP

#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/util/atomic.hpp"

#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/util/timer.hpp"
#include "nwgraph/util/util.hpp"

#include "tbb/concurrent_vector.h"
#include "tbb/parallel_for_each.h"
#include "tbb/queuing_mutex.h"

namespace nw {
namespace graph {

template <class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type>>
class _priority_queue : public std::priority_queue<T, Container, Compare> {
  using base = std::priority_queue<T, Container, Compare>;

public:
  explicit _priority_queue(const Compare& compare = Compare(), const Container& cont = Container()) : base(compare, cont) {}
  auto begin() { return base::c.begin(); }
  auto end() { return base::c.begin(); }
  auto begin() const { return base::c.begin(); }
  auto end() const { return base::c.begin(); }
};

template <class distance_t, adjacency_list_graph Graph, class Id, class Weight>
auto delta_stepping_m1(
    const Graph& graph, Id source, distance_t, Weight weight = [](auto& e) -> auto& { return std::get<1>(e); }) {
  std::vector<distance_t> tdist(num_vertices(graph), std::numeric_limits<distance_t>::max());
  size_t                  top_bin = 0;

  auto tdist_comp = [&](Id a, Id b) { return tdist[a] > tdist[b]; };

  _priority_queue<Id, std::vector<Id>, std::function<bool(Id, Id)>> Q(tdist_comp), frontier(tdist_comp);

  Q.push(source);
  tdist[source] = 0;

  auto relax = [&](Id i, Id j, auto wt) {
    auto new_distance = tdist[i] + wt;
    if (new_distance < tdist[j]) {
      tdist[j] = new_distance;
      Q.push(j);
    }
  };

  while (!Q.empty()) {

    frontier = _priority_queue<Id, std::vector<Id>, std::function<bool(Id, Id)>>(tdist_comp);
    std::swap(frontier, Q);

    std::for_each(frontier.begin(), frontier.end(), [&](Id i) {
      std::for_each(graph[i].begin(), graph[i].end(), [&](auto&& elt) {
        auto j  = target(graph, elt);
        auto wt = weight(elt);
        //auto&& [j, wt] = elt;    // i == v
        relax(i, j, wt);
      });
    });
  }

  return tdist;
}

/**
 * Delta-stepping single-source shortest-paths.
 *
 * Sequential implementation of delta-stepping single-source shortest-paths
 *
 * @verbatim embed:rst:inline :cite:`MEYER2003114`.@endverbatim 
 * @tparam distance_t Type of distance measure.
 * @tparam Graph Type of input graph.  Must meet the requirements of adjacency_list_graph.
 * @tparam T Type of delta parameter.
 * @tparam Weight Type of function used to compute edge weights.
 * @param graph The input graph.
 * @param source The starting vertex.
 * @param delta The delta parameter for the algorithm.
 * @param weight Function to compute weight of an edge.
 */
template <class distance_t, adjacency_list_graph Graph, class T, class Weight>
auto delta_stepping(
    const Graph& graph, vertex_id_t<Graph> source, T delta, Weight weight = [](auto& e) -> auto& { return std::get<1>(e); }) {
  using Id = vertex_id_t<Graph>;
  std::vector<distance_t>      tdist(num_vertices(graph), std::numeric_limits<distance_t>::max());
  std::vector<std::vector<Id>> bins(1);
  std::size_t                  top_bin = 0;

  bins[top_bin].push_back(source);
  tdist[source] = 0;

  auto relax = [&](Id i, Id j, auto wt) {
    auto new_distance = tdist[i] + wt;
    if (new_distance < tdist[j]) {
      tdist[j]             = new_distance;
      std::size_t dest_bin = new_distance / delta;
      if (dest_bin >= bins.size()) {
        bins.resize(dest_bin + 1);
      }
      bins[dest_bin].push_back(j);
    }
  };

  std::vector<Id> frontier;

  while (top_bin < bins.size()) {

    frontier.resize(0);
    std::swap(frontier, bins[top_bin]);

    std::for_each(frontier.begin(), frontier.end(), [&](Id i) {
      if (tdist[i] >= delta * top_bin) {
        std::for_each(graph[i].begin(), graph[i].end(), [&](auto&& elt) {
          auto&& j  = target(graph, elt);
          auto&& wt = weight(elt);
          //auto&& [j, wt] = elt;    // i == v
          relax(i, j, wt);
        });
      }
    });

    while (top_bin < bins.size() && bins[top_bin].size() == 0) {
      ++top_bin;
    }
  }

  return tdist;
}

/**
 * Delta-stepping single-source shortest-paths.
 *
 * Parallel implementation of delta-stepping single-source shortest-paths 
 * @verbatim embed:rst:inline :cite:`MEYER2003114`.@endverbatim  Uses Intel TBB
 * for parallelization.
 *
 * @tparam distance_t Type of distance measure.
 * @tparam Graph Type of input graph.  Must meet the requirements of adjacency_list_graph.
 * @tparam T Type of delta parameter.
 * @param graph The input graph.
 * @param source The starting vertex.
 * @param delta The delta parameter for the algorithm.
 */
template <class distance_t, adjacency_list_graph Graph, class T>
auto delta_stepping(const Graph& graph, vertex_id_t<Graph> source, T delta) {
  using Id = vertex_id_t<Graph>;
  tbb::queuing_mutex                                 lock;
  std::atomic<std::size_t>                           size = 1;
  tbb::concurrent_vector<tbb::concurrent_vector<Id>> bins(size);
  std::size_t                                        top_bin = 0;

  std::vector<std::atomic<distance_t>> tdist(num_vertices(graph));
  tbb::parallel_for_each(tdist, [](auto&& d) { d = std::numeric_limits<distance_t>::max(); });

  bins[top_bin].push_back(source);
  tdist[source] = 0;

  auto relax = [&](Id i, Id j, auto wt) {
    distance_t next = nw::graph::acquire(tdist[i]) + wt;
    distance_t prev = nw::graph::acquire(tdist[j]);
    bool       success;
    while (next < prev && !(success = nw::graph::cas(tdist[j], prev, next))) {
    }
    if (!success) return;

    // inject this into the right bin (double-checked locking to serialize
    // resizing because tbb::concurrent_vector resizing is not thread safe
    // https://www.threadingbuildingblocks.org/docs/help/hh_goto.htm?index.htm#tbb_userguide/Advanced_Idiom_Waiting_on_an_Element.html)
    std::size_t bin = next / delta;
    if (nw::graph::acquire(size) < bin + 1) {
      tbb::queuing_mutex::scoped_lock _(lock);
      if (nw::graph::acquire(size) < bin + 1) {
        bins.grow_to_at_least(bin + 1);
        nw::graph::release(size, bin + 1);
      }
    }
    bins[bin].push_back(j);
  };

  tbb::concurrent_vector<Id> frontier;

  while (top_bin < bins.size()) {
    frontier.resize(0);
    std::swap(frontier, bins[top_bin]);
    tbb::parallel_for_each(frontier, [&](auto&& u) {
      if (tdist[u] >= delta * top_bin) {
        nw::graph::parallel_for(graph[u], [&](auto&& v, auto&& wt) { relax(u, v, wt); });
      }
    });

    while (top_bin < bins.size() && bins[top_bin].size() == 0) {
      bins[top_bin++].shrink_to_fit();
    }
  }
  return tdist;
}

}    // namespace graph
}    // namespace nw
#endif    // DELTA_STEPPING_HPP
