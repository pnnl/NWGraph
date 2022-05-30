/**
 * @file connected_components.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Tony Liu
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *
 */

#ifndef CONNECTED_COMPONENT_EXPERIMENTAL_HPP
#define CONNECTED_COMPONENT_EXPERIMENTAL_HPP

#include "nwgraph/adaptors/bfs_edge_range.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"
#include "nwgraph/util/AtomicBitVector.hpp"
#include "nwgraph/adaptors/cyclic_neighbor_range.hpp"
#include <atomic>
#include <iostream>
#include <random>
#include <unordered_map>

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#else
#include <algorithm>
#include <execution>
#endif

namespace nw {
namespace graph {

template<class T>
inline bool writeMin(T& old, T& next) {
  T    prev;
  bool success = false;
  do
    prev = old;
  while (prev > next && !(success = nw::graph::cas(old, prev, next)));
  return success;
}

template <typename T>
inline bool compare_and_swap(T& x, T old_val, T new_val) {
  return __sync_bool_compare_and_swap(&x, *(&old_val), *(&new_val));
}
template <typename T>
struct atomwrapper {
  std::atomic<T> _a;

  atomwrapper() : _a() {}

  explicit atomwrapper(const std::atomic<T>& a) : _a(a.load()) {}

  atomwrapper(const atomwrapper& other) : _a(other._a.load()) {}

  atomwrapper& operator=(const atomwrapper& other) {
    _a.store(other._a.load());
    return *this;
  }
};

// BFS-based connected component algorithm
template <adjacency_list_graph Graph, typename T>
void compute_connected_components(Graph& A, std::vector<T>& component_ids) {
  size_t         N                        = A.size();
  std::atomic<T> global_component_counter = -1;
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto vtx) {
    if (std::numeric_limits<T>::max() == component_ids[vtx]) {
      global_component_counter++;
      component_ids[vtx] = global_component_counter.load();
      //      std::cout << "--------vertex--------------" << vtx << "
      //      component_ids[vtx]: " << component_ids[vtx] << std::endl;
      bfs_edge_range3 ranges(A, vtx);
      for (auto ite = ranges.begin(); ite != ranges.end(); ++ite) {
        // auto u = std::get<0>(*ite);
        auto v = target(A, *ite);
        //auto v           = std::get<1>(*ite);
        component_ids[v] = global_component_counter.load();
      }
    }
  });
}

template <typename T>
void hook(T u, T v, std::vector<T>& comp) {
  T p1 = comp[u];
  T p2 = comp[v];
  T high, low;
  while (p1 != p2) {
    high              = std::max(p1, p2);
    low               = p1 + p2 - high;
    volatile T p_high = comp[high];
    if (p_high == low || compare_and_swap(comp[high], high, low)) break;
    p1 = comp[comp[high]];
    p2 = comp[low];
  }    // while
}

template <typename T>
void compress(std::vector<T>& comp) {
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(comp.size()), [&](auto n) {
    while (comp[n] != comp[comp[n]])
      comp[n] = comp[comp[n]];
  });
}

template <typename T>
T findDominantComponentID(const std::vector<T>& comp, size_t nsamples = 1024) {
  if (0 == comp.size()) return 0;
  std::unordered_map<T, size_t>                              sample_counts(32);
  typedef typename std::unordered_map<T, size_t>::value_type kvp_type;
  std::mt19937                                               gen;
  std::uniform_int_distribution<T>                           distribution(0, comp.size() - 1);
  for (size_t i = 0; i < nsamples; i++) {
    T n = distribution(gen);
    ++sample_counts[comp[n]];
  }
  auto dominant =
      std::max_element(sample_counts.begin(), sample_counts.end(), [](const kvp_type& a, const kvp_type& b) { return a.second < b.second; });
  /*
    float frac_of_graph = static_cast<float>(dominant->second) / nsamples;
    std::cout
      << "Skipping largest intermediate component (ID: " << dominant->first
      << ", approx. " << static_cast<int>(frac_of_graph * 100)
      << "% of the graph)" << std::endl;
  */
  return dominant->first;
}

