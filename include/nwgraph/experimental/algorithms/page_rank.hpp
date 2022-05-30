/**
 * @file page_rank.hpp
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

#ifndef NW_GRAPH_PAGE_RANK_EXPERIMENTAL_HPP
#define NW_GRAPH_PAGE_RANK_EXPERIMENTAL_HPP

#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/algorithms/page_rank.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/adaptors/vertex_range.hpp"

namespace nw {
namespace graph {

#if 0
//****************************************************************************
template<typename GraphT, typename RealT = double>
void page_rank_range_for(GraphT& graph, std::vector<RealT>& page_rank, RealT damping_factor = 0.85, RealT threshold = 1.e-4,
                         size_t max_iters = std::numeric_limits<unsigned int>::max()) {
  /// @todo assert page_rank.size() == graph.size();

  RealT const scaled_teleport((1. - damping_factor) / (RealT)graph.size());

  // degree-normalize the rows of the graph and scale by damping factor
  std::vector<RealT> odegree(out_degree<GraphT, RealT>(graph));

  edge_list<directed, RealT> edges(0);
  edges.open_for_push_back();
  for (auto [i, j, v] : make_edge_range<0>(graph)) {
    edges.push_back(i, j, v * (damping_factor / odegree[i]));
    //v *= (damping_factor/out_degree[i]);
  }
  edges.close_for_push_back();
  adjacency<0, RealT> M(edges);

  // initialize page rank to 1/N
  std::vector<RealT> old_rank(graph.size(), 1. / (RealT)graph.size());

  for (size_t iter_num = 0; iter_num < max_iters; ++iter_num) {
    std::fill(page_rank.begin(), page_rank.end(), 0.0);

    // compute new rank (transpose the graph): PR_i = M' * PR_i-1
    for (auto&& [i, j, v] : make_edge_range<0>(M)) {
      page_rank[i] += v * old_rank[j];
    }

    // add scaled teleport term: (1 - damping_factor)/N
    for (auto& rank_val : page_rank) {
      rank_val += scaled_teleport;
    }

    // Test for convergence, compute squared error
    RealT squared_error(0.);
    for (size_t idx = 0; idx < page_rank.size(); ++idx) {
      RealT tmp = page_rank[idx] - old_rank[idx];
      squared_error += tmp * tmp;
    }

    //std::cout << "Iteration " << iter_num << ": sq_err = "
    //          << squared_error/((RealT)graph.size()) << std::endl;

    if (squared_error / ((RealT)graph.size()) < threshold) break;

    old_rank.swap(page_rank);
  }
}

#endif

template <adjacency_list_graph Graph, typename Real = double>
void page_rank_vc(const Graph& graph, std::vector<Real>& page_rank, const Real damping_factor = 0.85, const Real threshold = 1.e-4,
                  const size_t max_iters = std::numeric_limits<unsigned int>::max()) {
  using vertex_id_type = typename Graph::vertex_id_type;

  const Real init_score = 1.0 / page_rank.size();
  const Real base_score = (1.0 - damping_factor) / page_rank.size();

  std::fill(page_rank.begin(), page_rank.end(), init_score);
  std::vector<vertex_id_type> degrees(page_rank.size());
  //  for (auto&& [i, j, v] : make_edge_range<0>(graph)) {
  for (auto&& [i, j] : edge_range(graph)) {
    ++degrees[j];
  }

  std::vector<Real> outgoing_contrib(page_rank.size());

  for (size_t iter = 0; iter < max_iters; ++iter) {
    double error = 0;

    std::transform(page_rank.begin(), page_rank.end(), degrees.begin(), outgoing_contrib.begin(), [&](auto&& x, auto&& y) {
      ;
      return x / (y + 0);
    });

    size_t len  = graph.indices_.size() - 1;
    auto   ptrs = graph.indices_;
    auto   idxs = std::get<0>(graph.to_be_indexed_);

    for (size_t i = 0; i < len; ++i) {
      Real z = 0;
      for (size_t j = ptrs[i]; j < ptrs[i + 1]; ++j) {
        z += outgoing_contrib[idxs[j]];
      }
      auto old_rank = page_rank[i];
      page_rank[i]  = base_score + damping_factor * z;
      error += fabs(page_rank[i] - old_rank);
    }

    std::cout << iter << " " << error << std::endl;
    if (error < threshold) break;
  }
}

template <adjacency_list_graph Graph, typename Real = double>
void page_rank_v1(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees, std::vector<Real>& page_rank,
                  const Real damping_factor = 0.85, const Real threshold = 1.e-4,
                  const size_t max_iters = std::numeric_limits<unsigned int>::max()) {
  using vertex_id_type = typename Graph::vertex_id_type;

  const Real init_score = 1.0 / page_rank.size();
  const Real base_score = (1.0 - damping_factor) / page_rank.size();

  std::fill(page_rank.begin(), page_rank.end(), init_score);

  std::vector<Real> outgoing_contrib(page_rank.size());

  for (size_t iter = 0; iter < max_iters; ++iter) {

    double error = 0;

    std::transform(page_rank.begin(), page_rank.end(), degrees.begin(), outgoing_contrib.begin(), [&](auto&& x, auto&& y) {
      ;
      return x / (y + 0);
    });


    for (vertex_id_type i = 0; i < page_rank.size(); ++i) {
      Real z = 0;
      for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
        z += outgoing_contrib[std::get<0>(*j)];
      }
      auto old_rank = page_rank[i];
      page_rank[i]  = base_score + damping_factor * z;
      error += fabs(page_rank[i] - old_rank);
    }

    //std::cout << iter << " " << error << std::endl;

    if (error < threshold) break;
  }
}

template <adjacency_list_graph Graph, typename Real = double>
void page_rank_v2(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees, std::vector<Real>& page_rank,
                  const Real damping_factor = 0.85, const Real threshold = 1.e-4,
                  const size_t max_iters = std::numeric_limits<unsigned int>::max()) {
  using vertex_id_type = typename Graph::vertex_id_type;

  const Real init_score = 1.0 / page_rank.size();
  const Real base_score = (1.0 - damping_factor) / page_rank.size();

  std::fill(page_rank.begin(), page_rank.end(), init_score);

  std::vector<Real> outgoing_contrib(page_rank.size());

  for (size_t iter = 0; iter < max_iters; ++iter) {

    double error = 0;


    for (vertex_id_type i = 0; i < page_rank.size(); ++i) {
      Real z = 0;
      for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
        z += outgoing_contrib[std::get<0>(*j)];
      }
      auto old_rank = page_rank[i];
      page_rank[i]  = base_score + damping_factor * z;
      error += fabs(page_rank[i] - old_rank);
      outgoing_contrib[i] = page_rank[i] / (Real)degrees[i];    // Gauss-Seidel
    }
    std::cout << iter << " " << error << std::endl;
    if (error < threshold) break;
  }
}

template <adjacency_list_graph Graph, typename Real = double>
void page_rank_v4(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees, std::vector<Real>& page_rank,
                  const Real damping_factor = 0.85, const Real threshold = 1.e-4,
                  const size_t max_iters = std::numeric_limits<unsigned int>::max(), size_t num_threads = 1) {
  using vertex_id_type = typename Graph::vertex_id_type;

  const Real init_score = 1.0 / page_rank.size();
  const Real base_score = (1.0 - damping_factor) / page_rank.size();

  std::fill(std::execution::par_unseq, page_rank.begin(), page_rank.end(), init_score);

  std::vector<Real>                outgoing_contrib(page_rank.size());
  std::vector<std::future<double>> futures(num_threads);

  for (size_t iter = 0; iter < max_iters; ++iter) {

    std::transform(std::execution::par_unseq, page_rank.begin(), page_rank.end(), degrees.begin(), outgoing_contrib.begin(),
                   [&](auto&& x, auto&& y) {
                     ;
                     return x / (y + 0);
                   });


    for (size_t thread = 0; thread < num_threads; ++thread) {
      futures[thread] = std::async(
          std::launch::async,
          [&](size_t thread) {
            double error = 0;

            for (vertex_id_type i = thread; i < page_rank.size(); i += num_threads) {
              Real z = 0;
              for (auto j = graph[i].begin(); j != graph[i].end(); ++j) {
                z += outgoing_contrib[std::get<0>(*j)];
                // for (auto&& [j] : G[i]) {
                // z += outgoing_contrib[j];
              }
              auto old_rank = page_rank[i];
              page_rank[i]  = base_score + damping_factor * z;
              error += fabs(page_rank[i] - old_rank);
            }
            return error;
          },
          thread);
    }
    double error = 0;
    for (size_t i = 0; i < num_threads; ++i) {
      error += futures[i].get();
    }

    std::cout << iter << " " << error << std::endl;
    if (error < threshold) break;
  }
}

template <adjacency_list_graph Graph, typename Real = double>
[[gnu::noinline]] void page_rank_v6(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees,
                                    std::vector<Real>& page_rank, const Real damping_factor = 0.85, const Real threshold = 1.e-4,
                                    const size_t max_iters = std::numeric_limits<unsigned int>::max(), size_t num_threads = 1) {

  const Real init_score = 1.0 / page_rank.size();
  const Real base_score = (1.0 - damping_factor) / page_rank.size();

  std::fill(std::execution::par_unseq, page_rank.begin(), page_rank.end(), init_score);

  auto per = edge_range(graph);

  std::vector<Real> outgoing_contrib(page_rank.size());
  std::vector<Real> old_rank(page_rank.size());
  std::vector<Real> z(page_rank.size());

  for (size_t iter = 0; iter < max_iters; ++iter) {

    std::transform(std::execution::par_unseq, page_rank.begin(), page_rank.end(), degrees.begin(), outgoing_contrib.begin(),
                   [&](auto&& x, auto&& y) { return x / (y + 0); });

#if 0
    double error = tbb::parallel_reduce(per, [&](auto&& r, Real init) {
      Real z = std::transform_reduce(r.begin(), r.end(), Real(0.0), std::plus<Real>(),
                                     [&] (auto&& j) { }
                                     }, std::plus<Real>());

      counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(page_rank.size()), Real(0.0), std::plus<Real>(),

      [&](auto i) {
        Real z        = std::transform_reduce(std::execution::seq, G[i].begin(), G[i].end(), Real(0.0), std::plus<Real>(),
                                              [&](auto&& j) { return outgoing_contrib[std::get<0>(j)]; });
        auto old_rank = page_rank[i];
        page_rank[i]  = base_score + damping_factor * z;
        return fabs(page_rank[i] - old_rank);
      });
#else

    std::fill(std::execution::par_unseq, z.begin(), z.end(), Real(0));

    tbb::parallel_for(per, [&](auto&& x) {
      std::for_each(x.begin(), x.end(), [&](auto&& elt) {
        auto&& [i, j] = elt;
        z[i] += outgoing_contrib[j];
      });
    });

    std::swap(page_rank, old_rank);

    std::transform(std::execution::par_unseq, z.begin(), z.end(), page_rank.begin(), [&](auto&& a) { return base_score + damping_factor * a; });
    double error = std::transform_reduce(std::execution::par_unseq, page_rank.begin(), page_rank.end(), old_rank.begin(), 0.0,
                                         std::plus<Real>(), [&](auto&& a, auto&& b) { return fabs(a - b); });
#endif
    std::cout << iter << " " << error << std::endl;
    if (error < threshold) break;
  }
}

template <adjacency_list_graph Graph, typename Real = double>
void page_rank_v7(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees, std::vector<Real>& page_rank,
                  const Real damping_factor = 0.85, const Real threshold = 1.e-4,
                  const size_t max_iters = std::numeric_limits<unsigned int>::max(), size_t num_threads = 1) {
  using vertex_id_type = typename Graph::vertex_id_type;

  const Real init_score = 1.0 / page_rank.size();
  const Real base_score = (1.0 - damping_factor) / page_rank.size();

  {
    nw::util::life_timer _("fill");

    std::fill(std::execution::par_unseq, page_rank.begin(), page_rank.end(), init_score);
  }
  std::vector<Real> outgoing_contrib(page_rank.size());

  {
    nw::util::life_timer _("iters");
    for (size_t iter = 0; iter < max_iters; ++iter) {

      std::transform(std::execution::par, page_rank.begin(), page_rank.end(), degrees.begin(), outgoing_contrib.begin(),
                     [&](auto&& x, auto&& y) {
                       ;
                       return x / (y + 0);
                     });


      double error = std::transform_reduce(std::execution::par_unseq, counting_iterator<vertex_id_type>(0),
                                           counting_iterator<vertex_id_type>(page_rank.size()), Real(0.0), std::plus<Real>(),

                                           [&](auto i) {
                                             Real z = tbb::parallel_reduce(
                                                 graph[i], Real(0.0),
                                                 [&](auto&& j, const Real& foo) {
                                                   return foo + std::transform_reduce(
                                                                    std::execution::seq, j.begin(), j.end(), Real(0.0), std::plus<Real>(),
                                                                    [&](auto&& a) { return outgoing_contrib[std::get<0>(a)]; });
                                                 },
                                                 std::plus<Real>());
                                             auto old_rank = page_rank[i];
                                             page_rank[i]  = base_score + damping_factor * z;
                                             return fabs(page_rank[i] - old_rank);
                                           });
      std::cout << iter << " " << error << std::endl;
      if (error < threshold) break;
    }
  }
}

template <adjacency_list_graph Graph, typename Real = double>
void page_rank_v8(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees, std::vector<Real>& page_rank,
                  const Real damping_factor = 0.85, const Real threshold = 1.e-4,
                  const size_t max_iters = std::numeric_limits<unsigned int>::max(), size_t num_threads = 1) {
  using vertex_id_type = typename Graph::vertex_id_type;

  const Real init_score = 1.0 / page_rank.size();
  const Real base_score = (1.0 - damping_factor) / page_rank.size();

  std::fill(std::execution::par_unseq, page_rank.begin(), page_rank.end(), init_score);

  std::vector<Real> outgoing_contrib(page_rank.size());

  for (size_t iter = 0; iter < max_iters; ++iter) {

    std::transform(std::execution::par_unseq, page_rank.begin(), page_rank.end(), degrees.begin(), outgoing_contrib.begin(),
                   [&](auto&& x, auto&& y) {
                     ;
                     return x / (y + 0);
                   });


    double error =
        std::transform_reduce(std::execution::par_unseq, counting_iterator<vertex_id_type>(0),
                              counting_iterator<vertex_id_type>(page_rank.size()), Real(0.0), std::plus<Real>(),

                              [&](auto i) {
                                Real z        = std::transform_reduce(std::execution::par_unseq, graph[i].begin(), graph[i].end(), Real(0.0),
                                                               std::plus<Real>(), [&](auto&& j) { return outgoing_contrib[std::get<0>(j)]; });
                                auto old_rank = page_rank[i];
                                page_rank[i]  = base_score + damping_factor * z;
                                return fabs(page_rank[i] - old_rank);
                              });
    std::cout << iter << " " << error << std::endl;
    if (error < threshold) break;
  }
}

template <adjacency_list_graph Graph, typename Real>
[[gnu::noinline]] void page_rank_v9(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees,
                                    std::vector<Real>& page_rank, Real damping_factor, Real threshold, size_t max_iters, size_t num_threads) {
  using vertex_id_type = typename Graph::vertex_id_type;

  std::size_t N          = page_rank.size();
  Real        init_score = 1.0 / N;
  Real        base_score = (1.0 - damping_factor) / N;

  std::fill(std::execution::par_unseq, page_rank.begin(), page_rank.end(), init_score);
  std::unique_ptr<Real[]> outgoing_contrib(new Real[N]);

  pagerank::trace("iter", "error", "time", "outgoing");

  for (size_t iter = 0; iter < max_iters; ++iter) {
    auto&& [outgoing] = pagerank::time_op([&] {
      std::transform(std::execution::par_unseq, page_rank.begin(), page_rank.end(), degrees.begin(), &outgoing_contrib[0],
                     [&](auto&& x, auto&& y) { return x / y; });
    });


    auto&& [time, error] = pagerank::time_op([&] {
      return std::transform_reduce(std::execution::par_unseq, counting_iterator<vertex_id_type>(0), counting_iterator<vertex_id_type>(N),
                                   Real(0.0), std::plus{}, [&](auto&& i) {
                                     Real z = 0.0;
                                     for (auto&& j : graph[i]) {
                                       z += outgoing_contrib[std::get<0>(j)];
                                     }
                                     auto old_rank = page_rank[i];
                                     page_rank[i]  = base_score + damping_factor * z;
                                     return fabs(page_rank[i] - old_rank);
                                   });
    });

    pagerank::trace(iter, error, time, outgoing);

    if (error < threshold) {
      return;
    }
  }
}

template <adjacency_list_graph Graph, typename Real>
[[gnu::noinline]] void page_rank_v10(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees,
                                     std::vector<Real>& page_rank, Real damping_factor, Real threshold, size_t max_iters, size_t num_threads) {
  std::size_t N          = graph.size();
  Real        init_score = 1.0 / N;
  Real        base_score = (1.0 - damping_factor) / N;

  {
    nw::util::life_timer _("init page rank");

    // Initialize the page rank.
    tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
      for (auto i = r.begin(), e = r.end(); i != e; ++i) {
        page_rank[i] = init_score;
      }
    });
  }

  std::unique_ptr<Real[]> outgoing_contrib(new Real[N]);

  pagerank::trace("iter", "error", "time", "outgoing");

  for (size_t iter = 0; iter < max_iters; ++iter) {
    auto&& [outgoing] = pagerank::time_op([&] {
      tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
        for (auto i = r.begin(), e = r.end(); i != e; ++i) {
          outgoing_contrib[i] = page_rank[i] / degrees[i];
        }
      });
    });

    auto&& [time, error] = pagerank::time_op([&] {
      return tbb::parallel_reduce(
          tbb::blocked_range(0ul, N), 0.0,
          [&](auto&& r, auto partial_sum) {
            for (size_t i = r.begin(), e = r.end(); i != e; ++i) {
              Real z = 0.0;
              for (auto&& j : graph[i]) {
                z += outgoing_contrib[std::get<0>(j)];
              }
              auto old_rank = page_rank[i];
              page_rank[i]  = base_score + damping_factor * z;
              partial_sum += fabs(page_rank[i] - old_rank);
            }
            return partial_sum;
          },
          std::plus{});
    });

    pagerank::trace(iter, error, time, outgoing);

    if (error < threshold) {
      return;
    }
  }
}

template <adjacency_list_graph Graph, typename Real>
[[gnu::noinline]] void page_rank_v12(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees,
                                     std::vector<Real>& page_rank, Real damping_factor, Real threshold, size_t max_iters, size_t num_threads) {
  std::size_t N          = graph.size();
  Real        init_score = 1.0 / N;
  Real        base_score = (1.0 - damping_factor) / N;

  {
    nw::util::life_timer _("init page rank");

    // Initialize the page rank.
    tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
      for (auto i = r.begin(), e = r.end(); i != e; ++i) {
        page_rank[i] = init_score;
      }
    });
  }

  std::unique_ptr<Real[]> outgoing_contrib(new Real[N]);

  pagerank::trace("iter", "error", "time", "outgoing");

  tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      outgoing_contrib[i] = page_rank[i] / degrees[i];
    }
  });

  for (size_t iter = 0; iter < max_iters; ++iter) {

    auto&& [time, error] = pagerank::time_op([&] {
      return tbb::parallel_reduce(
          tbb::blocked_range(0ul, N), 0.0,
          [&](auto&& r, auto partial_sum) {
            for (size_t i = r.begin(), e = r.end(); i != e; ++i) {
              Real z = 0.0;
              for (auto&& j : graph[i]) {
                if (outgoing_contrib[std::get<0>(j)] > threshold) {
                  z += outgoing_contrib[std::get<0>(j)];
                }
              }
              auto old_rank = page_rank[i];
              page_rank[i]  = base_score + damping_factor * z;
              partial_sum += fabs(page_rank[i] - old_rank);
              outgoing_contrib[i] = page_rank[i] / (Real)degrees[i];
            }
            return partial_sum;
          },
          std::plus{});
    });

    pagerank::trace(iter, error, time, 0);

    if (error < threshold) {
      return;
    }
  }
}

template <adjacency_list_graph Graph, typename Real>
[[gnu::noinline]] void page_rank_v3(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees,
                                    std::vector<Real>& page_rank, Real damping_factor, Real threshold, size_t max_iters) {
  using vertex_id_type = typename Graph::vertex_id_type;

  std::size_t N          = graph.size();
  // Real        init_score = 1.0 / N;
  Real        base_score = (1.0 - damping_factor) / N;

  std::vector<Real> delta(N);
  std::vector<Real> residual(N);

  for (size_t i = 0; i < N; ++i) {
    page_rank[i] = 0.0;
    delta[i]     = 0.0;
    residual[i]  = 1.0 - damping_factor;    // Own contribution in the first iteration
  }

  for (size_t iter = 0; iter < max_iters; ++iter) {

    bool changed = false;

    Real           max_residual = 0, max_value = 0;
    vertex_id_type max_degree = 0;

    for (size_t src = 0; src < N; ++src) {
      delta[src]   = 0;
      max_degree   = std::max(degrees[src], max_degree);
      max_residual = std::max(residual[src], max_residual);
      if (residual[src] > threshold) {
        max_value        = std::max(max_value, page_rank[src]);
        Real oldResidual = residual[src];
        residual[src]    = 0.0;
        page_rank[src] += oldResidual;
        if (degrees[src] > 0) {
          delta[src] = oldResidual * damping_factor / (Real)degrees[src];
          changed    = true;
        }
      }
    }

    Real next_max = 0;

    for (size_t i = 0; i < N; ++i) {
      Real sum = 0.0;
      for (auto&& j : graph[i]) {
        if (delta[std::get<0>(j)] > 0) {
          sum += delta[std::get<0>(j)];
        }
      }
      next_max = std::max(next_max, sum);
      if (sum > 0) {
        residual[i] = sum;
      }
    }

    std::cout << iter << ": " << max_residual << ", " << next_max << ", " << max_value << ", " << max_degree << std::endl;

    if (!changed) {    // termination condition
      break;
    }
  }
}

template <adjacency_list_graph Graph, typename Real>
[[gnu::noinline]] void page_rank_v13(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees,
                                     std::vector<Real>& page_rank, Real damping_factor, Real threshold, size_t max_iters, size_t num_threads) {
  std::size_t N          = graph.size();
  Real        init_score = 1.0 / N;
  Real        base_score = (1.0 - damping_factor) / N;

  std::vector<Real> delta(N);
  std::vector<Real> residual(N);

  {
    nw::util::life_timer _("init data structures");

    // Initialize the page rank.
    tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
      for (auto i = r.begin(), e = r.end(); i != e; ++i) {
        page_rank[i] = 0.0;
        delta[i]     = 0.0;
        residual[i]  = 1.0 - damping_factor;    // Own contribution in the first iteration
      }
    });
  }


  for (size_t iter = 0; iter < max_iters; ++iter) {
    bool changed = false;

    auto&& [outgoing] = pagerank::time_op([&] {
      tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
        for (auto src = r.begin(), e = r.end(); src != e; ++src) {
          delta[src] = 0;
          if (residual[src] > threshold) {
            Real oldResidual = residual[src];
            page_rank[src] += oldResidual;
            residual[src] = 0.0;
            if (degrees[src] > 0) {
              delta[src] = oldResidual * damping_factor / degrees[src];
              changed    = true;
            }
          }
        }
      });
    });

    auto&& [time] = pagerank::time_op([&] {
      tbb::parallel_for(tbb::blocked_range(0ul, N), [&](auto&& r) {
        for (size_t i = r.begin(), e = r.end(); i != e; ++i) {

          Real sum = 0.0;
          for (auto&& j : graph[i]) {
            if (delta[std::get<0>(j)] > 0) {
              sum += delta[std::get<0>(j)];
            }
          }
          if (sum > 0) {
            residual[i] = sum;
          } else
            residual[i] = 0;
        }
      });
    });

    pagerank::trace(iter, 0, time, outgoing);

    if (!changed) {    // termination condition
      break;
    }
  }
}

template <adjacency_list_graph Graph, typename Real>
[[gnu::noinline]] std::size_t page_rank_v14(const Graph& graph, const std::vector<typename Graph::vertex_id_type>& degrees,
                                            std::vector<Real>& page_rank, Real damping_factor, Real threshold, size_t max_iters) {
  std::size_t N          = graph.size();
  Real        init_score = 1.0 / N;
  Real        base_score = (1.0 - damping_factor) / N;

  std::unique_ptr<Real[]> outgoing_contrib(new Real[N]);

  tbb::parallel_for(0ul, N, [&](auto&& i) {
    page_rank[i]        = init_score;
    outgoing_contrib[i] = init_score / degrees[i];
  });

  for (size_t iter = 0; iter < max_iters; ++iter) {
    Real error = nw::graph::parallel_reduce(
        tbb::blocked_range(0ul, N),
        [&](auto&& u) {
          Real z = 0.0;
          for (auto&& elt : graph[u]) {
            auto v = target(graph, elt);
            z += outgoing_contrib[v];
          }
          Real old_rank       = page_rank[u];
          page_rank[u]        = base_score + damping_factor * z;
          outgoing_contrib[u] = page_rank[u] / degrees[u];
          return fabs(page_rank[u] - old_rank);
        },
        std::plus{}, 0.0);

    if (error < threshold) {
      return iter;
    }
  }
  return max_iters;
}

}    // namespace graph
}    // namespace nw
#endif    //  NW_GRAPH_PAGE_RANK_EXPERIMENTAL_HPP
