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

#ifndef DELTA_STEPPING_EXPERIMENTAL_HPP
#define DELTA_STEPPING_EXPERIMENTAL_HPP

#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "nwgraph/algorithms/delta_stepping.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/util/atomic.hpp"

#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/util/timer.hpp"
#include "nwgraph/util/util.hpp"

#include "tbb/concurrent_vector.h"
#include "tbb/parallel_for_each.h"
#include "tbb/queuing_mutex.h"

namespace nw {
namespace graph {


// Inspired by gapbs implementation
template <class distance_t, adjacency_list_graph Graph, class Id, class T>
auto delta_stepping_v6(const Graph& graph, Id source, T delta) {
  tbb::concurrent_vector<tbb::concurrent_vector<Id>> bins(1);
  std::size_t                                        top_bin = 0;

  std::vector<distance_t> tdist(num_vertices(graph));
  tbb::parallel_for_each(tdist, [&](auto&& d) { d = std::numeric_limits<distance_t>::max(); });

  bins[top_bin].push_back(source);
  tdist[source] = 0;

  auto relax = [&](Id i, Id j, auto wt) {
    auto new_distance = tdist[i] + wt;
    if (new_distance < tdist[j]) {
      tdist[j]        = new_distance;
      size_t dest_bin = new_distance / delta;
      if (dest_bin >= bins.size()) {
        bins.resize(dest_bin + 1);
      }
      bins[dest_bin].push_back(j);
    }
  };

  tbb::concurrent_vector<Id> frontier;

  while (top_bin < bins.size()) {
    frontier.resize(0);
    std::swap(frontier, bins[top_bin]);
    tbb::parallel_for_each(frontier, [&](Id i) {
      if (tdist[i] >= delta * top_bin) {
        std::for_each(graph[i].begin(), graph[i].end(), [&](auto&& elt) { std::apply([&](Id j, auto wt) { relax(i, j, wt); }, std::move(elt)); });
      }
    });

    while (top_bin < bins.size() && bins[top_bin].size() == 0) {
      ++top_bin;
    }
  }

  return tdist;
}

// Inspired by gapbs implementation
template <class distance_t, adjacency_list_graph Graph, class Id, class T, class Weight>
auto delta_stepping_v8(const Graph& graph, Id source, T delta,
    Weight weight = [](auto& e) -> auto& { return std::get<1>(e); }) {
  Id                                                 N = num_vertices(graph);
  std::vector<std::atomic<distance_t>>               tdist(N);
  tbb::concurrent_vector<tbb::concurrent_vector<Id>> bins(1);
  std::size_t                                        top_bin = 0;
  for (auto&& d : tdist) {
    d = std::numeric_limits<distance_t>::max();
  }

  bins[top_bin].push_back(source);
  tdist[source] = 0;

  auto relax = [&](Id i, Id j, auto wt) {
    distance_t new_distance = tdist[i] + wt;
    distance_t old_distance = tdist[j];
    if (new_distance < old_distance) {
      bool changed_dist = true;
      while (!tdist[j].compare_exchange_strong(old_distance, new_distance)) {
        if (old_distance <= new_distance) {
          changed_dist = false;
          break;
        }
      }
      if (changed_dist) {
        std::size_t dest_bin = new_distance / delta;
        // if (dest_bin >= bins.size()) {
        bins.grow_to_at_least(dest_bin + 1);
        // }
        bins[dest_bin].push_back(j);
      }
    }
  };

  tbb::concurrent_vector<Id> frontier;

  while (top_bin < bins.size()) {

    frontier.resize(0);
    std::swap(frontier, bins[top_bin]);

    std::for_each(std::execution::par_unseq, frontier.begin(), frontier.end(), [&](Id i) {
      if (tdist[i] >= delta * top_bin) {
        std::for_each(std::execution::par_unseq, graph[i].begin(), graph[i].end(), [&](auto&& elt) {
          auto&& j = target(graph, elt);
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

template <class distance_t, adjacency_list_graph Graph, class Id, class T, class Weight>
auto delta_stepping_v9(const Graph& graph, Id source, T delta,
  Weight weight = [](auto& e) -> auto& { return std::get<1>(e); }) {
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
    tbb::parallel_for_each(frontier, [&](auto&& i) {
      if (tdist[i] >= delta * top_bin) {
        for (auto&& elt : graph[i]) {
          auto&& j = target(graph, elt);
          auto&& wt = weight(elt);
          //auto&& [j, wt] = elt;    // i == v
          relax(i, j, wt);
        }
      }
    });

    while (top_bin < bins.size() && bins[top_bin].size() == 0) {
      bins[top_bin++].shrink_to_fit();
    }
  }
  return tdist;
}

template <class distance_t, adjacency_list_graph Graph, class Id, class T, class Weight>
auto delta_stepping_v10(const Graph& graph, Id source, T delta,
  Weight weight = [](auto& e) -> auto& { return std::get<1>(e); }) {
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
    tbb::parallel_for(tbb::blocked_range(0ul, frontier.size()), [&](auto&& range) {
      for (auto id = range.begin(), e = range.end(); id < e; ++id) {
        auto i = frontier[id];
        if (tdist[i] >= delta * top_bin) {
          for (auto&& elt : graph[i]) {
            auto&& j = target(graph, elt);
            auto&& wt = weight(elt);
            //auto&& [j, wt] = elt;    // i == v
            relax(i, j, wt);
          }
        }
      }
    });

    while (top_bin < bins.size() && bins[top_bin].size() == 0) {
      bins[top_bin++].shrink_to_fit();
    }
  }
  return tdist;
}

template <class distance_t, adjacency_list_graph Graph, class Id, class T, class Weight>
auto delta_stepping_v11(const Graph& graph, Id source, T delta,
  Weight weight = [](auto& e) -> auto& { return std::get<1>(e); }) {
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
    tbb::parallel_for(tbb::blocked_range(0ul, frontier.size()), [&](auto&& range) {
      for (auto id = range.begin(), e = range.end(); id < e; ++id) {
        auto i = frontier[id];
        if (tdist[i] >= delta * top_bin) {
          tbb::parallel_for(graph[i], [&](auto&& range) {
            for (auto&& elt : range) {
              auto&& j = target(graph, elt);
              auto&& wt = weight(elt);
              //auto&& [j, wt] = elt;    // i == v
              relax(i, j, wt);
            }
          });
        }
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
#endif    // DELTA_STEPPING_EXPERIMENTAL_HPP