template <adjacency_list_graph Graph, typename T>
void push(const Graph& g, const T u, std::vector<T>& comp) {
  for (auto j = g[u].begin(); j != g[u].end(); ++j) {
    auto v = target(g, *j);
    //auto v = std::get<0>(*j);
    hook(u, v, comp);
  }
}

template <adjacency_list_graph Graph, typename T>
void link(const Graph& g, const T u, std::vector<T>& comp, const size_t neighbor_bound) {
  size_t i = 0;
  for (auto j = g[u].begin(); j != g[u].end() && i < neighbor_bound; ++j, ++i) {
    auto v = target(g, *j);
    hook(u, v, comp);
  }
}

// fetch the smallest comp_id among u's neighbors
template <adjacency_list_graph Graph, typename T>
bool pull(const Graph& g, const T u, std::vector<T>& comp) {
  T min_compid = comp[u];
  T v;
  for (auto j = g[u].begin(); j != g[u].end(); ++j) {
    v          = target(g, *j);
    //v          = std::get<0>(*j);
    min_compid = std::min(min_compid, comp[v]);
  }
  bool change = false;
  T    p1     = comp[u];
  T    p2     = min_compid;
  T    high = comp[u];
  T    low = min_compid;
  while (p1 != p2) {
    high              = std::max(p1, p2);
    low               = p1 + p2 - high;
    volatile T p_high = comp[high];
    if (p_high == low || compare_and_swap(comp[high], high, low)) {
      change = true;
      break;
    }
    p1 = comp[comp[high]];
    p2 = comp[low];
  }    // while
  return change;
}

template <adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
std::vector<T> compute_connected_components_v1(const Graph& g) {
  size_t N = g.size();
  // std::vector<atomwrapper<T>> comp(N,
  // atomwrapper<T>(-1));
  std::vector<atomwrapper<T>> comp(N);

  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto n) { comp[n]._a.store(n); });

  bool change = true;
  for (size_t num_iter = 0; num_iter < 2; ++num_iter) {
    if (false == change) {
      std::cout << "Shiloach-Vishkin took " << num_iter << " iterations" << std::endl;
      break;
    }
    change = false;
    /*
     * TODO bfs edge based parallel iterating over graph
      bfs_edge_range3 range(g, 0);
      auto first = range.begin();
      auto last = range.end();
      std::for_each(
    std::execution::par_unseq,
      first, last, [&](auto&& ite) {
        auto u = std::get<0>(*ite);
        auto v = std::get<1>(*ite);

          if (v != comp[v]._a.load()) return;
          auto p1 = comp[u]._a.load();
          auto p2 = comp[v]._a.load();
          while (p1 != p2) {
            auto high = std::max(p1, p2);
            auto low = p1 + p2 - high;
            auto p_high = comp[high]._a.load();
            if (p_high == low || comp[high]._a.compare_exchange_weak(p_high,
    comp[low]._a)) { change = true; break;
            }
            high = comp[high]._a.load();
            p1 = comp[high]._a.load();
            p2 = comp[low]._a.load();
          }
        });
    */
    std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto u) {
      T v;
      for (auto j = g[u].begin(); j != g[u].end(); ++j) {
        v = target(g, *j);
        //v = std::get<0>(*j);
        //      if (v != comp[v]._a.load()) continue;
        auto p1 = comp[u]._a.load();
        auto p2 = comp[v]._a.load();
        while (p1 != p2) {
          auto high   = std::max(p1, p2);
          auto low    = p1 + p2 - high;
          auto p_high = comp[high]._a.load();
          if (p_high == low || comp[high]._a.compare_exchange_weak(p_high, comp[low]._a)) {
            change = true;
            break;
          }
          high = comp[high]._a.load();
          p1   = comp[high]._a.load();
          p2   = comp[low]._a.load();
        }    // while
      }
    });
  }
  std::vector<T> res(N);
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto n) {
    auto m = n;
    //    while (comp[m]._a.load() != m)  m = comp[m]._a.load();
    while (comp[m]._a.load() != comp[comp[m]._a.load()]._a.load()) {
      comp[m]._a.store(comp[comp[m]._a.load()]._a.load());
      m = comp[m]._a.load();
    }
    res[n] = comp[n]._a.load();
    // std::cout << n <<":" <<  res[n] << std::endl;
    /*
          atomwrapper<T> m(std::atomic<T>(n));
          while ( comp[m._a.load()] != comp[comp[m._a.load()]._a.load()]) {
            m._a.store(comp[m._a.load()]);
          }
          res[n] = comp[m._a.load()].a_.load();
        */
  });
  // compress(comp);
  return res;
}    // compute_connected_components_v1

