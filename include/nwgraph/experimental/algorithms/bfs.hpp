/**
 * @file bfs.hpp
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

#ifndef NW_GRAPH_EXPERIMENTAL_BFS_HPP
#define NW_GRAPH_EXPERIMENTAL_BFS_HPP

#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/graph_traits.hpp"
#include "nwgraph/util/AtomicBitVector.hpp"
#include "nwgraph/util/atomic.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/adaptors/neighbor_range.hpp"
#include "nwgraph/adaptors/cyclic_range_adaptor.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"
#include <queue>

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for_each.h>


namespace nw {
namespace graph {

template <adjacency_list_graph Graph>
auto bfs_v4(const Graph& graph, typename graph_traits<Graph>::vertex_id_type root) {
  using vertex_id_type = typename graph_traits<Graph>::vertex_id_type;

  tbb::concurrent_queue<vertex_id_type> q1, q2;
  std::vector                       level(graph.max() + 1, std::numeric_limits<vertex_id_type>::max());
  std::vector                       parents(graph.max() + 1, std::numeric_limits<vertex_id_type>::max());
  size_t                            lvl = 0;

  q1.push(root);
  level[root] = lvl++;

  while (!q1.empty()) {

    std::for_each(std::execution::par_unseq, q1.unsafe_begin(), q1.unsafe_end(), [&](vertex_id_type u) {
      std::for_each(graph[u].begin(), graph[u].end(), [&](auto&& x) {
        vertex_id_type v = target(graph, x);
        if (level[v] == std::numeric_limits<vertex_id_type>::max()) {
          q2.push(v);
          level[v]   = lvl;
          parents[v] = u;
        }
      });
    });
    q1(q2);
    //q1.internal_swap(q2);
    q2.clear();

    ++lvl;
  }
  return parents;
}

template <adjacency_list_graph Graph>
auto bfs_v6(const Graph& graph, typename graph_traits<Graph>::vertex_id_type root) {
  using vertex_id_type = typename graph_traits<Graph>::vertex_id_type;

  tbb::concurrent_vector<vertex_id_type> q1, q2;
  std::vector<vertex_id_type>            level(num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  std::vector<vertex_id_type>            parents(num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  size_t                                 lvl = 0;

  q1.push_back(root);
  level[root]   = lvl++;
  parents[root] = root;

  while (!q1.empty()) {
    std::for_each(q1.begin(), q1.end(), [&](vertex_id_type u) {
      std::for_each(graph[u].begin(), graph[u].end(), [&](auto&& x) {
        vertex_id_type v = target(graph, x);
        if (level[v] == std::numeric_limits<vertex_id_type>::max()) {
          q2.push_back(v);
          level[v]   = lvl;
          parents[v] = u;
        }
      });
    });
    std::swap(q1, q2);
    q2.clear();
    ++lvl;
  }
  return parents;
}

template <adjacency_list_graph Graph>
auto bfs_v7(const Graph& graph, typename graph_traits<Graph>::vertex_id_type root) {
  using vertex_id_type = typename graph_traits<Graph>::vertex_id_type;

  tbb::concurrent_vector<vertex_id_type>   q1, q2;
  std::vector<std::atomic<vertex_id_type>> level(num_vertices(graph));
  for (size_t i = 0; i < graph.size(); ++i) {
    level[i] = std::numeric_limits<vertex_id_type>::max();
  }
  std::vector parents(num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  size_t      lvl = 0;

  q1.push_back(root);
  level[root]   = lvl++;
  parents[root] = root;

  while (!q1.empty()) {
    std::for_each(std::execution::par_unseq, q1.begin(), q1.end(), [&](vertex_id_type u) {
      std::for_each(graph[u].begin(), graph[u].end(), [&](auto&& x) {
        vertex_id_type v = target(graph, x);
        vertex_id_type old_lvl = level[v];
        vertex_id_type new_lvl = lvl;
        if (new_lvl < old_lvl) {
          bool changed = true;
          while (!level[v].compare_exchange_strong(old_lvl, new_lvl)) {
            if (old_lvl <= new_lvl) {
              changed = false;
              break;
            }
          }
          if (changed) {
            q2.push_back(v);
            parents[v] = u;
          }
        }
      });
    });
    std::swap(q1, q2);
    q2.clear();

    ++lvl;
  }
  return parents;
}

template <adjacency_list_graph Graph>
auto bfs_v8(const Graph& graph, typename graph_traits<Graph>::vertex_id_type root) {

  using vertex_id_type = typename graph_traits<Graph>::vertex_id_type;

  tbb::concurrent_queue<vertex_id_type> q[2];// q1, q2;
  std::vector                       level(num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  std::vector                       parents(num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  size_t                            lvl = 0;

  bool cur = false;
  q[cur].push(root);
  //q1.push(root);
  level[root]   = lvl++;
  parents[root] = root;

  while (!q[cur].empty()) {

    std::for_each(std::execution::par_unseq, q[cur].unsafe_begin(), q[cur].unsafe_end(), [&](vertex_id_type u) {
      std::for_each(std::execution::par_unseq, graph[u].begin(), graph[u].end(), [&](auto&& x) {
        vertex_id_type v = target(graph, x);
        if (level[v] == std::numeric_limits<vertex_id_type>::max()) {
          q[!cur].push(v);
          level[v]   = lvl;
          parents[v] = u;
        }
      });
    });
    cur = !cur;
    //q1.internal_swap(q2);
    q[!cur].clear();

    ++lvl;
  }
  return parents;
}

template <adjacency_list_graph Graph>
auto bfs_v9(const Graph& graph, typename graph_traits<Graph>::vertex_id_type root) {
  using vertex_id_type = typename graph_traits<Graph>::vertex_id_type;

  const size_t                                        num_bins = 32;
  const size_t                                        bin_mask = 0x1F;
  std::vector<tbb::concurrent_vector<vertex_id_type>> q1(num_bins), q2(num_bins);
  std::vector                                         level(num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  std::vector                                         parents(num_vertices(graph), std::numeric_limits<vertex_id_type>::max());
  size_t                                              lvl = 0;
  parents[root]                                           = root;

  q1[0].push_back(root);
  level[root]   = lvl++;
  parents[root] = root;

  bool done = false;
  while (!done) {
    std::for_each(std::execution::par_unseq, q1.begin(), q1.end(), [&](auto& q) {
      std::for_each(std::execution::par_unseq, q.begin(), q.end(), [&](vertex_id_type u) {
        tbb::parallel_for(graph[u], [&](auto&& gu) {
          std::for_each(gu.begin(), gu.end(), [&](auto&& x) {
            vertex_id_type v = target(graph, x);
            if (level[v] == std::numeric_limits<vertex_id_type>::max()) {
              q2[u & bin_mask].push_back(v);
              level[v]   = lvl;
              parents[v] = u;
            }
          });
        });
      });
    });

    done = true;
    for (size_t i = 0; i < num_bins; ++i) {
      if (q2[i].size() != 0) {
        done = false;
        break;
      }
    }

    std::swap(q1, q2);

    for (size_t i = 0; i < num_bins; ++i) {
      q2[i].clear();
    }

    ++lvl;
  }
  return parents;
}

template <adjacency_list_graph Graph>
[[gnu::noinline]] auto bfs_top_down(const Graph& graph, typename graph_traits<Graph>::vertex_id_type root) {
  using vertex_id_type = typename Graph::vertex_id_type;

  constexpr const std::size_t                         num_bins = 32;
  const std::size_t                                   N        = num_vertices(graph);
  std::vector<tbb::concurrent_vector<vertex_id_type>> q1(num_bins);
  std::vector<tbb::concurrent_vector<vertex_id_type>> q2(num_bins);
  std::vector<vertex_id_type>                         parents(N);

  constexpr const auto null_vertex = null_vertex_v<vertex_id_type>();

  std::fill(std::execution::par_unseq, parents.begin(), parents.end(), null_vertex);

  q1[0].push_back(root);
  parents[root] = root;

  bool done = false;
  while (!done) {
    std::for_each(std::execution::par_unseq, q1.begin(), q1.end(), [&](auto&& q) {
      std::for_each(std::execution::par_unseq, q.begin(), q.end(), [&](auto&& u) {
        nw::graph::parallel_for(graph[u], [&](auto&& v) {
          if (nw::graph::relaxed(parents[v]) == null_vertex) {
            nw::graph::relaxed(parents[v], u);
            q2[u % num_bins].push_back(v);
          }
        });
      });
    });

    done = true;
    for (auto&& q : q2) {
      if (q.size() != 0) {
        done = false;
        break;
      }
    }

    std::swap(q1, q2);

    for (auto&& q : q2) {
      q.clear();
    }
  }
  return parents;
}

template <adjacency_list_graph Graph>
[[gnu::noinline]] auto bfs_top_down_bitmap(const Graph& graph, typename graph_traits<Graph>::vertex_id_type root) {
  using vertex_id_type = vertex_id_t<Graph>;

  constexpr const std::size_t                         num_bins = 32;
  const std::size_t                                   N        = num_vertices(graph);
  std::vector<tbb::concurrent_vector<vertex_id_type>> q1(num_bins);
  std::vector<tbb::concurrent_vector<vertex_id_type>> q2(num_bins);
  std::vector<vertex_id_type>                         parents(N);
  nw::graph::AtomicBitVector                          visited(N);

  constexpr const auto null_vertex = null_vertex_v<vertex_id_type>();

  std::fill(std::execution::par_unseq, parents.begin(), parents.end(), null_vertex);

  q1[0].push_back(root);
  parents[root] = root;
  visited.set(root);

  bool done = false;
  while (!done) {
    std::for_each(std::execution::par_unseq, q1.begin(), q1.end(), [&](auto&& q) {
      std::for_each(std::execution::par_unseq, q.begin(), q.end(), [&](auto&& u) {
        nw::graph::parallel_for(graph[u], [&](auto&& v) {
          if (visited.atomic_get(v) == 0 && visited.atomic_set(v) == 0) {
            parents[v] = u;
            q2[u % num_bins].push_back(v);
          }
        });
      });
    });

    done = true;
    for (auto&& q : q2) {
      if (q.size() != 0) {
        done = false;
        break;
      }
    }

    std::swap(q1, q2);

    for (auto&& q : q2) {
      q.clear();
    }
  }
  return parents;
}

template <adjacency_list_graph Graph, adjacency_list_graph Transpose>
[[gnu::noinline]] auto bfs_bottom_up(const Graph& g, const Transpose& gx, vertex_id_t<Graph> root) {
  using vertex_id_type = vertex_id_t<Graph>;

  const std::size_t          N = num_vertices(gx);
  nw::graph::AtomicBitVector frontier(N);
  nw::graph::AtomicBitVector next(N);

  std::vector<vertex_id_type> parents(N);

  constexpr const auto null_vertex = null_vertex_v<vertex_id_type>();
  std::fill(std::execution::par_unseq, parents.begin(), parents.end(), null_vertex);

  parents[root] = root;
  next.set(root);
  for (vertex_id_type n = 1; n != 0; n = tbb::parallel_reduce(
                                         tbb::blocked_range(0ul, N), 0,
                                         [&](auto&&range, auto n) {
                                           for (auto &&v = range.begin(), e = range.end(); v != e; ++v) {
                                             if (parents[v] == null_vertex) {
                                               for (auto&& elt : gx[v]) {
                                                 vertex_id_type u = target(g, elt);
                                                 if (frontier.get(u)) {
                                                   next.atomic_set(v);
                                                   parents[v] = u;
                                                   ++n;
                                                   break;
                                                 }
                                               }
                                             }
                                           }
                                           return n;
                                         },
                                         std::plus{})) {
    std::swap(frontier, next);
    next.clear();
  }
  return parents;
}


template<adjacency_list_graph Graph>
size_t BU_step(const Graph& g, std::vector<vertex_id_t<Graph>>& parents,
nw::graph::AtomicBitVector<>& front, nw::graph::AtomicBitVector<>& next) {
  size_t N = num_vertices(g);    // number of nodes
  next.clear();
  return tbb::parallel_reduce(
      nw::graph::neighbor_range(g), 0ul,
      [&](auto &&range, auto n) {
        for (auto&& [u, neighbors] : range) {
          if (null_vertex_v<vertex_id_t<Graph>>() == parents[u]) {
            //if u has not found a parent (not visited)
            for (auto&& elt : neighbors) {
              auto v = target(g, elt);
              if (front.get(v)) {
                //if v is not visited
                next.atomic_set(u);
                parents[u] = v;
                ++n;
                break;
              }
            }
          }
        }
        return n;
      }, std::plus{});
}

template<adjacency_list_graph Graph, typename Vector>
size_t TD_step(const Graph& g, std::vector<vertex_id_t<Graph>>& parents,
Vector& cur, std::vector<Vector>& next) {
  size_t scout_count = 0;
  size_t N = cur.size();
  scout_count = tbb::parallel_reduce(
    tbb::blocked_range(0ul, N), 0ul,
    [&](auto&& range, auto n) {
      int worker_index = tbb::this_task_arena::current_thread_index();
      for (auto&& i = range.begin(), e = range.end(); i != e; ++i) {
        auto u = cur[i];
        for (auto&& elt : g[u]) {
          auto v = target(g, elt);
          auto curr_val = parents[v];
          if (null_vertex_v<vertex_id_t<Graph>>() == curr_val) {
            //if u has not found a parent (not visited)
            if (nw::graph::cas(parents[v], curr_val, u)) {
              next[worker_index].push_back(v);
              n += g[v].size();
            }
          }
        }
      }
      return n;
  }, std::plus{});
  return scout_count;
}
template<class T>
inline void queue_to_bitmap(std::vector<T>& queue, nw::graph::AtomicBitVector<>& bitmap) {
  std::for_each(std::execution::par_unseq, queue.begin(), queue.end(), [&](auto&& u) { 
    bitmap.atomic_set(u); 
  });
}
template<typename Vector>
inline void bitmap_to_queue(nw::graph::AtomicBitVector<>& bitmap, std::vector<Vector>& lqueue) {
  tbb::parallel_for(bitmap.non_zeros(nw::graph::pow2(15)), [&](auto&& range) {
    int worker_index = tbb::this_task_arena::current_thread_index();
    for (auto&& i = range.begin(), e = range.end(); i != e; ++i) {
      lqueue[worker_index].push_back(*i);
    }
  });
}

/*
* Flush thread-local queue to global queue
*/
template<typename Vector>
void flush(std::vector<Vector>& lqueue, Vector& queue) {
  size_t size = 0;
  size_t n = lqueue.size();
  std::vector<size_t> size_array(n, 0);
  for (size_t i = 0; i < n; ++i) {
    //calculate the size of each thread-local frontier
    size_array[i] = size;
    //accumulate the total size of all thread-local frontiers
    size += lqueue[i].size();
  }
  //resize 'queue'
  queue.resize(size); 
  std::for_each(std::execution::par_unseq, counting_iterator(0ul), counting_iterator(n), [&](auto i) {
    //copy each thread-local queue to global queue based on their size offset
    auto begin = std::next(queue.begin(), size_array[i]);
    std::copy(std::execution::par_unseq, lqueue[i].begin(), lqueue[i].end(), begin);
    lqueue[i].clear();
  });
}

