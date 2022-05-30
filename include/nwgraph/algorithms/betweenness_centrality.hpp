/**
 * @file betweenness_centrality.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *   Kevin Deweese
 *   Tony Liu
 *
 */

#ifndef BETWEENNESS_CENTRALITY_HPP
#define BETWEENNESS_CENTRALITY_HPP

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/adaptors/worklist.hpp"
#include "nwgraph/util/AtomicBitVector.hpp"
#include "nwgraph/util/atomic.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/util/util.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"

#include <algorithm>

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#include <dpstd/numeric>
#else
#include <execution>
#endif

#include <forward_list>
#include <future>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for_each.h>

namespace nw {
namespace graph {

template <class score_t, class accum_t, adjacency_list_graph Graph>
bool BCVerifier(const Graph& g, std::vector<typename graph_traits<Graph>::vertex_id_type>& trial_sources,
                std::vector<score_t>& scores_to_test, bool normalize = true) {
  using vertex_id_type = typename graph_traits<Graph>::vertex_id_type;

  std::vector<score_t> scores(num_vertices(g), 0);
  for (auto& source : trial_sources) {
    std::vector<int> depths(num_vertices(g), -1);
    depths[source] = 0;
    std::vector<accum_t> path_counts(num_vertices(g) + 1, 0);
    path_counts[source] = 1;
    std::vector<vertex_id_type> to_visit;
    to_visit.reserve(num_vertices(g));
    to_visit.push_back(source);
    for (auto it = to_visit.begin(); it != to_visit.end(); it++) {
      vertex_id_type u = *it;
      for (auto edge : g[u]) {
        vertex_id_type v = target(g, edge);
        if (depths[v] == -1) {
          depths[v] = depths[u] + 1;
          to_visit.push_back(v);
        }
        if (depths[v] == depths[u] + 1) {
          path_counts[v] += path_counts[u];
        }
      }
    }

    std::vector<std::vector<vertex_id_type>> verts_at_depth;
    for (size_t i = 0; i < num_vertices(g); ++i) {
      if (depths[i] != -1) {
        if (depths[i] >= static_cast<int>(verts_at_depth.size())) {
          verts_at_depth.resize(depths[i] + 1);
        }
        verts_at_depth[depths[i]].push_back(i);
      }
    }

    std::vector<score_t> deltas(num_vertices(g), 0);
    for (int depth = verts_at_depth.size() - 1; depth >= 0; depth--) {
      for (vertex_id_type u : verts_at_depth[depth]) {
        for (auto edge : g[u]) {
          vertex_id_type v = target(g, edge);
          if (depths[v] == depths[u] + 1) {
            deltas[u] += static_cast<double>(path_counts[u]) / static_cast<double>(path_counts[v]) * (1 + deltas[v]);
          }
        }
        scores[u] += deltas[u];
      }
    }
  }
  if (normalize){
    score_t biggest_score = *std::max_element(scores.begin(), scores.end());
    for (size_t i = 0; i < num_vertices(g); ++i) {
      scores[i] = scores[i] / biggest_score;
    }
  }

  bool all_ok = true;

  for (size_t i = 0; i < scores.size(); ++i) {
    accum_t delta = abs(scores_to_test[i] - scores[i]);
    if (delta > 1e-6) {
      std::cout << i << ": " << scores[i] << " != " << scores_to_test[i] << " " << scores[i] - scores_to_test[i] << std::endl;
      all_ok = false;
    }
  }

  return all_ok;
}

/**
 * Compute exact betweenness centrality using Brandes algorithm 
 * @verbatim embed:rst:inline :cite:`brandes_bc`.@endverbatim
 *
 * @tparam Graph Type of the graph.  Must meet requirements of adjacency_list_graph concept.
 * @tparam score_t Type of the centrality scores computed for each vertex.
 * @tparam accum_t Type of path counts.
 * @param G Input graph.
 * @param normalize Flag indicating whether to normalize centrality scores relative to largest score.
 * @return Vector of centrality for each vertex.
 */
template <adjacency_list_graph Graph, typename score_t = float, typename accum_t = size_t>
std::vector<score_t> brandes_bc(const Graph& G, bool normalize = true) {
  using vertex_id_type = typename Graph::vertex_id_type;

  size_t               n_vtx = num_vertices(G);
  std::vector<score_t> centrality(n_vtx, 0);

  std::stack<vertex_id_type> S;
  std::queue<vertex_id_type> Q;
  std::vector<accum_t>       path_counts(n_vtx);
  std::vector<score_t>       d(n_vtx);

  for (vertex_id_type s = 0; s < n_vtx; ++s) {
    path_counts.assign(n_vtx, 0);
    d.assign(n_vtx, -1);
    std::vector<std::list<size_t>> P(n_vtx);

    path_counts[s] = 1;
    d[s]           = 0;
    Q.push(s);

    while (!Q.empty()) {
      auto v = Q.front();
      Q.pop();
      S.push(v);
      for (auto inner = G[v].begin(); inner != G[v].end(); ++inner) {
        auto w = target(G, *inner);
        //auto w = std::get<0>(*inner);
        if (d[w] < 0) {
          Q.push(w);
          d[w] = d[v] + 1;
        }
        if (d[w] == (d[v] + 1)) {
          path_counts[w] += path_counts[v];
          P[w].push_back(v);
        }
      }
    }

    std::vector<score_t> delta(n_vtx, 0);
    while (!S.empty()) {
      auto w = S.top();
      S.pop();
      for (auto it = P[w].begin(); it != P[w].end(); ++it) {
        delta[*it] += static_cast<score_t>(path_counts[*it]) / static_cast<score_t>(path_counts[w]) * (1 + delta[w]);
      }
      if (w != s) {
        centrality[w] += delta[w];
      }
    }
  }
  if (normalize) {
    score_t largest = *std::max_element(centrality.begin(), centrality.end());
    std::transform(centrality.begin(), centrality.end(), centrality.begin(), [&](auto& val) { return val /= largest; });
  }
  return centrality;
}

/**
 * Parallel approximate betweenness centrality using Brandes algorithm @verbatim embed:rst:inline :cite:`brandes_bc`.@endverbatim
 * Rather than using all vertices in the graph to compute paths, the algorithm uses a
 * specified set of root nodes.  Parallelization is effected through C++ standard library
 * execution policies.
 *
 * @tparam Graph Type of the graph.  Must meet requirements of adjacency_list_graph concept.
 * @tparam score_t Type of the centrality scores computed for each vertex.
 * @tparam accum_t Type of path counts.
 * @tparam OuterExecutionPolicy Parallel execution policy type for outer loop of algorithm.  Default: std::execution::parallel_unsequenced_policy
 * @tparam InnerExecutionPolicy Parallel execution policy type for inner loop of algorithm.  Default: std::execution::parallel_unsequenced_policy
 * @param G Input graph.
 * @param normalize Flag indicating whether to normalize centrality scores relative to largest score.
 * @param sources Vector of starting sources.
 * @param outer_policy Outer loop parallel execution policy.
 * @param inner_policy Inner loop parallel execution policy.
 * @param threads Number of threads being used in computation.  Used to compute number of bins in computation.
 * @return Vector of centrality for each vertex.
 */
template <class score_t, class accum_t, adjacency_list_graph Graph, class OuterExecutionPolicy = std::execution::parallel_unsequenced_policy,
          class InnerExecutionPolicy = std::execution::parallel_unsequenced_policy>
auto brandes_bc(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& sources, int threads,
                OuterExecutionPolicy&& outer_policy = {}, InnerExecutionPolicy&& inner_policy = {}, bool normalize = true) {
  using vertex_id_type = typename Graph::vertex_id_type;

  vertex_id_type       N     = num_vertices(graph);
  size_t               M     = graph.to_be_indexed_.size();
  auto&&               edges = std::get<0>(*(graph[0]).begin());
  std::vector<score_t> bc(N);

  const vertex_id_type num_bins = nw::graph::pow2(nw::graph::ceil_log2(threads));
  const vertex_id_type bin_mask = num_bins - 1;

  std::vector<std::future<void>> futures(sources.size());
  for (size_t s_idx = 0; s_idx < sources.size(); ++s_idx) {
    futures[s_idx] = std::async(
        std::launch::async,
        [&](vertex_id_type root) {
          std::vector<vertex_id_type> levels(N);
          nw::graph::AtomicBitVector  succ(M);

          // Initialize the levels to infinity.
          std::fill(outer_policy, levels.begin(), levels.end(), std::numeric_limits<vertex_id_type>::max());

          std::vector<accum_t>                                             path_counts(N);
          std::vector<tbb::concurrent_vector<vertex_id_type>>              q1(num_bins);
          std::vector<tbb::concurrent_vector<vertex_id_type>>              q2(num_bins);
          std::vector<std::vector<tbb::concurrent_vector<vertex_id_type>>> retired;

          vertex_id_type lvl = 0;

          path_counts[root] = 1;
          q1[0].push_back(root);
          levels[root] = lvl++;

          bool done = false;
          while (!done) {
            std::for_each(outer_policy, q1.begin(), q1.end(), [&](auto&& q) {
              std::for_each(inner_policy, q.begin(), q.end(), [&](auto&& u) {
                for (auto&& elt : graph[u]) {
                  auto&&   v        = target(graph, elt);
                  auto&& infinity = std::numeric_limits<vertex_id_type>::max();
                  auto&& lvl_v    = nw::graph::acquire(levels[v]);

                  // If this is our first encounter with this node, or
                  // it's on the right level, then propagate the counts
                  // from u to v, and mark the edge from u to v as used.
                  if (lvl_v == infinity || lvl_v == lvl) {
                    nw::graph::fetch_add(path_counts[v], nw::graph::acquire(path_counts[u]));
                    succ.atomic_set(&v - &edges);    // edge(w,v) : P[w][v]
                  }

                  // We need to add v to the frontier exactly once the
                  // first time we encounter it, so we race to set its
                  // level and if we win that race we can be the one to
                  // add it.
                  if (lvl_v == infinity && nw::graph::cas(levels[v], infinity, lvl)) {
                    q2[u & bin_mask].push_back(v);
                  }
                }
              });
            });

            done = true;
            for (size_t i = 0; i < num_bins; ++i) {
              if (q2[i].size() != 0) {
                done = false;
                break;
              }
            }

            retired.emplace_back(num_bins);
            std::swap(q1, retired.back());
            std::swap(q1, q2);

            ++lvl;
          }

          std::vector<score_t> deltas(N);

          std::for_each(retired.rbegin(), retired.rend(), [&](auto&& vvv) {
            std::for_each(outer_policy, vvv.begin(), vvv.end(), [&](auto&& vv) {
              std::for_each(inner_policy, vv.begin(), vv.end(), [&](auto&& u) {
                score_t delta = 0;
                for (auto&& elt : graph[u]) {
                  auto&& v = target(graph, elt);
                  if (succ.get(&v - &edges)) {
                    delta += path_counts[u] / path_counts[v] * (1.0f + deltas[v]);
                  }
                }
                nw::graph::fetch_add(bc[u], deltas[u] = delta);
              });
            });
          });
        },
        sources[s_idx]);
  }

  for (auto&& f : futures) {
    f.wait();
  }

  if (normalize) {
    auto max = std::reduce(outer_policy, bc.begin(), bc.end(), 0.0f, nw::graph::max{});
    std::for_each(outer_policy, bc.begin(), bc.end(), [&](auto&& j) { j /= max; });
  }
  return bc;
}

/**
 * Parallel exact betweenness centrality using Brandes algorithm @verbatim embed:rst:inline :cite:`brandes_bc`.@endverbatim
 * Rather than using a specified set of root nodes to compute paths, 
 * the algorithm uses all vertices in the graph.  Parallelization is effected through C++ standard library
 * execution policies.
 *
 * @tparam Graph Type of the graph.  Must meet requirements of adjacency_list_graph concept.
 * @tparam score_t Type of the centrality scores computed for each vertex.
 * @tparam accum_t Type of path counts.
 * @tparam OuterExecutionPolicy Parallel execution policy type for outer loop of algorithm.  Default: std::execution::parallel_unsequenced_policy
 * @tparam InnerExecutionPolicy Parallel execution policy type for inner loop of algorithm.  Default: std::execution::parallel_unsequenced_policy
 * @param G Input graph.
 * @param normalize Flag indicating whether to normalize centrality scores relative to largest score.
 * @param outer_policy Outer loop parallel execution policy.
 * @param inner_policy Inner loop parallel execution policy.
 * @param threads Number of threads being used in computation.  Used to compute number of bins in computation.
 * @return Vector of centrality for each vertex.
 */
template <class score_t, class accum_t, adjacency_list_graph Graph, class OuterExecutionPolicy = std::execution::parallel_unsequenced_policy,
          class InnerExecutionPolicy = std::execution::parallel_unsequenced_policy>
auto exact_brandes_bc(const Graph& graph, int threads,
                OuterExecutionPolicy&& outer_policy = {}, InnerExecutionPolicy&& inner_policy = {}, bool normalize = true) {
  using vertex_id_type = typename Graph::vertex_id_type;
  vertex_id_type       N     = num_vertices(graph);
  std::vector<vertex_id_type> sources(N);
  auto&& r = vertex_range<Graph>(N);
  std::for_each(outer_policy, r.begin(), r.end(), [&](const auto& i) {
    sources[i] = i;
  });
  return brandes_bc<score_t, 
  accum_t, 
  Graph, 
  OuterExecutionPolicy, 
  InnerExecutionPolicy>(graph, sources, threads, std::forward<OuterExecutionPolicy>(outer_policy), std::forward<InnerExecutionPolicy>(inner_policy), normalize);
}

}    // namespace graph
}    // namespace nw
#endif    // BETWEENNESS_CENTRALITY_HPP