template <adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
std::vector<T> compute_connected_components_v2(const Graph& g) {
  size_t         N = g.size();
  std::vector<T> comp(g.size());
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto n) { comp[n] = n; });

  bool change = true;

  for (size_t num_iter = 0; num_iter < 1; ++num_iter) {
    if (false == change) {
      std::cout << "PULL-cc took " << num_iter << " iterations" << std::endl;
      break;
    }
    change = false;

    std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto u) { change = pull(g, u, comp); });
    std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto u) { push(g, u, comp); });

    compress(comp);
  }
  return comp;
}    // compute_connected_components_v2

template <adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
std::vector<T> ccv1(const Graph& g) {
  std::vector<T> comp(g.size());
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto n) { comp[n] = n; });

  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto u) { push(g, u, comp); });
  compress(comp);
  return comp;
}

template<typename Execution, adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
std::vector<T> lpcc(Execution& exec, Graph& g, int num_bins = 32) {
  //nw::util::life_timer _(__func__);
  size_t     num_nodes = g.size();
  std::vector<T> comp(num_nodes);
  nw::graph::AtomicBitVector   visited(num_nodes);
  std::vector<T> cur(num_nodes), next;
  std::for_each(exec,
  counting_iterator<T>(0), counting_iterator<T>(num_nodes), [&](auto n) { 
    comp[n] = n;
    cur[n] = n;
  });

  std::vector<T> frontier[num_bins];
  auto propagate = [&](auto& g, auto& cur, auto& bitmap, auto& labels) {
    tbb::parallel_for(tbb::blocked_range<T>(0ul, cur.size()), [&](tbb::blocked_range<T>& r) {
      int worker_index = tbb::this_task_arena::current_thread_index();
      for (auto i = r.begin(), e = r.end(); i < e; ++i) {
        auto x = cur[i];
        auto labelx = labels[x];
        //all neighbors of hyperedges are hypernode, vice versa
        std::for_each(g[x].begin(), g[x].end(), [&](auto &&j) {
          auto y = std::get<0>(j);
          if (labelx < labels[y]) {
            if (writeMin(labels[y], labelx)) {
              if (0 == bitmap.atomic_get(y) && 0 == bitmap.atomic_set(y))
                frontier[worker_index].push_back(y);
            }
          }
        });
      } //for
    }, tbb::auto_partitioner());
  };
  std::vector<size_t> size_array(num_bins);
  auto curtonext = [&](auto& next) {
    size_t size = 0;
    for (int i = 0; i < num_bins; ++i) {
      //calculate the size of each thread-local frontier
      size_array[i] = size;
      //accumulate the total size of all thread-local frontiers
      size += frontier[i].size();
    }
    //resize next frontier
    next.resize(size); 
    std::for_each(exec, counting_iterator(0), counting_iterator(num_bins), [&](auto i) {
      //copy each thread-local frontier to next frontier based on their size offset
      auto begin = std::next(next.begin(), size_array[i]);
      std::copy(exec, frontier[i].begin(), frontier[i].end(), begin);
      frontier[i].clear();
    });
  };
  while (!cur.empty()) {
    propagate(g, cur, visited, comp);
    curtonext(next);
    visited.clear();
    cur.clear();
    cur.swap(next);
  } //while

  return comp;
}