template <adjacency_list_graph OutGraph, adjacency_list_graph InGraph>
[[gnu::noinline]] auto bfs_v1(const OutGraph& out_graph, const InGraph& in_graph, vertex_id_t<OutGraph> root, int num_bins = 32,
                               int alpha = 15, int beta = 18) {

  using vertex_id_type = vertex_id_t<OutGraph>;
  using Vector = std::vector<vertex_id_type>;
  const std::size_t                                   n = nw::graph::pow2(nw::graph::ceil_log2(num_bins));
  const std::size_t                                   N = num_vertices(out_graph);
  const std::size_t                                   M = out_graph.to_be_indexed_.size();
  Vector queue;
  std::vector<Vector> lqueue(n);

  std::vector<vertex_id_type> parents(N);

  constexpr const auto null_vertex = null_vertex_v<vertex_id_type>();
  std::fill(std::execution::par_unseq, parents.begin(), parents.end(), null_vertex);

  std::uint64_t edges_to_check = M;
  std::uint64_t scout_count    = out_graph[root].size();

  parents[root] = root;
  queue.reserve(N);
  queue.push_back(root);

  nw::graph::AtomicBitVector front(N, false);
  nw::graph::AtomicBitVector curr(N);

  while (!queue.empty()) {
    if (scout_count > edges_to_check / alpha) {
      std::size_t                awake_count = queue.size();
      // Initialize the frontier bitmap from the frontier queues, and count the
      // number of non-zeros.
      queue_to_bitmap<vertex_id_type>(queue, curr);

      std::size_t old_awake_count = 0;
      do {
        old_awake_count = awake_count;
        std::swap(front, curr);
        curr.clear();

        awake_count = tbb::parallel_reduce(
            tbb::blocked_range(0ul, N), 0ul,
            [&](auto&& range, auto n) {
              for (auto&& u = range.begin(), e = range.end(); u != e; ++u) {
                if (null_vertex == parents[u]) {
                  for (auto&& elt : in_graph[u]) {
                    auto v = target(in_graph, elt);
                    if (front.get(v)) {
                      curr.atomic_set(u);
                      parents[u] = v;
                      ++n;
                      break;
                    }
                  }
                }
              }
              return n;
            }, std::plus{});
      } while ((awake_count >= old_awake_count) || (awake_count > N / beta));

      if (awake_count == 0) {
        return parents;
      }

      bitmap_to_queue(curr, lqueue);
      flush(lqueue, queue);

      scout_count = 1;
    } else {
      edges_to_check -= scout_count;
      scout_count = tbb::parallel_reduce(tbb::blocked_range(0ul, queue.size()), 0ul,
                                         [&](auto&&range, auto count) {
            int worker_index = tbb::this_task_arena::current_thread_index();
            for (auto&& i = range.begin(), e = range.end(); i != e; ++i) {
              auto u = queue[i];
              for (auto&& elt : out_graph[u]) {
                auto v = target(out_graph, elt);
                auto curr_val = parents[v];
                if (null_vertex == curr_val) {
                  if (nw::graph::cas(parents[v], curr_val, u)) {
                    lqueue[worker_index].push_back(v);
                    count += out_graph[v].size();
                  }
                }
              }
            }
            return count;
      }, std::plus{});
      flush(lqueue, queue);
    }
  }

  return parents;
}

