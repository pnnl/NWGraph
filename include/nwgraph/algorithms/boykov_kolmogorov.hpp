/**
 * @file boykov_kolmogorov.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 */

#ifndef BOYKOV_KOLMOGOROV_HPP
#define BOYKOV_KOLMOGOROV_HPP

#include <algorithm>
#include <ctime>
#include <queue>
#include <stack>
#include <tuple>
#include <utility>

#include "nwgraph/util/util.hpp"

namespace nw {
namespace graph {

const size_t INFINITE_D = 100000000;

enum class tree_mem : bool { source = false, term = true };

/**
 * Boykov-Kolmogorov max-flow 
 * @verbatim embed:rst:inline :cite:`boykov01:_exper_compar_min_cut_max`.@endverbatim
 *
 * @tparam Graph Type of the input graph.
 * @param A The input graph.
 * @param cap Vector of edge capacities.
 */
template <typename Graph>
std::tuple<double, std::vector<tree_mem>> bk_maxflow(const Graph& A, std::vector<double>& cap) {
  // std::clock_t start;
  // double grow = 0;
  // double augment = 0;
  // double adopt = 0;
  using vertex_id_type = vertex_id_t<Graph>;

  size_t         n_vtx    = A.size();
  vertex_id_type source   = n_vtx;
  vertex_id_type terminal = source + 1;
  double         max_flow = 0.0;
  double         tol      = 1e-12;
  size_t         gtime    = 0;

  std::queue<vertex_id_type>                active;
  std::queue<vertex_id_type>                orphans;
  std::vector<tree_mem>                     tree_id(n_vtx);
  std::vector<std::tuple<double*, double*>> trees(n_vtx);
  std::vector<vertex_id_type>               preds(n_vtx, null_vertex_v);
  std::vector<size_t>                       timestamp(n_vtx);
  std::vector<size_t>                       dist(n_vtx);

  double *       residual_cap, *back_residual_cap;
  vertex_id_type connect_s;
  vertex_id_type connect_t;
  double         min_cap;
  auto           G = A.begin();

  for (size_t i = 0; i < n_vtx; ++i) {
    if (cap[i] > 0) {
      tree_id[i] = tree_mem::source;
      preds[i]   = source;
    } else if (cap[i] < 0) {
      max_flow += cap[i];
      tree_id[i] = tree_mem::term;
      preds[i]   = terminal;
    } else {
      continue;
    }

    dist[i]      = 1;
    timestamp[i] = 0;
    active.push(i);
  }

  while (true) {
    bool found = false;
    // start = std::clock();
    // Grow
    while (!active.empty()) {
      auto p = active.front();

      // This node was disconnected, so make it inactive
      if (preds[p] == null_vertex_v) {
        active.pop();
        continue;
      }

      tree_mem t = tree_id[p];

      for (auto it = G[p].begin(); it != G[p].end(); ++it) {
        auto q = std::get<0>(*it);

        if (t == tree_mem::term) {
          // We are in the term tree, so we need to push flow "backwards", which
          // is the residual capacity.
          // residual_cap = std::get<2>(*it);
          residual_cap = &std::get<1>(A.get_back_edge(p, q));
          if (*residual_cap <= tol) {
            // No residual capacity
            continue;
          }
          back_residual_cap = &std::get<1>(*it);
          // potential endpoint in the source tree
          connect_s = q;
          connect_t = p;
        } else {    // s-tree
          residual_cap = &std::get<1>(*it);
          if (*residual_cap <= tol) {
            // no residual capacity
            continue;
          }
          // back_residual_cap = std::get<2>(*it);
          back_residual_cap = &std::get<1>(A.get_back_edge(p, q));
          connect_s         = p;
          // potential endpoint in the terminal tree
          connect_t = q;
        }

        // q is fresh (does not have a predecessor, not in the tree)
        if (preds[q] == null_vertex_v) {
          // Assign our tree to q
          tree_id[q] = t;
          // We are the predecessor now
          preds[q] = p;
          trees[q] = std::make_tuple(residual_cap, back_residual_cap);
          active.push(q);
          timestamp[q] = timestamp[p];
          dist[q]      = dist[p] + 1;
        }
        // Found a connection because q is in the other tree.
        else if (tree_id[q] != t) {
          found = true;
          break;
        }
        // q is in the same tree
        else if (timestamp[q] <= timestamp[p] && dist[q] > dist[p]) {
          preds[q]     = p;
          trees[q]     = std::make_tuple(residual_cap, back_residual_cap);
          timestamp[q] = timestamp[p];
          dist[q]      = dist[p] + 1;
        }
      }

      gtime++;

      if (found) break;

      active.pop();
    }

    if (!found) break;

    // grow += (std::clock() - start);
    // start = std::clock();
    // Augment
    min_cap = *residual_cap;
    // Search the t-tree for min capacity
    for (auto vtx = connect_t; vtx != terminal; vtx = preds[vtx]) {
      if (preds[vtx] == terminal) {
        min_cap = std::min(min_cap, -cap[vtx]);
      } else {
        auto edge = trees[vtx];
        min_cap   = std::min(min_cap, *std::get<0>(edge));
      }
    }
    // Search the s-tree for min capacity
    for (auto vtx = connect_s; vtx != source; vtx = preds[vtx]) {
      if (preds[vtx] == source) {
        min_cap = std::min(min_cap, cap[vtx]);
      } else {
        auto edge = trees[vtx];
        min_cap   = std::min(min_cap, *std::get<0>(edge));
      }
    }

    max_flow += min_cap;
    *residual_cap -= min_cap;
    *back_residual_cap += min_cap;
    // auto vtx = connect_t, next = preds[vtx]; vtx != terminal; vtx = next,
    // next = preds[vtx]
    // Update t-tree
    for (auto vtx = connect_t, next = preds[vtx]; vtx != terminal; vtx = next) {
      next = preds[vtx];
      if (next == terminal) {
        cap[vtx] += min_cap;
        if (cap[vtx] >= 0) {
          preds[vtx] = null_vertex_v;
          orphans.push(vtx);
        }
      } else {
        auto edge = trees[vtx];
        *std::get<0>(edge) -= min_cap;
        *std::get<1>(edge) += min_cap;
        if (*std::get<0>(edge) <= 0) {
          preds[vtx] = null_vertex_v;
          orphans.push(vtx);
        }
      }
    }
    // Update s-tree
    for (auto vtx = connect_s, next = preds[vtx]; vtx != source; vtx = next) {
      next = preds[vtx];
      if (next == source) {
        cap[vtx] -= min_cap;
        if (cap[vtx] <= 0) {
          preds[vtx] = null_vertex_v;
          orphans.push(vtx);
        }
      } else {
        auto edge = trees[vtx];
        *std::get<0>(edge) -= min_cap;
        *std::get<1>(edge) += min_cap;
        if (*std::get<0>(edge) <= 0) {
          preds[vtx] = null_vertex_v;
          orphans.push(vtx);
        }
      }
    }

    // augment += (std::clock() - start);
    // start = std::clock();
    // Adopt
    while (!orphans.empty()) {
      auto p = orphans.front();
      orphans.pop();

      tree_mem                     t = tree_id[p];
      vertex_id_type               min_pred{null_vertex_v};
      size_t                       d_min = INFINITE_D;
      std::tuple<double*, double*> min_edge;

      for (auto it = G[p].begin(); it != G[p].end(); ++it) {
        vertex_id_type q = std::get<0>(*it);
        vertex_id_type target;
        size_t         d = 0;

        if (tree_id[q] != t || preds[q] == p) continue;

        if (t == tree_mem::term) {
          residual_cap = &std::get<1>(*it);
          if (*residual_cap <= tol) continue;
          // back_residual_cap = std::get<2>(*it);
          back_residual_cap = &std::get<1>(A.get_back_edge(p, q));
          target            = terminal;
        } else {
          // residual_cap = std::get<2>(*it);
          residual_cap = &std::get<1>(A.get_back_edge(p, q));
          if (*residual_cap <= tol) continue;
          back_residual_cap = &std::get<1>(*it);
          target            = source;
        }

        for (auto vtx = q; vtx != target; vtx = preds[vtx]) {
          if (timestamp[vtx] == gtime) {
            d += dist[vtx];
            break;
          }

          d++;

          if (preds[vtx] == target) {
            timestamp[vtx] = gtime;
            dist[vtx]      = 1;
            break;
          } else if (preds[vtx] == null_vertex_v) {
            d = INFINITE_D;
            break;
          }
        }

        if (d < INFINITE_D) {
          if (d < d_min) {
            min_pred = q;
            min_edge = std::make_tuple(residual_cap, back_residual_cap);
            d_min    = d;
          }

          for (auto vtx = q; vtx != target; vtx = preds[vtx]) {
            if (timestamp[vtx] == gtime) break;

            timestamp[vtx] = gtime;
            dist[vtx]      = d--;
          }
        }
      }

      // can't find new parent
      if (min_pred == null_vertex_v) {
        timestamp[p] = 0;

        for (auto it = G[p].begin(); it != G[p].end(); ++it) {
          auto q = std::get<0>(*it);

          if (tree_id[q] != t) continue;

          if (tree_id[q] == tree_mem::source && std::get<1>(*it) > tol) {
            active.push(q);
          } else if (tree_id[q] == tree_mem::term && std::get<1>(A.get_back_edge(p, q)) > tol) {
            active.push(q);
          }

          if (preds[q] == p) {
            orphans.push(q);
            preds[q] = null_vertex_v;
          }
        }
      }
      // Found parent
      else {
        preds[p]     = min_pred;
        trees[p]     = min_edge;
        timestamp[p] = gtime;
        dist[p]      = d_min + 1;
      }
    }
    // adopt += (std::clock() - start);
  }
  /*std::cout << "grow time: " << grow / (double) CLOCKS_PER_SEC <<
    " augment time: " << augment / (double) CLOCKS_PER_SEC <<
    " adopt time: " << adopt / (double) CLOCKS_PER_SEC << std::endl;*/
  // std::cout << augment_ct << " aguments" << std::endl;
  // std::cout << path_length*(1.0)/augment_ct << " avg path length" <<
  // std::endl;
  for (size_t i = 0; i < n_vtx; ++i) {
    if (preds[i] == null_vertex_v) {
      tree_id[i] = tree_mem::term;
    }
  }
  return std::make_tuple(max_flow, tree_id);
}

}    // namespace graph
}    // namespace nw

#endif    // BOYKOV_KOLMOGOROV_HPP