template<typename Execution, adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
std::vector<T> lpcc_cyclic(Execution& exec, Graph& g, int num_bins = 32) {
  //nw::util::life_timer _(__func__);
  size_t     num_nodes = g.size();
  std::vector<T> comp(num_nodes);
  nw::graph::AtomicBitVector   visited(num_nodes);
  std::vector<T> cur(num_nodes), next;
  std::for_each(exec,
  counting_iterator<T>(0), counting_iterator<T>(num_nodes), [&](auto n) { 
    comp[n] = n;
    cur[n] = n;
  });

  std::vector<T> frontier[num_bins];
  auto propagate = [&](auto& g, auto& cur, auto& bitmap, auto& labels) {
    tbb::parallel_for(cyclic_neighbor_range(g, num_bins), [&](auto& i) {
      int worker_index = tbb::this_task_arena::current_thread_index();
      for (auto&& j = i.begin(); j != i.end(); ++j) {
        auto&& [x, x_ngh] = *j;
        auto labelx = labels[x];
        //all neighbors of hyperedges are hypernode, vice versa
        for (auto &&[y] : x_ngh) {
          if (labelx < labels[y]) {
            if (writeMin(labels[y], labelx)) {
              if (0 == bitmap.atomic_get(y) && 0 == bitmap.atomic_set(y))
                frontier[worker_index].push_back(y);
            }
          }
        }//for each neighbor
      } //for
    }, tbb::auto_partitioner());
  };
  std::vector<size_t> size_array(num_bins);
  auto curtonext = [&](auto& next) {
    size_t size = 0;
    for (int i = 0; i < num_bins; ++i) {
      //calculate the size of each thread-local frontier
      size_array[i] = size;
      //accumulate the total size of all thread-local frontiers
      size += frontier[i].size();
    }
    //resize next frontier
    next.resize(size); 
    std::for_each(exec, counting_iterator(0), counting_iterator(num_bins), [&](auto i) {
      //copy each thread-local frontier to next frontier based on their size offset
      auto begin = std::next(next.begin(), size_array[i]);
      std::copy(exec, frontier[i].begin(), frontier[i].end(), begin);
      frontier[i].clear();
    });
  };
  while (!cur.empty()) {
    propagate(g, cur, visited, comp);
    curtonext(next);
    visited.clear();
    cur.clear();
    cur.swap(next);
  } //while

  return comp;
}

template <adjacency_list_graph Graph, adjacency_list_graph Graph2, typename T = vertex_id_t<Graph>>
std::vector<T> Afforest(const Graph& g, Graph2& t_graph, size_t neighbor_bound = 2) {
  std::vector<T> comp(g.size());
  // set component id of vertex v to v
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto n) { comp[n] = n; });
  // approximate the dominant component by linking certain neighbors of each
  // vertex v (a sparse subgraph)
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()),
                [&](auto u) { link(g, u, comp, neighbor_bound); });
  compress(comp);
  // Sample certain vertices to find dominant component id
  T dominant_c = findDominantComponentID(comp);
  // link the rest vertices outside of dominant component
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto u) {
    if (dominant_c != comp[u]) {
      push(g, u, comp);
      if (t_graph.size() != 0) {
        push(t_graph, u, comp);
      }
    }
  });
  compress(comp);

  return comp;
}

template <adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
std::vector<T> ccv5(const Graph& g) {
  size_t         N = g.size();
  std::vector<T> comp(g.size());
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto n) { comp[n] = n; });

  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto u) { pull(g, u, comp); });
  T dominant_c = findDominantComponentID(comp);
  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(N), [&](auto u) {
    if (dominant_c != comp[u]) push(g, u, comp);
  });

  compress(comp);
  return comp;
}