template <adjacency_list_graph OutGraph, adjacency_list_graph InGraph>
[[gnu::noinline]] auto bfs_v2(const OutGraph& out_graph, const InGraph& in_graph, vertex_id_t<OutGraph> root, 
                              int num_bins = 32, int alpha = 15, int beta = 18) {

  using vertex_id_type = vertex_id_t<OutGraph>;
  using Vector = std::vector<vertex_id_type>;
  size_t n = nw::graph::pow2(nw::graph::ceil_log2(num_bins));
  size_t N = num_vertices(out_graph);
  Vector parents(N);
  constexpr const auto null_vertex = null_vertex_v<vertex_id_type>();
  std::fill(std::execution::par_unseq, parents.begin(), parents.end(), null_vertex);
  parents[root] = root;
  Vector frontier;
  std::vector<Vector> nextfrontier(n);
  frontier.reserve(N);
  frontier.push_back(root);
  nw::graph::AtomicBitVector front(N, false), cur(N);
  size_t edges_to_check = out_graph.to_be_indexed_.size();
  size_t scout_count = out_graph[root].size();
  
  while (!frontier.empty()) {
    if (scout_count > edges_to_check / alpha) {
      size_t old_awake_count;
      queue_to_bitmap<vertex_id_type>(frontier, front);
      size_t awake_count = frontier.size();
      do {
        old_awake_count = awake_count;
        awake_count = BU_step(in_graph, parents, front, cur);
        std::swap(front, cur);
      } while ((awake_count >= old_awake_count) || (awake_count > N / beta));
      bitmap_to_queue(front, nextfrontier);
      flush(nextfrontier, frontier);
      scout_count = 1;
    }
    else {
      edges_to_check -= scout_count;
      scout_count = TD_step(out_graph, parents, frontier, nextfrontier);
      flush(nextfrontier, frontier);
    }
  }//while

  return parents;
}

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_EXPERIMENTAL_BFS_HPP