template <adjacency_list_graph Graph, typename T= vertex_id_t<Graph>>
auto sv_v6(const Graph& g) {
  std::vector<T> comp(g.size());

  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto n) { comp[n] = n; });

  bool change = true;

  auto G = g.begin();

  for (size_t num_iter = 0; num_iter < g.size(); ++num_iter) {
    if (false == change) {
      // std::cout << "Shiloach-Vishkin took " << num_iter << " iterations" <<
      // std::endl;
      break;
    }
    change = false;

    std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto u) {
      for (auto&& elt : G[u]) {
        auto v = target(g, elt);
        T comp_u = comp[u];
        T comp_v = comp[v];
        if (comp_u == comp_v) continue;
        T high_comp = comp_u > comp_v ? comp_u : comp_v;
        T low_comp  = comp_u + (comp_v - high_comp);
        if (high_comp == comp[high_comp]) {
          change          = true;
          comp[high_comp] = low_comp;
        }
      }
    });

    for (T n = 0; n < g.size(); n++) {
      while (comp[n] != comp[comp[n]]) {
        comp[n] = comp[comp[n]];
      }
    }
  }

  return comp;
}

template <adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
auto sv_v8(Graph& g) {
  std::vector<T> comp(g.size());

  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto n) { comp[n] = n; });

  bool change = true;

  auto G = g.begin();

  for (size_t num_iter = 0; num_iter < g.size(); ++num_iter) {
    if (false == change) {
      std::cout << "Shiloach-Vishkin took " << num_iter << " iterations" << std::endl;
      break;
    }
    change = false;

    std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto u) {
      auto Gu = G[u];
      // tbb::parallel_for(G[u], [&] (auto& Gu) {
      for (auto&& elt : Gu) {
        auto v = target(g, elt);
        T comp_u = comp[u];
        T comp_v = comp[v];
        if (comp_u == comp_v) continue;
        T high_comp = std::max(comp_u, comp_v);

        T low_comp = comp_u + (comp_v - high_comp);
        if (high_comp == comp[high_comp]) {
          change          = true;
          comp[high_comp] = low_comp;
        }
      }
      // });
    });

    for (T n = 0; n < g.size(); n++) {
      while (comp[n] != comp[comp[n]]) {
        comp[n] = comp[comp[n]];
      }
    }
  }

  return comp;
}

template <adjacency_list_graph Graph, typename T = vertex_id_t<Graph>>
auto sv_v9(Graph& g) {
  std::vector<T> comp(g.size());

  std::for_each(std::execution::par_unseq, counting_iterator<T>(0), counting_iterator<T>(g.size()), [&](auto n) { comp[n] = n; });

  bool change = true;

  for (size_t num_iter = 0; num_iter < g.size(); ++num_iter) {
    if (false == change) {
      std::cout << "Shiloach-Vishkin took " << num_iter << " iterations" << std::endl;
      break;
    }
    change = false;

    tbb::parallel_for(edge_range(g), [&](auto&& sub) {
      for (auto&& [u, v] : sub) {
        T comp_u = comp[u];
        T comp_v = comp[v];
        if (comp_u == comp_v) continue;
        T high_comp = std::max(comp_u, comp_v);

        T low_comp = comp_u + (comp_v - high_comp);
        if (high_comp == comp[high_comp]) {
          change          = true;
          comp[high_comp] = low_comp;
        }
      }
    });

    for (T n = 0; n < g.size(); n++) {
      while (comp[n] != comp[comp[n]]) {
        comp[n] = comp[comp[n]];
      }
    }
  }

  return comp;
}

}    // namespace graph
}    // namespace nw
#endif    // CONNECTED_COMPONENT_EXPERIMENTAL_HPP
